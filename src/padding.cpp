#include "padding.hpp"
#include "drawable_window_wrapper.hpp"
#include "window_wrapper.hpp"
#include "wm.hpp"
#include "wm_config.hpp"
#include <stdexcept>
#include <vector>
#include <X11/X.h>
#include <X11/Xlib.h>

Padding::Padding() :
    m_border_sides(BorderSides::all)
{
}

Padding::~Padding()
{
}

void Padding::create()
{
    create_window(get_x(), get_y(), get_width(), get_height());
    set_event_mask(ExposureMask);
    show();
}

void Padding::set_border_sides(BorderSides border_sides) {
    m_border_sides = border_sides;
}

std::vector<XPoint> Padding::get_geometry_for_side_option() {
    int bevel_size = config->get_int("window.border.bevel_size");

    switch (m_border_sides) {
    case BorderSides::all:
        return {
                {                 0,                   0},
                {                 0, (short)get_height()},
                {(short)get_width(), (short)get_height()},
                {(short)get_width(),                   0}
        };
        break;
    case BorderSides::none:
        /* TODO: Make it so borders arent even rendered */
        return {
                {             (short)(-bevel_size),             (short)(-bevel_size)},
                {             (short)(-bevel_size), (short)(get_height()+bevel_size)},
                {(short)(get_width() + bevel_size), (short)(get_height()+bevel_size)},
                {(short)(get_width() + bevel_size),             (short)(-bevel_size)}
        };
        break;
    case BorderSides::right_left:
        /* TODO: Make it so only shown borders are rendered */
        return {
                {                 0,             (short)(-bevel_size)},
                {                 0, (short)(get_height()+bevel_size)},
                {(short)get_width(), (short)(get_height()+bevel_size)},
                {(short)get_width(),             (short)(-bevel_size)}
        };
        break;
    case BorderSides::top_bottom:
        return {
                {             (short)(-bevel_size),                   0},
                {             (short)(-bevel_size), (short)get_height()},
                {(short)(get_width() + bevel_size), (short)get_height()},
                {(short)(get_width() + bevel_size),                   0}
        };
        break;
    default:
        throw std::runtime_error("This can't happen.");
        break;
    }
}

void Padding::set_size(unsigned int width, unsigned int height)
{
    DrawableWindowWrapper::set_size(width, height);
    set_geometry(get_geometry_for_side_option());
}

void Padding::set_size_and_position(int x, int y, unsigned int width, unsigned int height)
{
    DrawableWindowWrapper::set_size_and_position(x, y, width, height);
    set_geometry(get_geometry_for_side_option());
}

void Padding::redraw()
{
    paint();
    XFlush(xapp->display());
}

void Padding::handle_expose(XExposeEvent *event)
{
    redraw();
}
