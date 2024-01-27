#include "client.hpp"
#include "assets.hpp"
#include "border.hpp"
#include "border_container.hpp"
#include "button.hpp"
#include "horizontal_array.hpp"
#include "monitor.hpp"
#include "taskbar.hpp"
#include "vertical_array.hpp"
#include "window_wrapper.hpp"
#include "wm.hpp"
#include "wm_config.hpp"
#include "workspace.hpp"
#include "xapp.hpp"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdexcept>

Time Client::m_time_last_motion = CurrentTime;
Window Client::m_resize_constraint_window;
Border::Direction Client::m_resizing_direction;
int Client::m_before_x, Client::m_before_y, Client::m_before_mouse_x, Client::m_before_mouse_y;
unsigned int Client::m_before_width, Client::m_before_height;

Client::Client(Window window, XWindowAttributes window_attributes, Workspace *workspace) :
    m_workspace(workspace),
    m_icon(nullptr),
    m_moving(false),
    m_resizing(false),
    m_maximized(false),
    m_tiled(false),
    m_net_state(0)
{
    int titlebar_height = config->get_int("window.titlebar.height");
    set_event_mask(ButtonPressMask | ButtonReleaseMask | ExposureMask | PointerMotionMask);

    BorderContainer *border_container = new BorderContainer();
    set_main_container(border_container);

    VerticalArray *vertical_container = new VerticalArray();
    border_container->set_child(vertical_container);

    m_managed_window = new ManagedWindow(window);
    vertical_container->set_center(m_managed_window);

    m_title = m_managed_window->get_title();
    m_icon = m_managed_window->get_icon();

    HorizontalArray *horizontal_container = new HorizontalArray();
    horizontal_container->set_height(titlebar_height);
    vertical_container->add_top(horizontal_container);

    m_titlebar = new TitleBar();
    m_titlebar->set_text(get_title());
    m_titlebar->set_icon(get_icon());
    horizontal_container->set_center(m_titlebar);

    m_button = new Button();
    m_button->set_width(titlebar_height);
    m_button->set_action([this](XButtonEvent *event) { this->close(); },
                         Button::ActionTime::on_release);
    m_button->set_icon(assets->get_image_from_path("images/close.xpm"));
    horizontal_container->add_right(m_button);

    Button *fullscreen_button = new Button();
    fullscreen_button->set_width(titlebar_height*1.5);
    fullscreen_button->set_text("full");
    fullscreen_button->set_action([this](XButtonEvent *event) { this->toggle_fullscreen(); },
                                  Button::ActionTime::on_release);
    
    horizontal_container->add_left(fullscreen_button);

    set_size_and_position(window_attributes.x,
                          window_attributes.y,
                          window_attributes.width + 2 * config->get_int("window.border.size"),
                          window_attributes.height + 2 * config->get_int("window.border.size")
                                  + config->get_int("window.titlebar.height"));

    m_workspace->add_client(this);

    create();
    focus();
    update_size_hints();
    XFlush(xapp->display());
}

Client::~Client()
{
    delete m_main_container;
    if (m_icon) {
        XDestroyImage(m_icon);
    }
    m_workspace->remove_client(this);
}

void Client::create()
{
    create_window(get_x(), get_y(), get_width(), get_height());

    /* Create children */
    m_main_container->create();

    show();
}

void Client::set_main_container(Container *container)
{
    m_main_container = container;
    m_main_container->set_parent(this);
    m_main_container->set_primary_container(this);
}

void Client::set_size(unsigned int width, unsigned int height)
{
    WindowWrapper::set_size(width, height);
    m_main_container->set_size(width, height);
}

void Client::set_size_and_position(int x, int y, unsigned int width, unsigned int height)
{
    WindowWrapper::set_size_and_position(x, y, width, height);
    m_main_container->set_size(width, height);
}

/* TODO: Maybe move this to WindowWrapper */
void Client::raise()
{
    XRaiseWindow(xapp->display(), get_window());
    XUngrabButton(xapp->display(), AnyButton, AnyModifier, get_managed()->get_window());
}

