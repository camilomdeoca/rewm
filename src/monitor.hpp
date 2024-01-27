#ifndef MONITOR_HPP
#define MONITOR_HPP

#include "workspace.hpp"
#include <string>
#include <unordered_map>

class Taskbar;
class Client;

class Monitor {
public:
    Monitor(short width, short height, short x, short y);
    ~Monitor();

    short get_width();
    short get_height();
    short get_x();
    short get_y();
    void focus_workspace(Workspace *workspace);
    Taskbar *get_taskbar();
    Workspace *get_selected_workspace();
    Workspace *get_workspace(std::string workspace);
    Workspace *get_workspace(unsigned int workspace_index);
    void set_selected_workspace(std::string workspace_name);
    void set_selected_workspace(unsigned int workspace_index);
    void move_client_to_workspace(Client *client, Workspace *workspace);
    size_t get_workspace_count();
private:
    Taskbar *m_taskbar;
    short m_width, m_height, m_x, m_y;
    std::vector<std::string> m_workspace_names;
    std::unordered_map<std::string, Workspace *> m_workspaces;
    Workspace *m_selected_workspace;
};

#endif
