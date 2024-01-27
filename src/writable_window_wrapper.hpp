#ifndef WRITABLE_WINDOW_WRAPPER_HPP
#define WRITABLE_WINDOW_WRAPPER_HPP

#include "drawable_window_wrapper.hpp"
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>

class WritableWindowWrapper : public DrawableWindowWrapper {
public:
    WritableWindowWrapper();
    virtual ~WritableWindowWrapper();

    void set_window(Window window);
    void paint(bool pressed = false, bool hovered = false);

    void set_text(std::string text);
    void set_icon(XImage *icon);
private:
    void create_icon_drawables(int width, int height);

    std::string m_text;
    XftDraw *m_draw;
    Pixmap m_icon_pixmap;
    Picture m_icon_picture;
    XImage *m_icon;
};

#endif /* WRITABLEWINDOWWRAPPER_HPP */