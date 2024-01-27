#include "drawable_window_wrapper.hpp"
#include "assets.hpp"
#include "window_wrapper.hpp"
#include "xapp.hpp"
#include <array>
#include <string>
#include <vector>
#include <X11/extensions/Xrender.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>

DrawableWindowWrapper::DrawableWindowWrapper()
{
    /* TODO: set a constant with default image path */
    set_image("images/lightblue.xpm");
}

DrawableWindowWrapper::~DrawableWindowWrapper()
{
    destroy_drawables();
    destroy_gcs();
}

void DrawableWindowWrapper::set_window(Window window)
{
    WindowWrapper::set_window(window);

    initialize_gcs();
    initialize_drawables();
}

void DrawableWindowWrapper::set_size(unsigned int width, unsigned int height)
{
    bool size_changed = width != get_width() || height != get_height();
    WindowWrapper::set_size(width, height);
    if (get_window() != None && size_changed) {
        resize_drawables();
    }
}

void DrawableWindowWrapper::set_size_and_position(int x,
                                                  int y,
                                                  unsigned int width,
                                                  unsigned int height)
{
    bool size_changed = width != get_width() || height != get_height();
    WindowWrapper::set_size_and_position(x, y, width, height);
    if (get_window() != None && size_changed) {
        resize_drawables();
    }
}

void DrawableWindowWrapper::initialize_gcs()
{
    m_gc = XCreateGC(xapp->display(), get_window(), 0, nullptr);
}

void DrawableWindowWrapper::initialize_drawables()
{
    m_background_tile_pixmap =
            XCreatePixmap(xapp->display(), get_window(), m_image->width, m_image->height, 32);

    m_overlay_pixmap = XCreatePixmap(xapp->display(), get_window(), get_width(), get_height(), 32);

    XPutImage(xapp->display(),
              m_background_tile_pixmap,
              m_gc,
              m_image,
              0,
              0,
              0,
              0,
              m_image->width,
              m_image->height);

    XRenderPictureAttributes picture_attributes;
    picture_attributes.component_alpha = True;
    picture_attributes.repeat = True;

    m_window_picture = XRenderCreatePicture(
            xapp->display(), get_window(), xapp->format(32), CPComponentAlpha, &picture_attributes);

    m_background_tile_picture = XRenderCreatePicture(xapp->display(),
                                                     m_background_tile_pixmap,
                                                     xapp->format(32),
                                                     CPRepeat | CPComponentAlpha,
                                                     &picture_attributes);

    m_overlay_picture = XRenderCreatePicture(
            xapp->display(), m_overlay_pixmap, xapp->format(32), CPComponentAlpha, &picture_attributes);
}

void DrawableWindowWrapper::resize_drawables()
{
    XRenderFreePicture(xapp->display(), m_overlay_picture);
    XFreePixmap(xapp->display(), m_overlay_pixmap);

    m_overlay_pixmap = XCreatePixmap(xapp->display(), get_window(), get_width(), get_height(), 32);

    XRenderPictureAttributes picture_attributes;
    picture_attributes.component_alpha = True;

    m_overlay_picture = XRenderCreatePicture(
            xapp->display(), m_overlay_pixmap, xapp->format(32), CPComponentAlpha, &picture_attributes);
}

void DrawableWindowWrapper::destroy_drawables()
{
    XRenderFreePicture(xapp->display(), m_overlay_picture);
    XRenderFreePicture(xapp->display(), m_background_tile_picture);
    XRenderFreePicture(xapp->display(), m_window_picture);
    XFreePixmap(xapp->display(), m_overlay_pixmap);
    XFreePixmap(xapp->display(), m_background_tile_pixmap);
}

void DrawableWindowWrapper::destroy_gcs()
{
    XFreeGC(xapp->display(), m_gc);
}

unsigned long premultiplied_alpha(unsigned long color)
{
    return (color & 0xff000000) | ((color & 0x00ffffff) * ((color & 0xff000000) >> 24) / 0xff);
}

/*
 *  The points should be going anticlockwise and the angle between the segments defined by the
 *  points should be 90º always (between adjacent segments).
 */
