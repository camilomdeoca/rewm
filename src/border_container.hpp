#ifndef BORDER_CONTAINER_HPP
#define BORDER_CONTAINER_HPP

#include "container.hpp"
#include "border.hpp"
#include <string>

class BorderContainer : public Container {
public:
    BorderContainer();
    ~BorderContainer();

    void create();

    void set_child(WindowWrapper *child);

    void set_size(unsigned int width, unsigned int height);
    void set_size_and_position(int x, int y, unsigned int width, unsigned int height);
    void toggle_borders();

    void set_border_image(std::string path);
private:
    void get_size_and_coordinates_of_direction(
            Border::Direction direction,
            int *x, int *y,
            unsigned int *width, unsigned int *height);
    void resize_and_reposition_borders();
    void resize_and_position_children_for_size(unsigned int width, unsigned int height);
    WindowWrapper *m_child;
    Border *m_borders[Border::Direction::last];
    bool m_borders_shown;
};

#endif