void Client::focus()
{
    m_focused = true;
    m_workspace->focus_client(this);

    XSetInputFocus(xapp->display(), get_managed()->get_window(), RevertToPointerRoot, CurrentTime);
    Window window = get_managed()->get_window();
    XChangeProperty(xapp->display(), xapp->root(), xapp->atom(XApp::AtomType::net_active_window),
            XA_WINDOW, 32, PropModeReplace, (unsigned char *) &window, 1);
    wm->send_message(this, xapp->atom(XApp::AtomType::wm_take_focus));

    m_titlebar->set_image("images/selectedblue.xpm");
    m_button->set_image("images/selectedblue.xpm");
    static_cast<BorderContainer *>(m_main_container)->set_border_image("images/selectedblue.xpm");
}

void Client::unfocus()
{
    m_focused = false;
    XGrabButton(xapp->display(), AnyButton, AnyModifier, get_managed()->get_window(), False,
            ButtonPressMask, GrabModeAsync, GrabModeSync, None, None);
    m_titlebar->set_image("images/lightblue.xpm");
    m_button->set_image("images/lightblue.xpm");
    static_cast<BorderContainer *>(m_main_container)->set_border_image("images/lightblue.xpm");
}

void Client::close()
{
    wm->send_message(this, xapp->atom(XApp::AtomType::wm_delete));
}

bool Client::is_fullscreen()
{
    return (m_net_state & NetState::fullscreen) != 0;
}

void Client::toggle_fullscreen()
{
    if (!is_fullscreen()) {
        const Atom net_wm_fullscreen = xapp->atom(XApp::AtomType::net_wm_fullscreen);

        XChangeProperty(xapp->display(), get_managed()->get_window(),
                xapp->atom(XApp::AtomType::net_wm_state), XA_ATOM, 32, PropModeReplace,
                (unsigned char *)&net_wm_fullscreen, 1);
        
        m_net_state |= NetState::fullscreen;

        m_unmaximized_or_untiled_width = get_width();
        m_unmaximized_or_untiled_height = get_height();
        m_unmaximized_or_untiled_x = get_x();
        m_unmaximized_or_untiled_y = get_y();

        Monitor *monitor = m_workspace->get_monitor();
        set_size_and_position(monitor->get_x(), monitor->get_y(),
                monitor->get_width(), monitor->get_height());
        static_cast<BorderContainer *>(m_main_container)->toggle_borders();
        get_managed()->set_size_and_position(0, 0, get_width(), get_height());
        get_managed()->rise();
        
        raise();
    } else {
        XChangeProperty(xapp->display(), get_managed()->get_window(),
                xapp->atom(XApp::AtomType::net_wm_state), XA_ATOM, 32, PropModeReplace,
                (unsigned char *)0, 0);
        static_cast<BorderContainer *>(m_main_container)->toggle_borders();
        
        m_net_state &= ~NetState::fullscreen;

        set_size_and_position(m_unmaximized_or_untiled_x, m_unmaximized_or_untiled_y,
                m_unmaximized_or_untiled_width, m_unmaximized_or_untiled_height);
    }
}

void Client::maximize()
{
    int titlebar_height = config->get_int("window.titlebar.height");
    int monitor_x = m_workspace->get_monitor()->get_x();
    int monitor_y = m_workspace->get_monitor()->get_y();
    int monitor_width = m_workspace->get_monitor()->get_width();
    int monitor_height = m_workspace->get_monitor()->get_height();
    m_maximized = true;

    m_unmaximized_or_untiled_x = get_x();
    m_unmaximized_or_untiled_y = get_y();
    m_unmaximized_or_untiled_width = get_width();
    m_unmaximized_or_untiled_height = get_height();
    set_size_and_position(monitor_x,
                          monitor_y + titlebar_height,
                          monitor_width,
                          monitor_height - titlebar_height);
}

void Client::tile(int x, int y, unsigned int width, unsigned int height)
{
    m_tiled = true;

    m_unmaximized_or_untiled_x = get_x();
    m_unmaximized_or_untiled_y = get_y();
    m_unmaximized_or_untiled_width = get_width();
    m_unmaximized_or_untiled_height = get_height();
    set_size_and_position(x, y, width, height);
}

void Client::unmaximize_or_untile()
{
    m_maximized = false;
    m_tiled = false;
    set_size_and_position(m_unmaximized_or_untiled_x,
                          m_unmaximized_or_untiled_y,
                          m_unmaximized_or_untiled_width,
                          m_unmaximized_or_untiled_height);
}

void Client::toggle_maximize()
{
    if (!m_maximized) {
        maximize();
    } else {
        unmaximize_or_untile();
    }
}

