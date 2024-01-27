#ifndef WM_HPP
#define WM_HPP

#include "wm_root_window.hpp"
#include "xapp.hpp"
#include "client.hpp"
#include "monitor.hpp"
#include "window_wrapper.hpp"

#include <X11/Xlib.h>
#include <unordered_map>
#include <unordered_set>

class WindowManager : public XApp {
public:
    WindowManager();
    virtual ~WindowManager();

    void setup();
    void main_loop();
    void exit();

    void add_window_wrapper(WindowWrapper *window_wrapper);
    void remove_window_wrapper(WindowWrapper *window_wrapper);

    void focus_monitor(Monitor *monitor);
    void set_window_moving(bool value);
    void set_window_resizing(bool value);
    void draw_rectangle(int x, int y, unsigned int width, unsigned int height);
    void send_message(Client *client, Atom message);

    // Gets monitor from position in screen
    Monitor *get_monitor_from_position(int x, int y);

    Monitor *get_focused_monitor();

    // Gets the Client object from the managed window XID
    Client *get_client(Window window);

private:
    void map_window(Window window);
    void handle_event(XEvent *ev);

    Window m_check_window;
    std::unordered_map<Window, WindowWrapper *> m_window_wrappers;
    std::unordered_map<Window, Client *> m_clients; // The key is the managed window XID
    bool m_exit, m_window_moving, m_window_resizing;
    GC m_rectangle_gc;
    XRectangle m_old_rectangle;
    std::list<Monitor *> m_monitors;
    Monitor *m_focused_monitor;
    WMRootWindow *m_root_window;
};

extern WindowManager *wm;

#endif
