#include "border_container.hpp"
#include "border.hpp"
#include "window_wrapper.hpp"
#include "wm_config.hpp"

BorderContainer::BorderContainer() : m_borders_shown(true)
{
    for (int direction = Border::Direction::nw; direction < Border::Direction::last; direction++) {
        m_borders[direction] = new Border(static_cast<Border::Direction>(direction));
        m_borders[direction]->set_parent(this);
    }
}

BorderContainer::~BorderContainer()
{
    for (int direction = Border::Direction::nw; direction < Border::Direction::last; direction++) {
        delete m_borders[direction];
    }
    delete m_child;
}

void BorderContainer::create()
{
    create_window(get_x(), get_y(), get_width(), get_height());

    /* Create children */
    for (int direction = Border::Direction::nw; direction < Border::Direction::last; direction++) {
        m_borders[direction]->set_primary_container(get_primary_container());
        m_borders[direction]->create();
    }
    m_child->create();

    show();
}

void BorderContainer::set_child(WindowWrapper *child)
{
    m_child = child;
    child->set_parent(this);
    child->set_primary_container(get_primary_container());
}

void BorderContainer::set_size(unsigned int width, unsigned int height)
{
    resize_and_position_children_for_size(width, height);
    WindowWrapper::set_size(width, height);
    resize_and_reposition_borders();
}

void BorderContainer::set_size_and_position(int x, int y, unsigned int width, unsigned int height)
{
    resize_and_position_children_for_size(width, height);
    WindowWrapper::set_size_and_position(x, y, width, height);
    resize_and_reposition_borders();
}

void BorderContainer::toggle_borders()
{
    m_borders_shown = !m_borders_shown;
    set_size_and_position(get_x(), get_y(), get_width(), get_height());
}

void BorderContainer::set_border_image(std::string path)
{
    for (Border *border : m_borders) {
        border->set_image(path);
    }
}

void BorderContainer::get_size_and_coordinates_of_direction(
        Border::Direction direction, int *x, int *y, unsigned int *width, unsigned int *height)
{
    int corner_size = config->get_int("window.border.corner_size");
    int border_size = config->get_int("window.border.size");
    switch (direction) {
    case Border::Direction::nw:
    case Border::Direction::ne:
    case Border::Direction::sw:
    case Border::Direction::se:
        {
            *width = corner_size;
            *height = corner_size;
        }
        break;
    case Border::Direction::n:
    case Border::Direction::s:
        {
            *height = border_size;
            *width = get_width() - 2 * corner_size;
        }
        break;
    case Border::Direction::w:
    case Border::Direction::e:
        {
            *height = get_height() - 2 * corner_size;
            *width = border_size;
        }
        break;
    default:
        /* Shouldn't happen */
        break;
    }
    switch (direction) {
    case Border::Direction::nw:
        *x = 0;
        *y = 0;
        break;
    case Border::Direction::ne:
        *x = get_width() - corner_size;
        *y = 0;
        break;
    case Border::Direction::sw:
        *x = 0;
        *y = get_height() - corner_size;
        break;
    case Border::Direction::se:
        *x = get_width() - corner_size;
        *y = get_height() - corner_size;
        break;
    case Border::Direction::n:
        *x = corner_size;
        *y = 0;
        break;
    case Border::Direction::s:
        *x = corner_size;
        *y = get_height() - border_size;
        break;
    case Border::Direction::w:
        *x = 0;
        *y = corner_size;
        break;
    case Border::Direction::e:
        *x = get_width() - border_size;
        *y = corner_size;
        break;
    default:
        /* Shouldn't happen */
        break;
    }
}

void BorderContainer::resize_and_reposition_borders()
{
    for (int direction = Border::Direction::nw; direction < Border::Direction::last; direction++) {
        int x, y;
        unsigned int width, height;
        get_size_and_coordinates_of_direction(
                static_cast<Border::Direction>(direction), &x, &y, &width, &height);
        m_borders[direction]->set_size_and_position(x, y, width, height);
    }
}

void BorderContainer::resize_and_position_children_for_size(unsigned int width, unsigned int height)
{
    if (m_child) {
        if (m_borders_shown) {
            int border_size = config->get_int("window.border.size");
            m_child->set_size_and_position(
                    border_size, border_size, width - 2 * border_size, height - 2 * border_size);
        } else {
            m_child->set_size_and_position(0, 0, width, height);
        }
    }
}
