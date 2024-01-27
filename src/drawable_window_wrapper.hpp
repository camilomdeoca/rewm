#ifndef DRAWABLE_WINDOW_WRAPPER_HPP
#define DRAWABLE_WINDOW_WRAPPER_HPP

#include "window_wrapper.hpp"
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <vector>
#include <string>

class DrawableWindowWrapper : public WindowWrapper {
public:
    DrawableWindowWrapper();
    virtual ~DrawableWindowWrapper();

    virtual void set_window(Window window);

    virtual void set_size(unsigned int width, unsigned int height);
    virtual void set_size_and_position(int x, int y, unsigned int width, unsigned int height);

    void initialize_gcs();

    virtual void redraw() = 0;
    virtual void paint(bool pressed = false, bool hovered = false);
    void set_geometry(std::vector<XPoint> geometry);
    void set_image(std::string path);

    Picture get_window_picture();
    GC get_gc();

private:
    void initialize_drawables();
    void resize_drawables();
    void destroy_drawables();
    void destroy_gcs();
    void draw_bevel(unsigned int size, bool pressed = false, bool hovered = false);

    XImage *m_image;
    std::vector<XPoint> m_window_geometry;
    Pixmap m_background_tile_pixmap, m_overlay_pixmap;
    Picture m_window_picture, m_background_tile_picture, m_overlay_picture;
    GC m_gc;
};

#endif