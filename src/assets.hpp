#ifndef ASSETS_HPP
#define ASSETS_HPP

#include "wm_config.hpp"
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <map>
#include <string>

class Assets {
public:
    Assets();
    virtual ~Assets();

    XImage *get_image_from_path(std::string path);
    XftFont *get_xft_font(std::string font_name);
    XftColor *get_xft_color(std::string color);
private:
    std::map<std::string, XImage *> m_images;
    std::map<std::string, XftFont *> m_xft_fonts;
    std::map<std::string, XftColor> m_xft_colors;
};

extern Assets *assets;

#endif