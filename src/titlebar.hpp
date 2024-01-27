#ifndef TITLEBAR_HPP
#define TITLEBAR_HPP

#include "button.hpp"

class TitleBar : public Button {
public:
    TitleBar();
    virtual ~TitleBar();
private:
    Time m_time_last_click;
};

#endif