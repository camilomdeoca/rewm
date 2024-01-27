#ifndef CLIENT_COMPONENT_HPP
#define CLIENT_COMPONENT_HPP

#include "window_wrapper.hpp"

class ClientComponent : public WindowWrapper {
public:
    ClientComponent(Window window = None);
    virtual ~ClientComponent();
    Client *get_client();
};

#endif