std::string Client::get_title()
{
    return m_title;
}

XImage *Client::get_icon()
{
    return m_icon;
}

void Client::update_title()
{
    m_title = get_managed()->get_title();
    m_workspace->get_monitor()->get_taskbar()->update_tasklist();
    m_titlebar->set_text(m_title);
}

void Client::update_icon()
{
    m_icon = get_managed()->get_icon();
    m_titlebar->set_icon(m_icon);
}

void Client::update_size_hints()
{
    long msize;
    XSizeHints size;

    if (!XGetWMNormalHints(xapp->display(), get_managed()->get_window(), &size, &msize)) {
        throw std::runtime_error("failed to get window hints for: " + std::string(get_title()));
    }

    if (size.flags & PResizeInc) {
        m_constraints.width_inc = size.width_inc;
        m_constraints.height_inc = size.height_inc;
    } else {
        m_constraints.width_inc = 1;
        m_constraints.height_inc = 1;
    }

    if (size.flags & PMinSize) {
        m_constraints.min_width = size.min_width;
        m_constraints.min_height = size.min_height;
    } else {
        m_constraints.min_width = 100;
        m_constraints.min_height = 26;
    }

    if (size.flags & PMaxSize) {
        m_constraints.max_width = size.max_width;
        m_constraints.max_height = size.max_height;
    } else {
        m_constraints.max_width = 0;
        m_constraints.max_height = 0;
    }
}

void Client::begin_drag_move(XButtonEvent *event)
{
    if (m_maximized || m_tiled) {
        int border_size = config->get_int("window.border.size");
        int titlebar_height = config->get_int("window.titlebar.height");
        unmaximize_or_untile();
        // XWarpPointer(xapp->display(), None, get_window(),
        //         0, 0, 0, 0,
        //         get_width() / 2, border_size + titlebar_height / 2);
        m_before_mouse_x = get_x() + get_width() / 2;
        m_before_mouse_y = get_y() + border_size + titlebar_height / 2;
    } else {
        m_before_mouse_x = event->x_root;
        m_before_mouse_y = event->y_root;
    }
    XGrabPointer(xapp->display(),
                 get_window(),
                 False,
                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync,
                 GrabModeAsync,
                 None,
                 wm->cursor(XApp::CursorType::move),
                 CurrentTime);
    m_moving = true;
    wm->set_window_moving(true);
    m_before_x = get_x();
    m_before_y = get_y();
}

bool Client::stick_to_sides(
        int mouse_x, int mouse_y, int *x, int *y, unsigned int *width, unsigned int *height)
{
    int titlebar_height = config->get_int("window.titlebar.height");
    int monitor_x = m_workspace->get_monitor()->get_x();
    int monitor_y = m_workspace->get_monitor()->get_y();
    int monitor_width = m_workspace->get_monitor()->get_width();
    int monitor_height = m_workspace->get_monitor()->get_height();
    int margin = 10;
    int corner_size = 100;
    if (mouse_x < monitor_x + margin) { /* Left */
        *x = monitor_x;
        *width = monitor_width / 2;
        if (mouse_y < monitor_y + corner_size + titlebar_height) { /* Top left */
            *y = monitor_y + titlebar_height;
            *height = (monitor_height - titlebar_height) / 2;
        } else if (mouse_y > monitor_y + monitor_height - corner_size) { /* Bottom left */
            *y = monitor_y + titlebar_height + (monitor_height - titlebar_height) / 2;
            /* The boolean operation prevents the missing pixel in the bottom if division is odd*/
            *height = (monitor_height - titlebar_height) / 2
                      + ((monitor_height - titlebar_height) % 2 != 0);
        } else { /* Middle left */
            *y = monitor_y + titlebar_height;
            *height = monitor_height - titlebar_height;
        }
    } else if (mouse_x > monitor_x + monitor_width - margin) { /* Right */
        *x = monitor_x + monitor_width / 2;
        *width = monitor_width / 2 + (monitor_width % 2 != 0);
        if (mouse_y < monitor_y + corner_size + titlebar_height) { /* Top right */
            *y = monitor_y + titlebar_height;
            *height = (monitor_height - titlebar_height) / 2;
        } else if (mouse_y > monitor_y + monitor_height - corner_size) { /* Bottom right */
            *y = monitor_y + titlebar_height + (monitor_height - titlebar_height) / 2;
            /* The boolean operation prevents the missing pixel in the bottom if division is odd*/
            *height = (monitor_height - titlebar_height) / 2
                      + ((monitor_height - titlebar_height) % 2 != 0);
        } else { /* Middle right */
            *y = titlebar_height;
            *height = monitor_height - titlebar_height;
        }
    } else if (mouse_y < monitor_y + margin + titlebar_height) { /* Top */
        *y = monitor_y + titlebar_height;
        *height = (monitor_height - titlebar_height) / 2;
        if (mouse_x < monitor_x + corner_size) { /* Top left */
            *x = monitor_x;
            *width = monitor_width / 2;
        } else if (mouse_x > monitor_x + monitor_width - corner_size) { /* Top right */
            *x = monitor_width / 2;
            /* The boolean operation prevents the missing pixel in the bottom if division is odd*/
            *width = monitor_width / 2 + (monitor_width % 2 != 0);
        } else { /* Top middle */
            *x = monitor_x;
            *width = monitor_width;
        }
    } else if (mouse_y > monitor_y + monitor_height - margin) { /* Bottom */
        *y = monitor_y + titlebar_height + (monitor_height - titlebar_height) / 2;
        *height = (monitor_height - titlebar_height) / 2
                  + ((monitor_height - titlebar_height) % 2 != 0);
        if (mouse_x < monitor_x + corner_size) { /* Bottom left */
            *x = monitor_x;
            *width = monitor_width / 2;
        } else if (mouse_x > monitor_width - corner_size) { /* Bottom right */
            *x = monitor_x + monitor_width / 2;
            /* The boolean operation prevents the missing pixel in the bottom if division is odd*/
            *width = monitor_width / 2 + (monitor_width % 2 != 0);
        } else { /* Bottom middle */
            *x = monitor_x;
            *width = monitor_width;
        }
    } else { /* did not stick/is not in a border */
        return false;
    }
    return true;
}

