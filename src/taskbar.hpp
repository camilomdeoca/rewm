#ifndef TASKBAR_HPP
#define TASKBAR_HPP

#include "primary_container.hpp"
#include "horizontal_array.hpp"
#include "monitor.hpp"
#include "button.hpp"
#include "padding.hpp"
#include <string>
#include <unordered_map>
#include <vector>

class Monitor;

class Taskbar : public PrimaryContainer {
public:
    Taskbar(Monitor *monitor);
    virtual ~Taskbar();

    void create();

    void set_main_container(Container *container);
    void set_size(unsigned int width, unsigned int height);
    void set_size_and_position(int x, int y, unsigned int width, unsigned int height);

    void set_selected_workspace_button(std::string workspace_name);

    void update_tasklist();

private:
    Container *m_main_container;
    Monitor *m_monitor;
    HorizontalArray *m_tasklist;
    std::unordered_map<std::string, Button *> m_workspace_buttons;
    Button *m_selected_workspace_button;
    //std::vector<std::pair<Button, Padding>> m_tasklist_buttons_and_padding;
};

#endif /* TASKBAR_HPP */