void DrawableWindowWrapper::draw_bevel(unsigned int size, bool pressed, bool hovered)
{
    std::vector<std::array<XPoint, 4>> shine_polygons;
    std::vector<std::array<XPoint, 4>> shadow_polygons;
    std::vector<XSegment> light_shine_segments;
    std::vector<XSegment> medium_segments;
    std::vector<XSegment> dark_shadow_segments;
    if (m_window_geometry.size() > 0) {
        shine_polygons.reserve(m_window_geometry.size() / 2);
        shadow_polygons.reserve(m_window_geometry.size() / 2);
        light_shine_segments.reserve(m_window_geometry.size() / 4);
        medium_segments.reserve(m_window_geometry.size() / 2);
        dark_shadow_segments.reserve(m_window_geometry.size() / 4);

        XPoint past_past, past, actual, next;
        past_past = m_window_geometry[m_window_geometry.size() - 3];
        past = m_window_geometry[m_window_geometry.size() - 2];
        actual = m_window_geometry[m_window_geometry.size() - 1];
        size_t i = 0;
        while (i < m_window_geometry.size() && i >= 0) {
            next = m_window_geometry[i];
            if (past.y == actual.y) {    /* horizontal */
                if (actual.x < past.x) { /* left */
                    shine_polygons.push_back(std::array<XPoint, 4>());
                    if (next.y > actual.y) { /* left down */
                        shine_polygons.back()[1] = {(short)(actual.x + 1), (short)(actual.y)};
                        shine_polygons.back()[2] = {(short)(actual.x + size + 1),
                                                    (short)(actual.y + size)};
                        light_shine_segments.push_back(XSegment());
                        light_shine_segments.back() = {(short)(actual.x),
                                                       (short)(actual.y),
                                                       (short)(actual.x + size - 1),
                                                       (short)(actual.y + size - 1)};
                    } else { /* left up */
                        shine_polygons.back()[1] = {(short)(actual.x), (short)(actual.y)};
                        shine_polygons.back()[2] = {(short)(actual.x - size),
                                                    (short)(actual.y + size)};
                        medium_segments.push_back(XSegment());
                        medium_segments.back() = {(short)(actual.x - 1),
                                                  (short)(actual.y),
                                                  (short)(actual.x - size),
                                                  (short)(actual.y + size - 1)};
                    }
                    if (past_past.y > past.y) { /* up left */
                        shine_polygons.back()[3] = {(short)(past.x - size - 1),
                                                    (short)(past.y + size)};
                        shine_polygons.back()[0] = {(short)(past.x - 1), (short)(past.y)};
                    } else { /* down left */
                        shine_polygons.back()[3] = {(short)(past.x + size), (short)(past.y + size)};
                        shine_polygons.back()[0] = {(short)(past.x), (short)(past.y)};
                    }
                } else { /* right */
                    shadow_polygons.push_back(std::array<XPoint, 4>());
                    if (next.y > actual.y) { /* right down */
                        shadow_polygons.back()[1] = {(short)(actual.x - 1), (short)(actual.y)};
                        shadow_polygons.back()[2] = {(short)(actual.x + size - 1),
                                                     (short)(actual.y - size)};
                        medium_segments.push_back(XSegment());
                        medium_segments.back() = {(short)(actual.x),
                                                  (short)(actual.y - 1),
                                                  (short)(actual.x + size - 1),
                                                  (short)(actual.y - size)};
                    } else { /* right up */
                        shadow_polygons.back()[1] = {(short)(actual.x), (short)(actual.y)};
                        shadow_polygons.back()[2] = {(short)(actual.x - size),
                                                     (short)(actual.y - size)};
                        dark_shadow_segments.push_back(XSegment());
                        dark_shadow_segments.back() = {(short)(actual.x - 1),
                                                       (short)(actual.y - 1),
                                                       (short)(actual.x - size),
                                                       (short)(actual.y - size)};
                    }
                    if (past_past.y > past.y) { /* up right */
                        shadow_polygons.back()[3] = {(short)(past.x - size + 1),
                                                     (short)(past.y - size)};
                        shadow_polygons.back()[0] = {(short)(past.x + 1), (short)(past.y)};
                    } else { /* down right */
                        shadow_polygons.back()[3] = {(short)(past.x + size),
                                                     (short)(past.y - size)};
                        shadow_polygons.back()[0] = {(short)(past.x), (short)(past.y)};
                    }
                }
            } else {                     /* vertical */
                if (actual.y > past.y) { /* down */
                    shine_polygons.push_back(std::array<XPoint, 4>());
                    if (next.x > actual.x) { /* down right */
                        shine_polygons.back()[1] = {(short)(actual.x), (short)(actual.y - 1)};
                        shine_polygons.back()[2] = {(short)(actual.x + size),
                                                    (short)(actual.y - size - 1)};
                        medium_segments.push_back(XSegment());
                        medium_segments.back() = {(short)(actual.x),
                                                  (short)(actual.y - 1),
                                                  (short)(actual.x + size - 1),
                                                  (short)(actual.y - size)};
                    } else { /* down left */
                        shine_polygons.back()[1] = {(short)(actual.x), (short)(actual.y - 1)};
                        shine_polygons.back()[2] = {(short)(actual.x + size),
                                                    (short)(actual.y + size - 1)};
                        light_shine_segments.push_back(XSegment());
                        light_shine_segments.back() = {(short)(actual.x),
                                                       (short)(actual.y),
                                                       (short)(actual.x + size - 1),
                                                       (short)(actual.y + size - 1)};
                    }
                    if (past_past.x > past.x) { /* left down */
                        shine_polygons.back()[3] = {(short)(past.x + size), (short)(past.y + size)};
                        shine_polygons.back()[0] = {(short)(past.x), (short)(past.y)};
                    } else { /*right down*/
                        shine_polygons.back()[3] = {(short)(past.x + size), (short)(past.y - size)};
                        shine_polygons.back()[0] = {(short)(past.x), (short)(past.y)};
                    }
                } else { /* up */
                    shadow_polygons.push_back(std::array<XPoint, 4>());
                    if (next.x > actual.x) { /* up right */
                        shadow_polygons.back()[1] = {(short)(actual.x), (short)(actual.y)};
                        shadow_polygons.back()[2] = {(short)(actual.x - size),
                                                     (short)(actual.y - size)};
                        dark_shadow_segments.push_back(XSegment());
                        dark_shadow_segments.back() = {(short)(actual.x - 1),
                                                       (short)(actual.y - 1),
                                                       (short)(actual.x - size),
                                                       (short)(actual.y - size)};
                    } else { /* up left */
                        shadow_polygons.back()[1] = {(short)(actual.x), (short)(actual.y)};
                        shadow_polygons.back()[2] = {(short)(actual.x - size),
                                                     (short)(actual.y + size)};
                        medium_segments.push_back(XSegment());
                        medium_segments.back() = {(short)(actual.x - 1),
                                                  (short)(actual.y),
                                                  (short)(actual.x - size),
                                                  (short)(actual.y + size - 1)};
                    }
                    if (past_past.x > past.x) { /* left up */
                        shadow_polygons.back()[3] = {(short)(past.x - size),
                                                     (short)(past.y + size - 1)};
                        shadow_polygons.back()[0] = {(short)(past.x), (short)(past.y - 1)};
                    } else { /*right up*/
                        shadow_polygons.back()[3] = {(short)(past.x - size),
                                                     (short)(past.y - size - 1)};
                        shadow_polygons.back()[0] = {(short)(past.x), (short)(past.y - 1)};
                    }
                }
            }

            past_past = past;
            past = actual;
            actual = next;
            i++;
        }
    } else {
        shine_polygons.resize(2);
        /* Top bevel polygon */
        shine_polygons[0][0] = {1, 0};
        shine_polygons[0][1] = {(short)(size + 1), (short)size};
        shine_polygons[0][2] = {(short)(get_width() - size - 1), (short)size};
        shine_polygons[0][3] = {(short)(get_width() - 1), 0};

        /* Left bevel polygon */
        shine_polygons[1][0] = {0, 0};
        shine_polygons[1][1] = {0, (short)(get_height() - 1)};
        shine_polygons[1][2] = {(short)size, (short)(get_height() - size - 1)};
        shine_polygons[1][3] = {(short)size, (short)size};

        shadow_polygons.resize(2);
        /* Bottom bevel polygon */
        shadow_polygons[0][0] = {0, (short)(get_height())};
        shadow_polygons[0][1] = {(short)get_width(), (short)(get_height())};
        shadow_polygons[0][2] = {(short)(get_width() - size), (short)(get_height() - size)};
        shadow_polygons[0][3] = {(short)size, (short)(get_height() - size)};

        /* Right bevel polygon */
        shadow_polygons[1][0] = {(short)(get_width()), (short)(get_height() - 1)};
        shadow_polygons[1][1] = {(short)(get_width()), 0};
        shadow_polygons[1][2] = {(short)(get_width() - size), (short)size};
        shadow_polygons[1][3] = {(short)(get_width() - size), (short)(get_height() - size - 1)};

        light_shine_segments.resize(1);
        /* Top left bevel segment */
        light_shine_segments[0] = {0, 0, (short)(size - 1), (short)(size - 1)};

        medium_segments.resize(2);
        /* Bottom left bevel segment */
        medium_segments[0] = {
                0, (short)(get_height() - 1), (short)(size - 1), (short)(get_height() - size)};

        /* Top right bevel segment */
        medium_segments[1] = {
                (short)(get_width() - 1), 0, (short)(get_width() - size), (short)(size - 1)};

        dark_shadow_segments.resize(1);
        /* Bottom right bevel segment */
        dark_shadow_segments[0] = {(short)(get_width() - 1),
                                   (short)(get_height() - 1),
                                   (short)(get_width() - size),
                                   (short)(get_height() - size)};
    }

    if (pressed) {
        std::swap(shine_polygons, shadow_polygons);
        std::swap(dark_shadow_segments, light_shine_segments);
    }

    if (!hovered) {
        XSetForeground(
                xapp->display(),
                m_gc,
                premultiplied_alpha(config->get_color_ulong("window.colors.normal_overlay")));
    } else {
        XSetForeground(
                xapp->display(),
                m_gc,
                premultiplied_alpha(config->get_color_ulong("window.colors.hovered_overlay")));
    }
    XFillRectangle(xapp->display(), m_overlay_pixmap, m_gc, 0, 0, get_width(), get_height());

    XSetForeground(xapp->display(),
                   m_gc,
                   premultiplied_alpha(config->get_color_ulong("window.colors.shine")));
    for (std::array<XPoint, 4> polygon : shine_polygons) {
        XFillPolygon(xapp->display(),
                     m_overlay_pixmap,
                     m_gc,
                     polygon.data(),
                     polygon.size(),
                     Complex,
                     CoordModeOrigin);
    }

    XSetForeground(xapp->display(),
                   m_gc,
                   premultiplied_alpha(config->get_color_ulong("window.colors.shadow")));
    for (std::array<XPoint, 4> polygon : shadow_polygons) {
        XFillPolygon(xapp->display(),
                     m_overlay_pixmap,
                     m_gc,
                     polygon.data(),
                     polygon.size(),
                     Complex,
                     CoordModeOrigin);
    }

    XSetForeground(xapp->display(),
                   m_gc,
                   premultiplied_alpha(config->get_color_ulong("window.colors.light_shine")));
    XDrawSegments(xapp->display(),
                  m_overlay_pixmap,
                  m_gc,
                  light_shine_segments.data(),
                  light_shine_segments.size());

    XSetForeground(xapp->display(),
                   m_gc,
                   premultiplied_alpha(config->get_color_ulong("window.colors.medium")));
    XDrawSegments(
            xapp->display(), m_overlay_pixmap, m_gc, medium_segments.data(), medium_segments.size());

    XSetForeground(xapp->display(),
                   m_gc,
                   premultiplied_alpha(config->get_color_ulong("window.colors.dark_shadow")));
    XDrawSegments(xapp->display(),
                  m_overlay_pixmap,
                  m_gc,
                  dark_shadow_segments.data(),
                  dark_shadow_segments.size());
}