void Client::drag_move(XMotionEvent *event)
{
    int x = event->x_root + m_before_x - m_before_mouse_x;
    int y = event->y_root + m_before_y - m_before_mouse_y;
    unsigned int width = get_width();
    unsigned int height = get_height();

    /* if mouse moved to another screen change screen of client*/
    /* TODO: maybe do something better */
    Monitor *new_monitor = wm->get_monitor_from_position(event->x_root, event->y_root);
    if (m_workspace->get_monitor() != new_monitor) {
        m_workspace->remove_client(this);
        m_workspace = new_monitor->get_selected_workspace();
        m_workspace->add_client(this);
        m_workspace->focus_client(this);
    }

    stick_to_sides(event->x_root, event->y_root, &x, &y, &width, &height);
    if (config->get_bool("window.animation.draw_rectangle_on_move")) {
        wm->draw_rectangle(x, y, width, height);
    } else {
        set_size_and_position(x, y, width, height);
    }
}

void Client::end_drag_move(XButtonEvent *event)
{
    int new_x, new_y;
    unsigned int new_width, new_height;
    XUngrabPointer(xapp->display(), CurrentTime);
    XSync(xapp->display(), False);
    m_moving = false;
    wm->set_window_moving(false);
    if (config->get_bool("window.animation.draw_rectangle_on_move")) {
        wm->draw_rectangle(0, 0, 0, 0);
    }
    new_x = event->x_root + m_before_x - m_before_mouse_x;
    new_y = event->y_root + m_before_y - m_before_mouse_y;
    new_width = get_width();
    new_height = get_height();

    /* If sticked to a border because mouse is in a border tile the client in that position */
    if (stick_to_sides(event->x_root, event->y_root, &new_x, &new_y, &new_width, &new_height)) {
        tile(new_x, new_y, new_width, new_height);

    /* If it didn't stick move the window to where the mouse moved */
    } else if (new_x != get_x() || new_y != get_y() || new_width != get_width()
               || new_height != get_height())
    {
        set_size_and_position(new_x, new_y, new_width, new_height);
        /* TODO: maybe do something better */
        Monitor *new_monitor = wm->get_monitor_from_position(event->x_root, event->y_root);
        if (m_workspace->get_monitor() != new_monitor) {
            m_workspace->remove_client(this);
            m_workspace = new_monitor->get_selected_workspace();
            m_workspace->add_client(this);
            m_workspace->focus_client(this);
        }
    }
    m_titlebar->set_pressed(false); // TODO: Do something better?
}

