#ifndef MANAGED_WINDOW_HPP
#define MANAGED_WINDOW_HPP

#include "client_component.hpp"
#include <X11/Xlib.h>
#include <string>

class Client;

class ManagedWindow : public ClientComponent {
public:
    ManagedWindow(Window window);
    virtual ~ManagedWindow();

    void create();

    std::string get_title();
    XImage *get_icon();

    void handle_enter(XEnterWindowEvent *event);
    void handle_button(XButtonEvent *event);
    void handle_client_message(XClientMessageEvent *event);
    void handle_property_notify(XPropertyEvent *event);
    void handle_destroy_notify(XDestroyWindowEvent *event);
    void handle_unmap_notify(XUnmapEvent *event);
};

#endif