#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include "window_wrapper.hpp"

class Container : public WindowWrapper {
public:
    Container();
    ~Container();

    virtual void set_size(unsigned int width, unsigned int height) = 0;
    virtual void set_size_and_position(int x, int y, unsigned int width, unsigned int height) = 0;
};

#endif