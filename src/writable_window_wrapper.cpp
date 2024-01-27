#include "writable_window_wrapper.hpp"
#include "assets.hpp"
#include "wm.hpp"
#include "xapp.hpp"
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

WritableWindowWrapper::WritableWindowWrapper() :
    m_draw(nullptr),
    m_icon_pixmap(None),
    m_icon_picture(None),
    m_icon(nullptr)
{
}

WritableWindowWrapper::~WritableWindowWrapper()
{
    if (m_draw) {
        XftDrawDestroy(m_draw);
    }
    if (m_icon_picture) {
        XRenderFreePicture(xapp->display(), m_icon_picture);
    }
    if (m_icon_pixmap) {
        XFreePixmap(xapp->display(), m_icon_pixmap);
    }
}

void WritableWindowWrapper::set_text(std::string text)
{
    m_text = text;
    if (get_window() != None) {
        redraw();
    }
}

void WritableWindowWrapper::create_icon_drawables(int width, int height)
{
    m_icon_pixmap = XCreatePixmap(xapp->display(), get_window(), width, height, 32);
    XSetForeground(xapp->display(), get_gc(), 0x0);
    XFillRectangle(xapp->display(), m_icon_pixmap, get_gc(), 0, 0, width, height);

    XRenderPictureAttributes pic_attr;
    pic_attr.component_alpha = True;
    m_icon_picture = XRenderCreatePicture(xapp->display(), m_icon_pixmap, xapp->format(32),
            CPComponentAlpha, &pic_attr);
}

void WritableWindowWrapper::set_icon(XImage *icon)
{
    XImage *old_icon = m_icon;
    m_icon = icon;
    if (get_window() != None) {
        if (!old_icon) {
            create_icon_drawables(m_icon->width, m_icon->height);
        }
        XPutImage(xapp->display(), m_icon_pixmap, get_gc(), m_icon,
                0, 0,
                0, 0,
                m_icon->width, m_icon->height);
        redraw();
    }
}

void WritableWindowWrapper::set_window(Window window)
{
    DrawableWindowWrapper::set_window(window);
    m_draw = XftDrawCreate(xapp->display(), get_window(), xapp->visual(32), xapp->colormap(32));
    if (m_icon) {
        create_icon_drawables(m_icon->width, m_icon->height);
        XPutImage(xapp->display(), m_icon_pixmap, get_gc(), m_icon,
                0, 0,
                0, 0,
                m_icon->width, m_icon->height);
    }
}

void WritableWindowWrapper::paint(bool pressed, bool hovered)
{
    DrawableWindowWrapper::paint(pressed, hovered);
    if (m_icon_picture) {
        XRenderComposite(xapp->display(), PictOpOver, m_icon_picture, None, get_window_picture(),
                0, 0,
                0, 0,
                (get_height() - m_icon->height)/2, (get_height() - m_icon->height)/2,
                m_icon->width, m_icon->height);
    }
    if (!m_text.empty()) {
        XftFont *font = assets->get_xft_font(config->get_string("font"));
        XftColor *color = assets->get_xft_color(config->get_string("foreground"));
        XGlyphInfo box;

        /* Get size of text to draw */
        XftTextExtentsUtf8(
                xapp->display(), font, (const FcChar8 *)m_text.c_str(), m_text.length(), &box);

        /* Draw text */
        XftDrawStringUtf8(m_draw,
                          color,
                          font,
                          (get_width() - box.width) / 2 + box.x,
                          (get_height() - box.height) / 2 + box.y,
                          (const FcChar8 *)m_text.c_str(),
                          m_text.length());
    }
}
