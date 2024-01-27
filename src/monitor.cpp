#include "monitor.hpp"
#include "taskbar.hpp"
#include "window_wrapper.hpp"
#include "wm.hpp"
#include "wm_config.hpp"
#include "workspace.hpp"
#include <string>
#include <vector>

Monitor::Monitor(short width, short height, short x, short y) :
    m_taskbar(nullptr),
    m_width(width),
    m_height(height),
    m_x(x),
    m_y(y),
    m_workspace_names(config->get_string_array("workspace.names")),
    m_selected_workspace(nullptr)
{
    for (std::string workspace_name : m_workspace_names) {
        m_workspaces.insert({workspace_name, new Workspace(this)});
    }

    m_taskbar = new Taskbar(this);

    set_selected_workspace(0);
}

Monitor::~Monitor()
{
    for (auto [workspace_name, workspace] : m_workspaces) {
        delete workspace;
    }
    delete m_taskbar;
}

short Monitor::get_width()
{
    return m_width;
}

short Monitor::get_height()
{
    return m_height;
}

short Monitor::get_x()
{
    return m_x;
}

short Monitor::get_y()
{
    return m_y;
}

void Monitor::focus_workspace(Workspace *workspace)
{
    wm->focus_monitor(this);
    if (m_selected_workspace != workspace) {
        if (m_selected_workspace->get_focused_client()) {
            m_selected_workspace->get_focused_client()->unfocus();
        }
        m_selected_workspace = workspace;
    }
}

Taskbar *Monitor::get_taskbar()
{
    return m_taskbar;
}

Workspace *Monitor::get_selected_workspace()
{
    return m_selected_workspace;
}

Workspace *Monitor::get_workspace(std::string workspace)
{
    return m_workspaces.find(workspace)->second;
}

Workspace *Monitor::get_workspace(unsigned int workspace_index)
{
    return get_workspace(m_workspace_names[workspace_index]);
}

void Monitor::set_selected_workspace(std::string workspace_name)
{
    Workspace *new_selected_workspace = get_workspace(workspace_name);
    if (new_selected_workspace != m_selected_workspace) {
        if (m_selected_workspace) {
            m_selected_workspace->hide_clients();
            if (m_selected_workspace->get_focused_client()) {
                m_selected_workspace->get_focused_client()->unfocus();
            }
        }

        m_selected_workspace = new_selected_workspace;

        m_taskbar->set_selected_workspace_button(workspace_name);
        m_taskbar->update_tasklist();

        m_selected_workspace->show_clients();
        if (m_selected_workspace->get_focused_client()) {
            m_selected_workspace->get_focused_client()->focus();
        }
    }
}

void Monitor::set_selected_workspace(unsigned int workspace_index)
{
    set_selected_workspace(m_workspace_names[workspace_index]);
}

void Monitor::move_client_to_workspace(Client *client, Workspace *workspace)
{
    Workspace *from = client->get_workspace();

    client->hide();
    from->remove_client(client);
    client->set_workspace(workspace);
    workspace->add_client(client);
}

size_t Monitor::get_workspace_count()
{
    return m_workspaces.size();
}
