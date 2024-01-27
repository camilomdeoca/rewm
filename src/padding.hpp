#ifndef PADDING_HPP
#define PADDING_HPP

#include "drawable_window_wrapper.hpp"
#include <X11/Xlib.h>
#include <vector>

class Padding : public DrawableWindowWrapper {
public:
    enum class BorderSides {
        none,
        all,
        right_left,
        top_bottom
    };

    Padding();
    virtual ~Padding();

    void set_border_sides(BorderSides border_sides);

    void create();

    void set_size(unsigned int width, unsigned int height);
    void set_size_and_position(int x, int y, unsigned int width, unsigned int height);

    void redraw();

    void handle_expose(XExposeEvent *event);

private:
    std::vector<XPoint> get_geometry_for_side_option();

    BorderSides m_border_sides;
};

#endif