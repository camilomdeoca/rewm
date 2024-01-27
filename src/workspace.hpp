#ifndef WORKSPACE_HPP
#define WORKSPACE_HPP

#include <list>
#include <vector>

class Client;
class Monitor;

class Workspace {
public:
    Workspace(Monitor *monitor);
    ~Workspace();
    void add_client(Client *client);
    void remove_client(Client *client);
    void focus_client(Client *client);
    Client *get_focused_client();
    Monitor *get_monitor();
    std::vector<Client *> get_clients();
    void hide_clients();
    void show_clients();

private:
    std::list<Client *> m_clients;
    Client *m_focused_client;
    Monitor *m_monitor;
};

#endif
