#include "horizontal_array.hpp"
#include "window_wrapper.hpp"

HorizontalArray::HorizontalArray() : m_center_child(nullptr)
{
}

HorizontalArray::~HorizontalArray()
{
    for (WindowWrapper *child : m_left_children) {
        delete child;
    }
    delete m_center_child;
    for (WindowWrapper *child : m_right_children) {
        delete child;
    }
}

void HorizontalArray::create()
{
    create_window(get_x(), get_y(), get_width(), get_height());

    /* Create children */
    for (WindowWrapper *child : m_left_children) {
        child->create();
    }

    m_center_child->create();

    for (WindowWrapper *child : m_right_children) {
        child->create();
    }

    show();
}

void HorizontalArray::add_left(WindowWrapper *child)
{
    m_left_children.push_back(child);
    child->set_parent(this);
    child->set_primary_container(get_primary_container());
}

void HorizontalArray::set_center(WindowWrapper *child)
{
    m_center_child = child;
    child->set_parent(this);
    child->set_primary_container(get_primary_container());
}

void HorizontalArray::add_right(WindowWrapper *child)
{
    m_right_children.push_back(child);
    child->set_parent(this);
    child->set_primary_container(get_primary_container());
}

void HorizontalArray::resize_and_position_children_for_size(unsigned int width, unsigned int height)
{
    int x = 0;
    for (WindowWrapper *child : m_left_children) {
        child->set_size_and_position(x, 0, child->get_width(), height);
        x += child->get_width();
    }
    int x2 = 0;
    std::list<WindowWrapper *>::reverse_iterator iter;
    for (iter = m_right_children.rbegin(); iter != m_right_children.rend(); iter++) {
        WindowWrapper *child = *iter;
        child->set_size_and_position(
                width - child->get_width() - x2, 0, child->get_width(), height);
        x2 += child->get_width();
    }
    if (m_center_child)
        m_center_child->set_size_and_position(x, 0, width - x - x2, height);
}

void HorizontalArray::set_size(unsigned int width, unsigned int height)
{
    WindowWrapper::set_size(width, height);
    resize_and_position_children_for_size(width, height);
}

void HorizontalArray::set_size_and_position(int x, int y, unsigned int width, unsigned int height)
{
    WindowWrapper::set_size_and_position(x, y, width, height);
    resize_and_position_children_for_size(width, height);
}