void DrawableWindowWrapper::paint(bool pressed, bool hovered)
{
    draw_bevel(config->get_int("window.border.bevel_size"), pressed, hovered);
    XRenderComposite(xapp->display(),
                     PictOpOver,
                     m_background_tile_picture,
                     None,
                     m_window_picture,
                     0,
                     0,
                     0,
                     0,
                     0,
                     0,
                     get_width(),
                     get_height());
    XRenderComposite(xapp->display(),
                     PictOpOver,
                     m_overlay_picture,
                     None,
                     m_window_picture,
                     0,
                     0,
                     0,
                     0,
                     0,
                     0,
                     get_width(),
                     get_height());
}

void DrawableWindowWrapper::set_geometry(std::vector<XPoint> geometry)
{
    m_window_geometry = geometry;
}

void DrawableWindowWrapper::set_image(std::string path)
{
    m_image = assets->get_image_from_path(path);
    if (get_window() != None) {
        XRenderFreePicture(xapp->display(), m_background_tile_picture);
        XFreePixmap(xapp->display(), m_background_tile_pixmap);

        m_background_tile_pixmap =
                XCreatePixmap(xapp->display(), get_window(), m_image->width, m_image->height, 32);

        XPutImage(xapp->display(),
                  m_background_tile_pixmap,
                  m_gc,
                  m_image,
                  0,
                  0,
                  0,
                  0,
                  m_image->width,
                  m_image->height);

        XRenderPictureAttributes picture_attributes;
        picture_attributes.component_alpha = True;
        picture_attributes.repeat = True;
        m_background_tile_picture = XRenderCreatePicture(xapp->display(),
                                                         m_background_tile_pixmap,
                                                         xapp->format(32),
                                                         CPRepeat | CPComponentAlpha,
                                                         &picture_attributes);

        redraw();
    }
}

Picture DrawableWindowWrapper::get_window_picture()
{
    return m_window_picture;
}

GC DrawableWindowWrapper::get_gc()
{
    return m_gc;
}
