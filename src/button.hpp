#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "writable_window_wrapper.hpp"
#include <X11/Xlib.h>
#include <functional>

class Client;

class Button : public WritableWindowWrapper {
public:
    enum class ActionTime {
        on_press,
        on_release
    };

    Button();
    virtual ~Button();

    void set_pressed(bool pressed);

    void create();

    void redraw();

    void handle_button(XButtonEvent *event);
    void handle_expose(XExposeEvent *event);
    void handle_enter(XEnterWindowEvent *event);
    void handle_leave(XLeaveWindowEvent *eveny);

    void set_action(std::function<void(XButtonEvent *)> function, ActionTime action_time);
private:
    bool m_hovered, m_pressed;
    ActionTime m_action_time;
    std::function<void(XButtonEvent *)> m_press_action;
};

#endif