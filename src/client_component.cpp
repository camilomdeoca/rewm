#include "client_component.hpp"
#include "client.hpp"
#include "window_wrapper.hpp"

ClientComponent::ClientComponent(Window window) : WindowWrapper(window)
{
}

ClientComponent::~ClientComponent()
{
}

Client *ClientComponent::get_client()
{
    return static_cast<Client *>(get_primary_container());
}