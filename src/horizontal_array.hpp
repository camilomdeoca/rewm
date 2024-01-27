#ifndef HORIZONTAL_ARRAY_HPP
#define HORIZONTAL_ARRAY_HPP

#include "container.hpp"
#include <list>

class HorizontalArray : public Container {
public:
    HorizontalArray();
    ~HorizontalArray();

    void create();

    void add_left(WindowWrapper *child);
    void set_center(WindowWrapper *child);
    void add_right(WindowWrapper *child);

    void set_size(unsigned int width, unsigned int height);
    void set_size_and_position(int x, int y, unsigned int width, unsigned int height);
private:
    void resize_and_position_children_for_size(unsigned int width, unsigned int height);
    std::list<WindowWrapper *> m_left_children, m_right_children;
    WindowWrapper *m_center_child;
};

#endif