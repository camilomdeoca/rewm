#ifndef VERTICAL_ARRAY_HPP
#define VERTICAL_ARRAY_HPP

#include "container.hpp"
#include <list>

class VerticalArray : public Container {
public:
    VerticalArray();
    ~VerticalArray();

    void create();

    void add_top(WindowWrapper *child);
    void set_center(WindowWrapper *child);
    void add_bottom(WindowWrapper *child);

    void set_size(unsigned int width, unsigned int height);
    void set_size_and_position(int x, int y, unsigned int width, unsigned int height);
private:
    void resize_and_position_children_for_size(unsigned int width, unsigned int height);
    std::list<WindowWrapper *> m_top_children, m_bottom_children;
    WindowWrapper *m_center_child;
};

#endif