#ifndef WM_ROOT_WINDOW_HPP
#define WM_ROOT_WINDOW_HPP

#include "window_wrapper.hpp"

class WMRootWindow : public WindowWrapper {
public:
    WMRootWindow(Window root_window);
    virtual ~WMRootWindow();

    void create();

    void handle_client_message(XClientMessageEvent *event);
};

#endif