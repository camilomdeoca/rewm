#include "button.hpp"
#include "client.hpp"
#include "wm.hpp"
#include "wm_config.hpp"
#include "xapp.hpp"
#include <X11/Xlib.h>

Button::Button() :
    m_hovered(false),
    m_pressed(false)
{
}

Button::~Button()
{
}

void Button::set_pressed(bool pressed)
{
    m_pressed = pressed;
    redraw();
}

void Button::create()
{
    create_window(get_x(), get_y(), get_width(), get_height());
    set_event_mask(ButtonPressMask | ButtonReleaseMask | ExposureMask | EnterWindowMask
                   | LeaveWindowMask);
    show();
}

void Button::handle_button(XButtonEvent *event)
{
    m_pressed = event->type == ButtonPress;
    paint(m_pressed, m_hovered);
    if (event->type == (m_action_time == ActionTime::on_press ? ButtonPress : ButtonRelease)) {
        if (m_press_action) {
            m_press_action(event);
        }
    }
}

void Button::redraw()
{
    paint(m_pressed, m_hovered);
    XFlush(xapp->display());
}

void Button::handle_expose(XExposeEvent *event)
{
    redraw();
}

void Button::handle_enter(XEnterWindowEvent *event)
{
    m_hovered = true;
    redraw();
    get_primary_container()->child_hovered();
}

void Button::handle_leave(XLeaveWindowEvent *event)
{
    m_hovered = false;
    redraw();
}

void Button::set_action(std::function<void(XButtonEvent *)> function, ActionTime action_time)
{
    m_press_action = function;
    m_action_time = action_time;
}
