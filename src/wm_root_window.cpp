#include "wm_root_window.hpp"
#include "wm.hpp"
#include "xapp.hpp"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <iostream>

WMRootWindow::WMRootWindow(Window root_window) :
    WindowWrapper(root_window)
{
    long current_desktop[] = {0};
    XChangeProperty(xapp->display(), get_window(),
            xapp->atom(XApp::AtomType::net_current_desktop),
            XA_CARDINAL, 32, PropModeReplace,
            (unsigned char *)current_desktop, 1);

    long number_of_desktops[] = { static_cast<long>(wm->get_focused_monitor()->get_workspace_count()) };
    XChangeProperty(xapp->display(), get_window(),
            xapp->atom(XApp::AtomType::net_number_of_desktops),
            XA_CARDINAL, 32, PropModeReplace,
            (unsigned char *)number_of_desktops, 1);

}

WMRootWindow::~WMRootWindow()
{
}

void WMRootWindow::create()
{
}

void WMRootWindow::handle_client_message(XClientMessageEvent *event)
{
    if (event->message_type == xapp->atom(XApp::AtomType::net_current_desktop)) {
        wm->get_focused_monitor()->set_selected_workspace(event->data.l[0]);

        long data[] = {event->data.l[0]};
        XChangeProperty(xapp->display(), get_window(),
                xapp->atom(XApp::AtomType::net_current_desktop),
                XA_CARDINAL, 32, PropModeReplace,
                (unsigned char *)data, 1);
    }
}