void Client::begin_drag_resize(XButtonEvent *event, Border::Direction direction)
{
    /* TODO: For restraining the pointer when resizing */
    // XSetWindowAttributes window_attributes;
    // window_attributes.colormap = xapp->colormap(32);
    // window_attributes.background_pixel = 0;
    // window_attributes.border_pixel = 0;
    // m_resize_constraint_window = XCreateWindow(
    //         xapp->display(), xapp->root(),
    //         0, 0,
    //         400, 400,
    //         0, 32,
    //         CopyFromParent, xapp->visual(32),
    //         CWColormap|CWBackingPixel|CWBorderPixel, &window_attributes);

    XGrabPointer(xapp->display(),
                 get_window(),
                 False,
                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync,
                 GrabModeAsync,
                 None,
                 wm->cursor(Border::cursor_type_from_direction(direction)),
                 CurrentTime);

    m_resizing = true;
    wm->set_window_resizing(true);
    m_resizing_direction = direction;
    m_before_x = get_x();
    m_before_y = get_y();
    m_before_width = get_width();
    m_before_height = get_height();
    m_before_mouse_x = event->x_root;
    m_before_mouse_y = event->y_root;
}

void Client::get_drag_resize_parameters(
        int mouse_x, int mouse_y, int *x, int *y, int *width, int *height)
{
    int new_x, new_y;
    int new_width, new_height;
    switch (m_resizing_direction) {
    case Border::Direction::nw:
        {
            new_x = m_before_x + mouse_x - m_before_mouse_x;
            new_y = m_before_y + mouse_y - m_before_mouse_y;
            new_width = m_before_width - mouse_x + m_before_mouse_x;
            new_height = m_before_height - mouse_y + m_before_mouse_y;
        }
        break;
    case Border::Direction::ne:
        {
            new_x = m_before_x;
            new_y = m_before_y + mouse_y - m_before_mouse_y;
            new_width = m_before_width + mouse_x - m_before_mouse_x;
            new_height = m_before_height - mouse_y + m_before_mouse_y;
        }
        break;
    case Border::Direction::sw:
        {
            new_x = m_before_x + mouse_x - m_before_mouse_x;
            new_y = m_before_y;
            new_width = m_before_width - mouse_x + m_before_mouse_x;
            new_height = m_before_height + mouse_y - m_before_mouse_y;
        }
        break;
    case Border::Direction::se:
        {
            new_x = m_before_x;
            new_y = m_before_y;
            new_width = m_before_width + mouse_x - m_before_mouse_x;
            new_height = m_before_height + mouse_y - m_before_mouse_y;
        }
        break;
    case Border::Direction::n:
        {
            new_x = m_before_x;
            new_y = m_before_y + mouse_y - m_before_mouse_y;
            new_width = m_before_width;
            new_height = m_before_height - mouse_y + m_before_mouse_y;
        }
        break;
    case Border::Direction::e:
        {
            new_x = m_before_x;
            new_y = m_before_y;
            new_width = m_before_width + mouse_x - m_before_mouse_x;
            new_height = m_before_height;
        }
        break;
    case Border::Direction::w:
        {
            new_x = m_before_x + mouse_x - m_before_mouse_x;
            new_y = m_before_y;
            new_width = m_before_width - mouse_x + m_before_mouse_x;
            new_height = m_before_height;
        }
        break;
    case Border::Direction::s:
        {
            new_x = m_before_x;
            new_y = m_before_y;
            new_width = m_before_width;
            new_height = m_before_height + mouse_y - m_before_mouse_y;
        }
        break;
    case Border::last:
        throw std::runtime_error("This can't happen.");
        break;
    }
    *x = new_x;
    *y = new_y;
    *width = new_width;
    *height = new_height;
}

