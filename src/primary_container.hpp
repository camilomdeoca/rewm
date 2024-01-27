#ifndef PRIMARY_CONTAINER_HPP
#define PRIMARY_CONTAINER_HPP

#include "container.hpp"
class PrimaryContainer : public Container {
public:
    PrimaryContainer();
    ~PrimaryContainer();

    /* This function gets called from children when they are hovered */
    virtual void child_hovered() {};
    /* This gets called when children are pressed */
    virtual void child_pressed() {};
};

#endif /* PRIMARY_CONTAINER_HPP */