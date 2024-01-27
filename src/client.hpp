#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "primary_container.hpp"
#include "managed_window.hpp"
#include "monitor.hpp"
#include "titlebar.hpp"
#include "border.hpp"
#include "button.hpp"
#include "workspace.hpp"

class Client : public PrimaryContainer {
public:
    enum NetState : int {
        modal                  = (1 << 0),
        sticky                 = (1 << 1),
        maximized_vertically   = (1 << 2),
        maximized_horizontally = (1 << 3),
        shaded                 = (1 << 4),
        skip_taskbar           = (1 << 5),
        skip_pager             = (1 << 6),
        hidden                 = (1 << 7),
        fullscreen             = (1 << 8),
        above                  = (1 << 9),
        below                  = (1 << 10),
        demands_atention       = (1 << 11),
        focused                = (1 << 12)
    };

    enum class WindowType {
        combo,
        desktop,
        dialog,
        dnd,
        dock,
        dropdown_menu,
        menu,
        normal,
        notification,
        popup_menu,
        splash,
        toolbar,
        tooltip,
        utility,
    };

    Client(Window window, XWindowAttributes window_attributes, Workspace *workspace);
    virtual ~Client();

    void create();

    void handle_button(XButtonEvent *event);
    void handle_motion(XMotionEvent *event);

    ManagedWindow *get_managed();
    Workspace *get_workspace();
    void set_workspace(Workspace *workspace);

    void set_main_container(Container *container);
    void set_size(unsigned int width, unsigned int height);
    void set_size_and_position(int x, int y, unsigned int width, unsigned int height);
    void raise();
    void focus();
    void unfocus();
    void close();

    bool is_fullscreen();

    void toggle_fullscreen();
    void maximize();
    void tile(int x, int y, unsigned int width, unsigned int height);
    void unmaximize_or_untile();
    void toggle_maximize();
    std::string get_title();
    XImage *get_icon();
    void update_title();
    void update_icon();
    void update_size_hints();

    void begin_drag_move(XButtonEvent *event);
    void drag_move(XMotionEvent *event);
    void end_drag_move(XButtonEvent *event);
    void begin_drag_resize(XButtonEvent *event, Border::Direction direction);
    void drag_resize(XMotionEvent *event);
    void end_drag_resize(XButtonEvent *event);

    void child_hovered();
    void child_pressed();

private:
    void get_drag_resize_parameters(int mouse_x, int mouse_y, int *x, int *y, int *width, int *height);
    void apply_size_hints(int *x, int *y, int *width, int *height);
    bool stick_to_sides(int mouse_x, int mouse_y, int *x, int *y, unsigned int *width, unsigned int *height);

    Workspace *m_workspace;
    Container *m_main_container;
    ManagedWindow *m_managed_window;
    TitleBar *m_titlebar;
    Button *m_button;
    std::string m_title;
    XImage *m_icon;
    static Time m_time_last_motion;
    int m_unmaximized_or_untiled_x, m_unmaximized_or_untiled_y;
    unsigned int m_unmaximized_or_untiled_width, m_unmaximized_or_untiled_height;
    bool m_moving, m_resizing, m_maximized, m_tiled, m_focused;
    static Window m_resize_constraint_window;
    static Border::Direction m_resizing_direction;
    struct ResizeConstraints {
        int min_width, max_width, min_height, max_height;
        int width_inc, height_inc;
    } m_constraints;
    static int m_before_x, m_before_y, m_before_mouse_x, m_before_mouse_y;
    static unsigned int m_before_width, m_before_height;
    int m_net_state; /* Uses bitmask from Client::net_state enum */
    WindowType m_window_type;
};

#endif