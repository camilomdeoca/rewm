#include "wm.hpp"
#include "monitor.hpp"
#include "window_wrapper.hpp"
#include "wm_config.hpp"
#include "wm_root_window.hpp"
#include <vector>
#include <X11/extensions/Xinerama.h>
#include <X11/Xlib.h>

WindowManager *wm;

WindowManager::WindowManager() :
    m_exit(false),
    m_window_moving(false),
    m_window_resizing(false),
    m_old_rectangle({0, 0, 0, 0})
{
    XSetWindowAttributes root_window_attributes;

    root_window_attributes.event_mask =
            SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | PointerMotionMask
            | EnterWindowMask | LeaveWindowMask | StructureNotifyMask | PropertyChangeMask;

    root_window_attributes.cursor = cursor(CursorType::normal);

    XChangeWindowAttributes(display(), root(), CWEventMask | CWCursor, &root_window_attributes);
    XSelectInput(display(), root(), root_window_attributes.event_mask);

    int rectangle_line_width = config->get_int("window.animation.rectangle_line_width");
    XGCValues rectangle_gc_values;
    rectangle_gc_values.function = GXxor;
    rectangle_gc_values.foreground = 0xffffffff;
    rectangle_gc_values.line_width = rectangle_line_width;
    rectangle_gc_values.subwindow_mode = IncludeInferiors;
    m_rectangle_gc = XCreateGC(display(),
                               root(),
                               GCFunction | GCLineWidth | GCForeground | GCSubwindowMode,
                               &rectangle_gc_values);

    m_old_rectangle = {0, 0, 0, 0};
}

WindowManager::~WindowManager()
{
    for (Monitor *monitor : m_monitors) {
        delete monitor;
    }
    XFreeGC(display(), m_rectangle_gc);
}

void WindowManager::setup()
{
    /* Setup monitors */
    int monitor_count;
    XineramaScreenInfo *xinerama_monitor_infos =
            XineramaQueryScreens(xapp->display(), &monitor_count);
    std::vector<XineramaScreenInfo> monitor_infos(xinerama_monitor_infos,
                                                  xinerama_monitor_infos + monitor_count);

    #if 0 // For testing multiple monitors
    Monitor *monitor1 = new Monitor(800, 720, 0, 0);
    Monitor *monitor2 = new Monitor(800, 720, 800, 0);
    m_monitors.push_back(monitor1);
    m_monitors.push_back(monitor2);
    #else
    for (XineramaScreenInfo info : monitor_infos) {
        /* TODO: check for duplicate monitors */
        m_monitors.emplace_back(new Monitor(info.width, info.height, info.x_org, info.y_org));
    }
    #endif

    XFree(xinerama_monitor_infos);
    m_focused_monitor = m_monitors.front();

    m_root_window = new WMRootWindow(root());

    xapp->flush();

    /* Manage existing windows */
    //scan_existing_windows(); // Does not work
}

void WindowManager::exit()
{
    m_exit = true;
}

void WindowManager::map_window(Window window)
{
    XWindowAttributes window_attributes;
    int error = XGetWindowAttributes(display(), window, &window_attributes);
    if (!error || window_attributes.override_redirect)
        return;
    m_clients.insert({window, new Client(window, window_attributes, m_focused_monitor->get_selected_workspace())});
}

void WindowManager::add_window_wrapper(WindowWrapper *window_wrapper)
{
    m_window_wrappers.insert({window_wrapper->get_window(), window_wrapper});
}

void WindowManager::remove_window_wrapper(WindowWrapper *window_wrapper)
{
    m_window_wrappers.erase(window_wrapper->get_window());
}

void WindowManager::focus_monitor(Monitor *monitor)
{
    if (m_focused_monitor != monitor) {
        if (m_focused_monitor->get_selected_workspace()->get_focused_client()) {
            m_focused_monitor->get_selected_workspace()->get_focused_client()->unfocus();
        }
        m_focused_monitor = monitor;
    }
}

void WindowManager::set_window_moving(bool value)
{
    m_window_moving = value;
}

void WindowManager::set_window_resizing(bool value)
{
    m_window_resizing = value;
}

/* This is how fvwm did it :) */
void WindowManager::draw_rectangle(int x, int y, unsigned int width, unsigned int height)
{
    int rectangle_line_width = config->get_int("window.animation.rectangle_line_width");
    XRectangle rect[2];
    rect[0].x = x + rectangle_line_width / 2;
    rect[0].y = y + rectangle_line_width / 2;
    rect[0].width = width - rectangle_line_width;
    rect[0].height = height - rectangle_line_width;
    rect[1] = m_old_rectangle;
    if (m_window_moving || m_window_resizing) {
        if (m_old_rectangle.width == 0 || m_old_rectangle.height == 0) { // If w or h = 0 we assume
            XDrawRectangles(xapp->display(),                             // its the first rectangle
                            xapp->root(),                                // so we dont have to erase
                            m_rectangle_gc,                              // the old (draw over it).
                            &rect[0],
                            1);
            m_old_rectangle = rect[0];
        } else {
            XDrawRectangles(xapp->display(), xapp->root(), m_rectangle_gc, &rect[0], 2);
            m_old_rectangle = rect[0];
        }
    } else { // erase last rectangle by drawing over it
        XDrawRectangles(xapp->display(), xapp->root(), m_rectangle_gc, &m_old_rectangle, 1);
        m_old_rectangle.width = 0;
        m_old_rectangle.height = 0;
    }
    XFlush(xapp->display());
}

