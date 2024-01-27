#include "assets.hpp"
#include "xapp.hpp"
#include <iostream>
#include <string>
#include <X11/Xft/Xft.h>
#include <X11/xpm.h>
#include <X11/Xutil.h>

Assets *assets;

Assets::Assets()
{
}

Assets::~Assets()
{
    for (auto [path, image] : m_images) {
        XDestroyImage(image);
    }

    for (auto [code, font] : m_xft_fonts) {
        XftFontClose(xapp->display(), font);
    }

    for (auto [hex_name, color] : m_xft_colors) {
        XftColorFree(xapp->display(), xapp->visual(32), xapp->colormap(32), &color);
    }
}

XImage *Assets::get_image_from_path(std::string path)
{
    XImage *x_image, *x_mask;
    XpmAttributes xpm_attributes;
    xpm_attributes.visual = xapp->visual(32);
    xpm_attributes.colormap = xapp->colormap(32);
    xpm_attributes.depth = 32;
    xpm_attributes.valuemask = XpmVisual | XpmColormap | XpmDepth;

    std::map<std::string, XImage *>::iterator search = m_images.find(path);
    if (search == m_images.end()) {
        int status =
                XpmReadFileToImage(xapp->display(), path.c_str(), &x_image, &x_mask, &xpm_attributes);
        if (status)
            std::cerr << "Error loading image from: '" << path << "', with error: " << status
                      << std::endl;
        XpmFreeAttributes(&xpm_attributes);
        if (x_mask != 0)
            XDestroyImage(x_mask);
        m_images.insert({path, x_image});
    } else {
        x_image = search->second;
    }

    return x_image;
}

XftFont *Assets::get_xft_font(std::string font_name)
{
    XftFont *font;

    std::map<std::string, XftFont *>::iterator search = m_xft_fonts.find(font_name);
    if (search == m_xft_fonts.end()) {
        font = XftFontOpenName(xapp->display(), xapp->screen(), font_name.c_str());
        if (font != nullptr) {
            m_xft_fonts.insert({font_name, font});
        } else {
            std::cerr << "Error loading font: '" << font_name << "'" << std::endl;
            return nullptr;
        }
    } else {
        font = search->second;
    }
    return font;
}

XftColor *Assets::get_xft_color(std::string hex_color)
{
    XftColor color;

    std::map<std::string, XftColor>::iterator search = m_xft_colors.find(hex_color);
    if (search == m_xft_colors.end()) { /* If color was not created previously */
        auto [iterator_to_inserted, was_inserted] = m_xft_colors.insert({hex_color, color});

        int status = XftColorAllocName(xapp->display(),
                                       xapp->visual(32),
                                       xapp->colormap(32),
                                       hex_color.c_str(),
                                       &iterator_to_inserted->second);

        if (!status) {
            std::cerr << "Error allocating color: " << hex_color << std::endl;
            m_xft_colors.erase(iterator_to_inserted);
            return nullptr;
        } else {
            return &iterator_to_inserted->second;
        }
    } else {
        return &search->second;
    }
}
