#include "vertical_array.hpp"
#include "window_wrapper.hpp"

VerticalArray::VerticalArray() : m_center_child(nullptr)
{
}

VerticalArray::~VerticalArray()
{
    for (WindowWrapper *child : m_top_children) {
        delete child;
    }
    delete m_center_child;
    for (WindowWrapper *child : m_bottom_children) {
        delete child;
    }
}

void VerticalArray::create()
{
    create_window(get_x(), get_y(), get_width(), get_height());

    /* Create children */
    for (WindowWrapper *child : m_bottom_children) {
        child->create();
    }

    m_center_child->create();

    for (WindowWrapper *child : m_top_children) {
        child->create();
    }

    show();
}

void VerticalArray::add_top(WindowWrapper *child)
{
    m_top_children.push_back(child);
    child->set_parent(this);
    child->set_primary_container(get_primary_container());
}

void VerticalArray::set_center(WindowWrapper *child)
{
    m_center_child = child;
    child->set_parent(this);
    child->set_primary_container(get_primary_container());
}

void VerticalArray::add_bottom(WindowWrapper *child)
{
    m_bottom_children.push_back(child);
    child->set_parent(this);
    child->set_primary_container(get_primary_container());
}

void VerticalArray::resize_and_position_children_for_size(unsigned int width, unsigned int height)
{
    int y = 0;
    for (WindowWrapper *child : m_top_children) {
        child->set_size_and_position(0, y, width, child->get_height());
        y += child->get_height();
    }
    int y2 = 0;
    std::list<WindowWrapper *>::reverse_iterator iter;
    for (iter = m_bottom_children.rbegin(); iter != m_bottom_children.rend(); iter++) {
        WindowWrapper *child = *iter;
        child->set_size_and_position(
                0, height - child->get_height() - y2, width, child->get_height());
        y2 += child->get_height();
    }
    if (m_center_child)
        m_center_child->set_size_and_position(0, y, width, height - y - y2);
}

void VerticalArray::set_size(unsigned int width, unsigned int height)
{
    WindowWrapper::set_size(width, height);
    resize_and_position_children_for_size(width, height);
}

void VerticalArray::set_size_and_position(int x, int y, unsigned int width, unsigned int height)
{
    WindowWrapper::set_size_and_position(x, y, width, height);
    resize_and_position_children_for_size(width, height);
}