void Client::apply_size_hints(int *x, int *y, int *width, int *height)
{
    int decorations_width = get_width() - get_managed()->get_width();
    int decorations_height = get_height() - get_managed()->get_height();

    *width -= decorations_width;
    *height -= decorations_height;

    
    if (m_resizing_direction == Border::Direction::nw
            || m_resizing_direction == Border::Direction::w
            || m_resizing_direction == Border::Direction::sw) {
        *x += *width % m_constraints.width_inc;
    }
    if (m_resizing_direction == Border::Direction::nw
            || m_resizing_direction == Border::Direction::n
            || m_resizing_direction == Border::Direction::ne) {
        *y += *height % m_constraints.height_inc;
    }
    *width -= *width % m_constraints.width_inc;
    *height -= *height % m_constraints.height_inc;

    if (m_constraints.max_width && *width > m_constraints.max_width) {
        if (m_resizing_direction == Border::Direction::nw
                || m_resizing_direction == Border::Direction::w
                || m_resizing_direction == Border::Direction::sw) {
            *x += *width - m_constraints.max_width;
        }
        *width = std::min(*width, m_constraints.max_width);
    }
    if (m_constraints.max_height && *height > m_constraints.max_height) {
        if (m_resizing_direction == Border::Direction::nw
                || m_resizing_direction == Border::Direction::n
                || m_resizing_direction == Border::Direction::ne) {
            *y += *height - m_constraints.max_height;
        }
        *height = std::min(*height, m_constraints.max_height);
    }

    if (*width < m_constraints.min_width) {
        if (m_resizing_direction == Border::Direction::nw
                || m_resizing_direction == Border::Direction::w
                || m_resizing_direction == Border::Direction::sw) {
            *x += *width - m_constraints.min_width;
        }
        *width = std::max(*width, m_constraints.min_width);
    }
    if (*height < m_constraints.min_height) {
        if (m_resizing_direction == Border::Direction::nw
                || m_resizing_direction == Border::Direction::n
                || m_resizing_direction == Border::Direction::ne) {
            *y += *height - m_constraints.min_height;
        }
        *height = std::max(*height, m_constraints.min_height);
    }

    // TODO: aspect ratio

    *width += decorations_width;
    *height += decorations_height;
}

void Client::drag_resize(XMotionEvent *event)
{
    int new_x, new_y;
    int new_width, new_height;

    get_drag_resize_parameters(
            event->x_root, event->y_root, &new_x, &new_y, &new_width, &new_height);

    /* TODO: Use a window that constrains the mouse */
    apply_size_hints(&new_x, &new_y, &new_width, &new_height);

    if (config->get_bool("window.animation.draw_rectangle_on_move")) {
        wm->draw_rectangle(new_x, new_y, new_width, new_height);
    } else {
        set_size_and_position(new_x, new_y, new_width, new_height);
    }
}

void Client::end_drag_resize(XButtonEvent *event)
{
    int new_x, new_y;
    int new_width, new_height;
    XUngrabPointer(xapp->display(), CurrentTime);
    XSync(xapp->display(), False);
    m_resizing = false;
    wm->set_window_resizing(false);
    if (config->get_bool("window.animation.draw_rectangle_on_move")) {
        wm->draw_rectangle(0, 0, 0, 0);
    }
    get_drag_resize_parameters(
            event->x_root, event->y_root, &new_x, &new_y, &new_width, &new_height);
    apply_size_hints(&new_x, &new_y, &new_width, &new_height);
    if (new_x != get_x() || new_y != get_y() || new_width != static_cast<int>(get_width())
        || new_height != static_cast<int>(get_height()))
    {
        set_size_and_position(new_x, new_y, new_width, new_height);
    }
    // Destroy constrain window
}

void Client::child_hovered()
{
    if (config->get_bool("sloppy_focus")) {
        focus();
    }
}

void Client::child_pressed()
{
    if (!config->get_bool("sloppy_focus")) {
        focus();
    }
    raise();
}

void Client::handle_button(XButtonEvent *event)
{
    if (m_moving) {
        if (event->type == ButtonRelease) {
            end_drag_move(event);
        }
    }
    if (m_resizing) {
        if (event->type == ButtonRelease) {
            end_drag_resize(event);
        }
    }
}

void Client::handle_motion(XMotionEvent *event)
{
    if (m_moving) {
        if (event->time - m_time_last_motion > 1000 / 60) {
            m_time_last_motion = event->time;
            drag_move(event);
        }
    } else if (m_resizing) {
        if (event->time - m_time_last_motion > 1000 / 60) {
            m_time_last_motion = event->time;
            drag_resize(event);
        }
    }
}

ManagedWindow *Client::get_managed()
{
    return m_managed_window;
}

Workspace *Client::get_workspace()
{
    return m_workspace;
}

void Client::set_workspace(Workspace *workspace)
{
    m_workspace = workspace;
}