void WindowManager::send_message(Client *client, Atom message)
{
    ManagedWindow *managed;
    int protocol_count;
    Atom *protocols;
    XEvent client_event;
    bool client_has_protocol = false;

    /* Get Client of window */
    managed = client->get_managed();

    if (XGetWMProtocols(display(), managed->get_window(), &protocols, &protocol_count)) {
        int i = 0;
        while (i < protocol_count && !client_has_protocol) {
            client_has_protocol = protocols[i] == message;
            i++;
        }
        XFree(protocols);
    }

    if (client_has_protocol) {
        client_event.type = ClientMessage;
        client_event.xclient.window = managed->get_window();
        client_event.xclient.message_type = atom(AtomType::wm_protocols);
        client_event.xclient.format = 32;
        client_event.xclient.data.l[0] = message;
        client_event.xclient.data.l[1] = CurrentTime;

        XSendEvent(display(), managed->get_window(), False, NoEventMask, &client_event);
    } else {
        /* TODO: close client anyway if it doesnt support WM_DELETE_WINDOW */
    }
}

Monitor *WindowManager::get_monitor_from_position(int x, int y)
{
    for (Monitor *monitor : m_monitors) {
        if (x >= monitor->get_x() && x < monitor->get_x() + monitor->get_width()
            && y >= monitor->get_y() && y < monitor->get_y() + monitor->get_height())
        {
            return monitor;
        }
    }
    throw std::runtime_error("Mouse not inside any monitor.");
}

Monitor *WindowManager::get_focused_monitor()
{
    return m_focused_monitor;
}

Client *WindowManager::get_client(Window window)
{
    return m_clients.find(window)->second;
}

#if 0
long getstate(Window w)
{
	int format;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;

	if (XGetWindowProperty(xapp->display(), w, xapp->atom(XApp::AtomType::wm_state), 0L, 2L, False,
            xapp->atom(XApp::AtomType::wm_state), &real, &format, &n, &extra, (unsigned char **)&p) != Success) 
		return -1;

	if (n != 0)
		result = *p;

	XFree(p);

	return result;
}

void WindowManager::scan_existing_windows()
{
    XGrabServer(xapp->display());

    unsigned int window_count;
    Window root, parent, transient_for, *children;
    XWindowAttributes window_attributes;

    if (XQueryTree(xapp->display(), xapp->root(), &root, &parent, &children, &window_count)) {
        for (unsigned int i = 0; i < window_count; i++) {
            if (m_window_wrappers.find(children[i]) != m_window_wrappers.end())
                continue;

            if (!XGetWindowAttributes(xapp->display(), children[i], &window_attributes)
                    || window_attributes.override_redirect
                    || XGetTransientForHint(xapp->display(), children[i], &transient_for))
                continue;

            if (window_attributes.map_state == IsViewable || getstate(children[i]) == IconicState) {
                map_window(children[i]);
            }
        }

        for (unsigned int i = 0; i < window_count; i++) {
            if (m_window_wrappers.find(children[i]) != m_window_wrappers.end())
                continue;

            if (!XGetWindowAttributes(xapp->display(), children[i], &window_attributes))
                continue;

            if (XGetTransientForHint(xapp->display(), children[i], &transient_for)
                    && (window_attributes.map_state == IsViewable || getstate(children[i]) == IconicState))
                //map_window(children[i]);
                continue;
        }

        XFree(children);
    }
    XUngrabServer(xapp->display());
}
#endif

void WindowManager::handle_event(XEvent *event)
{
    std::unordered_map<Window, WindowWrapper *>::iterator found_pair =
            m_window_wrappers.find(event->xany.window);
    WindowWrapper *window_wrapper_of_event =
            found_pair == m_window_wrappers.end() ? nullptr : found_pair->second;

    if (!window_wrapper_of_event && event->type != MapRequest)
        return;
    switch (event->type) {
    case MapRequest:
        map_window(event->xmaprequest.window);
        break;
    case ButtonPress:
    case ButtonRelease:
        window_wrapper_of_event->handle_button(&event->xbutton);
        break;
    case Expose:
        window_wrapper_of_event->handle_expose(&event->xexpose);
        break;
    case DestroyNotify:
        window_wrapper_of_event->handle_destroy_notify(&event->xdestroywindow);
        break;
    case MotionNotify:
        window_wrapper_of_event->handle_motion(&event->xmotion);
        break;
    case EnterNotify:
        window_wrapper_of_event->handle_enter(&event->xcrossing);
        break;
    case LeaveNotify:
        window_wrapper_of_event->handle_leave(&event->xcrossing);
        break;
    case PropertyNotify:
        window_wrapper_of_event->handle_property_notify(&event->xproperty);
        break;
    case ClientMessage:
        window_wrapper_of_event->handle_client_message(&event->xclient);
        break;
    case UnmapNotify:
        window_wrapper_of_event->handle_unmap_notify(&event->xunmap);
    default:
        // Not implemented
        break;
    }
}

void WindowManager::main_loop()
{
    XEvent event;
    XSync(display(), False);
    while (!m_exit && !XNextEvent(display(), &event)) {
        handle_event(&event);
    }
}
