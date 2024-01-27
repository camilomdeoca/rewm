#include "workspace.hpp"
#include "client.hpp"
#include "monitor.hpp"
#include "taskbar.hpp"
#include <string>
#include <vector>
#include <iostream>

Workspace::Workspace(Monitor *monitor) :
    m_focused_client(nullptr),
    m_monitor(monitor)
{
}

Workspace::~Workspace()
{
    while (!m_clients.empty()) {
        delete m_clients.front();
    }
}

void Workspace::add_client(Client *client)
{
    m_clients.push_front(client);
    m_monitor->get_taskbar()->update_tasklist();
}

void Workspace::remove_client(Client *client)
{
    if (m_focused_client == client) {
        m_focused_client = nullptr;
    }
    m_clients.remove(client);
    m_monitor->get_taskbar()->update_tasklist();
}

void Workspace::focus_client(Client *client)
{
    if (m_focused_client != client) {
        if (m_focused_client){
            m_focused_client->unfocus();
        }
        m_focused_client = client;
    }
    m_monitor->focus_workspace(this);
    m_monitor->get_taskbar()->update_tasklist();
}

Client *Workspace::get_focused_client()
{
    return m_focused_client;
}

Monitor *Workspace::get_monitor()
{
    return m_monitor;
}

void Workspace::show_clients()
{
    for (Client *client : m_clients) {
        client->show();
    }
}

std::vector<Client *> Workspace::get_clients()
{
    std::vector<Client *> clients;
    clients.reserve(m_clients.size());
    for (Client *client : m_clients) {
        clients.push_back(client);
    }
    return clients;
}

void Workspace::hide_clients()
{
    for (Client *client : m_clients) {
        client->hide();
    }
}
