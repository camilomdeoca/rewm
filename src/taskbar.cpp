#include "taskbar.hpp"
#include "button.hpp"
#include "client.hpp"
#include "container.hpp"
#include "horizontal_array.hpp"
#include "monitor.hpp"
#include "padding.hpp"
#include "window_wrapper.hpp"
#include "wm.hpp"
#include "wm_config.hpp"
#include "workspace.hpp"
#include <csignal>
#include <iostream>
#include <string>
#include <vector>
#include <X11/Xlib.h>

Taskbar::Taskbar(Monitor *monitor) :
    m_monitor(monitor),
    m_selected_workspace_button(nullptr)
{
    int titlebar_height = config->get_int("window.titlebar.height");

    HorizontalArray *horizontal_container = new HorizontalArray();
    set_main_container(horizontal_container);

    std::vector<std::string> workspace_names = config->get_string_array("workspace.names");
    for (std::string workspace_name : workspace_names) {
        Button *button = new Button();
        button->set_width(titlebar_height * 1.2);
        button->set_text(workspace_name);
        button->set_action(
                [this, workspace_name](XButtonEvent *event) {
                    m_monitor->set_selected_workspace(workspace_name);
                },
                Button::ActionTime::on_release);
        m_workspace_buttons.insert({workspace_name, button});
        horizontal_container->add_left(button);
    }

    Padding *padding = new Padding();
    padding->set_border_sides(Padding::BorderSides::top_bottom);
    padding->set_width(titlebar_height / 5);
    horizontal_container->add_left(padding);

    /* TASKLIST (Probaly will move to its own class) */
    m_tasklist = new HorizontalArray();
    Padding *tasklist_end_padding = new Padding();
    tasklist_end_padding->set_border_sides(Padding::BorderSides::all);
    /* width is forced because is in the expanding part of the container */
    m_tasklist->set_center(tasklist_end_padding);
    horizontal_container->set_center(m_tasklist);
    /* END TASKLIST */

    Padding *padding3 = new Padding();
    padding3->set_border_sides(Padding::BorderSides::top_bottom);
    padding3->set_width(titlebar_height / 5);
    horizontal_container->add_right(padding3);

    Button *close_button = new Button();
    close_button->set_width(titlebar_height);
    close_button->set_action([](XButtonEvent *event) { wm->exit(); },
                             Button::ActionTime::on_release);
    horizontal_container->add_right(close_button);

    set_size_and_position(m_monitor->get_x(),
                          m_monitor->get_y(),
                          m_monitor->get_width(),
                          config->get_int("window.titlebar.height"));

    create();
}

Taskbar::~Taskbar()
{
    delete m_main_container;
}

void Taskbar::create()
{
    create_window(get_x(), get_y(), get_width(), get_height());

    /* Create children */
    m_main_container->create();

    show();
}

void Taskbar::set_main_container(Container *container)
{
    m_main_container = container;
    m_main_container->set_parent(this);
    m_main_container->set_primary_container(this);
}

void Taskbar::set_size(unsigned int width, unsigned int height)
{
    WindowWrapper::set_size(width, height);
    m_main_container->set_size(width, height);
}

void Taskbar::set_size_and_position(int x, int y, unsigned int width, unsigned int height)
{
    WindowWrapper::set_size_and_position(x, y, width, height);
    m_main_container->set_size(width, height);
}

void Taskbar::set_selected_workspace_button(std::string workspace_name)
{
    if (m_selected_workspace_button) {
        m_selected_workspace_button->set_image("images/lightblue.xpm");
    }
    m_selected_workspace_button = m_workspace_buttons.find(workspace_name)->second;
    m_selected_workspace_button->set_image("images/selectedblue.xpm");
}

// TODO: Do something better here.
void Taskbar::update_tasklist() {
    int tasklist_button_width = config->get_int("tasklist.button_width");
    int tasklist_padding_width = config->get_int("tasklist.padding_width");
    unsigned int width = m_tasklist->get_width();
    unsigned int height = m_tasklist->get_height();
    delete m_tasklist; //
    m_tasklist = new HorizontalArray();
    m_tasklist->set_size(width, height);
    /* TEST!!! */
    Workspace *selected_workspace = m_monitor->get_selected_workspace();
    std::vector<Client *> clients = selected_workspace->get_clients();
    Client *focused_client = selected_workspace->get_focused_client();
    for (Client *client : clients) {
        Button *button = new Button();
        button->set_width(tasklist_button_width);
        button->set_text(client->get_title());
        button->set_action(
                [client](XButtonEvent *event) {
                    client->focus();
                }, Button::ActionTime::on_press);
        if (client == focused_client) {
            button->set_image("images/selectedblue.xpm");
        }
        m_tasklist->add_left(button);

        Padding *padding = new Padding();
        padding->set_width(tasklist_padding_width);
        padding->set_border_sides(Padding::BorderSides::top_bottom);
        m_tasklist->add_left(padding);
    }
    Padding *tasklist_end_padding = new Padding();
    tasklist_end_padding->set_border_sides(Padding::BorderSides::all);
    /* width is forced because is in the expanding part of the container */
    m_tasklist->set_center(tasklist_end_padding);
    static_cast<HorizontalArray *>(m_main_container)->set_center(m_tasklist);
    m_tasklist->create();
    m_main_container->set_size(m_main_container->get_width(), m_main_container->get_height());
}
