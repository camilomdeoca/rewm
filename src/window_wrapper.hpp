#ifndef WINDOW_WRAPPER_HPP
#define WINDOW_WRAPPER_HPP

#include <X11/Xlib.h>
#include <unordered_set>

class Container;
class PrimaryContainer;
class Client;

class WindowWrapper {
public:
    WindowWrapper(Window window = None);
    virtual ~WindowWrapper();

    virtual void create() = 0;
    void create_window(int x = 0, int y = 0, unsigned int width = 1, unsigned int height = 1);
    Window get_window();
    virtual void set_window(Window m_window);
    void set_parent(Container *parent);
    void set_primary_container(PrimaryContainer *primary_container);
    void set_event_mask(unsigned long event_mask);
    void set_cursor(Cursor cursor);
    void set_window_deleted();
    int get_x();
    int get_y();
    unsigned int get_width();
    unsigned int get_height();

    virtual void set_width(unsigned int width);
    virtual void set_height(unsigned int height);
    virtual void set_size(unsigned int width, unsigned int height);
    virtual void set_position(int x, int y);
    virtual void set_size_and_position(int x, int y, unsigned int width, unsigned int height);
    void show();
    void hide();
    void rise();
    void set_border_width(unsigned int width);
    void set_border_color(unsigned long color);
    Container *get_parent();
    PrimaryContainer *get_primary_container();

    virtual void handle_map_request(XMapRequestEvent *event) {};
    virtual void handle_button(XButtonEvent *event) {};
    //virtual void handle_button_press(XButtonPressedEvent *event) {};
    //virtual void handle_button_release(XButtonReleasedEvent *event) {};
    virtual void handle_expose(XExposeEvent *event) {};
    virtual void handle_motion(XMotionEvent *event) {};
    virtual void handle_destroy_notify(XDestroyWindowEvent *event) {};
    virtual void handle_enter(XEnterWindowEvent *event) {};
    virtual void handle_leave(XLeaveWindowEvent *event) {};
    virtual void handle_client_message(XClientMessageEvent *event) {};
    virtual void handle_property_notify(XPropertyEvent *event) {};
    virtual void handle_unmap_notify(XUnmapEvent *event) {};

private:
    Window m_window;
    Container *m_parent;
    PrimaryContainer *m_primary_container;
    int m_x, m_y;
    unsigned int m_width, m_height;

    unsigned long m_event_mask;
    Cursor m_cursor;
};

#endif
