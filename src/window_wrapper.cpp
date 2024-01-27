#include "window_wrapper.hpp"
#include "container.hpp"
#include "primary_container.hpp"
#include "wm.hpp"
#include "xapp.hpp"
#include <X11/Xlib.h>

WindowWrapper::WindowWrapper(Window window) :
    m_window(None),
    m_parent(nullptr),
    m_x(0),
    m_y(0),
    m_width(1),
    m_height(1)
{
    if (window != None) {
        set_window(window);
    }
}

WindowWrapper::~WindowWrapper()
{
    /* If window exists */
    if (get_window() != None) {
        wm->remove_window_wrapper(this);
        XDestroyWindow(xapp->display(), get_window());
    }
}

void WindowWrapper::set_event_mask(unsigned long event_mask)
{
    m_event_mask = event_mask;
    if (get_window() != None) {
        XSelectInput(xapp->display(), get_window(), event_mask);
    }
}

void WindowWrapper::set_cursor(Cursor cursor)
{
    m_cursor = cursor;
    if (get_window() != None) {
        XDefineCursor(xapp->display(), get_window(), cursor);
    }
}

void WindowWrapper::create_window(int x, int y, unsigned int width, unsigned int height)
{
    XSetWindowAttributes window_attributes;
    window_attributes.colormap = xapp->colormap(32);
    window_attributes.background_pixel = 0;
    window_attributes.border_pixel = 0;

    Window window =
            XCreateWindow(xapp->display(),
                          get_parent() != nullptr ? get_parent()->get_window() : xapp->root(),
                          x,
                          y,
                          width,
                          height,
                          0,
                          32,
                          CopyFromParent,
                          xapp->visual(32),
                          CWBackPixel | CWBorderPixel | CWColormap,
                          &window_attributes);

    set_window(window);
}

Window WindowWrapper::get_window()
{
    return m_window;
}

void WindowWrapper::set_window(Window window)
{
    m_window = window;
    XWindowAttributes window_attributes;
    XGetWindowAttributes(xapp->display(), m_window, &window_attributes);

    m_x = window_attributes.x;
    m_y = window_attributes.y;
    m_width = window_attributes.width;
    m_height = window_attributes.height;

    wm->add_window_wrapper(this);
}

void WindowWrapper::set_parent(Container *parent)
{
    m_parent = parent;
    if (get_window() != None && get_parent()->get_window() != None) {
        XReparentWindow(
                xapp->display(), get_window(), get_parent()->get_window(), get_x(), get_y());
    }
}

void WindowWrapper::set_primary_container(PrimaryContainer *primary_container)
{
    m_primary_container = primary_container;
}

void WindowWrapper::set_window_deleted()
{
    wm->remove_window_wrapper(this);
    m_window = None;
}

int WindowWrapper::get_x()
{
    return m_x;
}

int WindowWrapper::get_y()
{
    return m_y;
}

unsigned int WindowWrapper::get_width()
{
    return m_width;
}

unsigned int WindowWrapper::get_height()
{
    return m_height;
}

void WindowWrapper::set_width(unsigned int width)
{
    set_size(width, get_height());
}

void WindowWrapper::set_height(unsigned int height)
{
    set_size(get_width(), height);
}

void WindowWrapper::set_size(unsigned int width, unsigned int height)
{
    m_width = width;
    m_height = height;
    if (get_window() != None) {
        XResizeWindow(xapp->display(), m_window, width, height);
    }
}

void WindowWrapper::set_position(int x, int y)
{
    m_x = x;
    m_y = y;
    if (get_window() != None) {
        XMoveWindow(xapp->display(), m_window, x, y);
    }
}

void WindowWrapper::set_size_and_position(int x, int y, unsigned int width, unsigned int heigth)
{
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = heigth;
    if (get_window() != None) {
        XMoveResizeWindow(xapp->display(), m_window, x, y, width, heigth);
    }
}

void WindowWrapper::show()
{
    XMapWindow(xapp->display(), m_window);
}

void WindowWrapper::hide()
{
    XUnmapWindow(xapp->display(), m_window);
}

void WindowWrapper::rise()
{
    XRaiseWindow(xapp->display(), m_window);
}

void WindowWrapper::set_border_width(unsigned int width)
{
    XSetWindowBorderWidth(xapp->display(), m_window, width);
}

void WindowWrapper::set_border_color(unsigned long color)
{
    XSetWindowBorder(xapp->display(), m_window, color);
}

Container *WindowWrapper::get_parent()
{
    return m_parent;
}

PrimaryContainer *WindowWrapper::get_primary_container()
{
    return m_primary_container;
}
