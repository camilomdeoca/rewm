#ifndef BORDER_HPP
#define BORDER_HPP

#include "drawable_window_wrapper.hpp"
#include "xapp.hpp"
#include <map>

class Client;

class Border : public DrawableWindowWrapper {
public:
    enum Direction : int {
        nw, n, ne,
        w,      e,
        sw, s, se,
        last
    };
    static XApp::CursorType cursor_type_from_direction(Direction direction);

    Border(Direction direction);
    virtual ~Border();

    void create();

    void redraw();

    void handle_button(XButtonEvent *event);
    void handle_expose(XExposeEvent *event);
    void handle_enter(XEnterWindowEvent *event);
private:
    std::vector<XPoint> get_geometry_for_direction(Direction direction);
    Direction m_direction;
};

#endif
