#include "titlebar.hpp"
#include "client.hpp"
#include "container.hpp"
#include "wm.hpp"
#include "wm_config.hpp"
#include "xapp.hpp"
#include <string>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

TitleBar::TitleBar() : m_time_last_click(0)
{
    set_action(
            [this](XButtonEvent *event) {
                get_primary_container()->child_pressed();
                if (event->time - m_time_last_click < 200 /* ms*/) { /* Double click */
                    static_cast<Client *>(get_primary_container())->toggle_maximize();
                } else {
                    static_cast<Client *>(get_primary_container())->begin_drag_move(event);
                }
                m_time_last_click = event->time;
            },
            ActionTime::on_press);
}

TitleBar::~TitleBar()
{
}
