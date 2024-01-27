#include "border.hpp"
#include "wm.hpp"
#include "wm_config.hpp"
#include "xapp.hpp"
#include <X11/X.h>
#include <X11/Xlib.h>

Border::Border(Direction direction) :
    m_direction(direction)
{
    int corner_size = config->get_int("window.border.corner_size");
    set_size(corner_size, corner_size);
}

Border::~Border()
{
}

void Border::create()
{
    create_window(get_x(), get_y(), get_width(), get_height());
    set_event_mask(ButtonPressMask | ExposureMask | EnterWindowMask);
    set_cursor(xapp->cursor(Border::cursor_type_from_direction(m_direction)));
    set_geometry(get_geometry_for_direction(m_direction));

    show();
}

XApp::CursorType Border::cursor_type_from_direction(Direction direction)
{
    XApp::CursorType cursor_type;
    switch (direction) {
    case Border::Direction::nw: cursor_type = XApp::CursorType::nw; break;
    case Border::Direction::ne: cursor_type = XApp::CursorType::ne; break;
    case Border::Direction::sw: cursor_type = XApp::CursorType::sw; break;
    case Border::Direction::se: cursor_type = XApp::CursorType::se; break;
    case Border::Direction::n: cursor_type = XApp::CursorType::n; break;
    case Border::Direction::w: cursor_type = XApp::CursorType::w; break;
    case Border::Direction::s: cursor_type = XApp::CursorType::s; break;
    case Border::Direction::e: cursor_type = XApp::CursorType::e; break;
    default: break;
    }
    return cursor_type;
}

void Border::handle_button(XButtonEvent *event)
{
    get_primary_container()->child_pressed();
    if (event->type == ButtonPress) {
        static_cast<Client *>(get_primary_container())->begin_drag_resize(event, m_direction);
    }
}

void Border::redraw()
{
    paint();
    XFlush(xapp->display());
}

void Border::handle_expose(XExposeEvent *event)
{
    redraw();
}

void Border::handle_enter(XEnterWindowEvent *event)
{
    get_primary_container()->child_hovered();
}

std::vector<XPoint> Border::get_geometry_for_direction(Direction direction)
{
    int border_size = config->get_int("window.border.size");
    int corner_size = config->get_int("window.border.corner_size");
    std::vector<XPoint> geometry;
    switch (direction) {
    case Direction::nw:
        geometry = {
                {                   0,                    0},
                {                   0, (short)(corner_size)},
                {(short)(border_size), (short)(corner_size)},
                {(short)(border_size), (short)(border_size)},
                {(short)(corner_size), (short)(border_size)},
                {(short)(corner_size),                    0}
        };
        break;
    case Direction::ne:
        geometry = {
                {                                 0,                    0},
                {                                 0, (short)(border_size)},
                {(short)(corner_size - border_size), (short)(border_size)},
                {(short)(corner_size - border_size), (short)(corner_size)},
                {              (short)(corner_size), (short)(corner_size)},
                {              (short)(corner_size),                    0}
        };
        break;
    case Direction::sw:
        geometry = {
                {                   0,                                  0},
                {                   0,               (short)(corner_size)},
                {(short)(corner_size),               (short)(corner_size)},
                {(short)(corner_size), (short)(corner_size - border_size)},
                {(short)(border_size), (short)(corner_size - border_size)},
                {(short)(border_size),                                  0}
        };
        break;
    case Direction::se:
        geometry = {
                {                                 0, (short)(corner_size - border_size)},
                {                                 0,               (short)(corner_size)},
                {              (short)(corner_size),               (short)(corner_size)},
                {              (short)(corner_size),                                  0},
                {(short)(corner_size - border_size),                                  0},
                {(short)(corner_size - border_size), (short)(corner_size - border_size)}
        };
        break;
    /* If window is not a corner use the default window geometry (window sized rectangle) */
    default:
        break;
    }
    return geometry;
}
