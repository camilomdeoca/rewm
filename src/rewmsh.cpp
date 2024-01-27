#include "xapp.hpp"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdexcept>
#include <string>
#include <iostream>

const long SOURCE_INDICATION = 1;

class ReWmSh : public XApp {
public:
    ReWmSh(int argc, char **argv);
    virtual ~ReWmSh();

    Window get_active_window();
};

Window ReWmSh::get_active_window()
{
    int format;
    unsigned long num_of_items, bytes_after;
    Atom type = XA_WINDOW;
    Window *window;

    if (XGetWindowProperty(display(), root(), atom(AtomType::net_active_window), 0, 1, False,
            type, &type, &format, &num_of_items, &bytes_after, (unsigned char **)&window) != Success
            || num_of_items == 0) {
        return None;
    }

    return *window;
}

ReWmSh::ReWmSh(int argc, char **argv)
{
    if (argc < 2) {
        std::cout << "No command" << std::endl;
        exit(1);
    }

    std::string command = argv[1];

    if (command == "set-current-workspace-index") {    
        XEvent xev{};

        if (argc <= 2) {
            std::cout << "No workspace index" << std::endl;
            exit(1);
        }

        xev.xclient.type = ClientMessage;
        xev.xclient.window = root();
        xev.xclient.message_type = atom(AtomType::net_current_desktop);
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = std::stol(argv[2]);

        XSendEvent(display(), root(), False, SubstructureNotifyMask, &xev);
    }
    else if (command == "move-selected-window-to-workspace") {
        XEvent xev{};

        if (argc <= 2) {
            std::cout << "No workspace index" << std::endl;
            exit(1);
        }

        Window selected_window = get_active_window();

        xev.xclient.type = ClientMessage;
        xev.xclient.window = selected_window;
        xev.xclient.message_type = atom(AtomType::net_wm_desktop);
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = std::stol(argv[2]);
        xev.xclient.data.l[1] = SOURCE_INDICATION;

        XSendEvent(display(), root(), False, SubstructureNotifyMask, &xev);

        std::cout << "Moved window: " << selected_window << " to workspace: " << std::stol(argv[2]) << std::endl;
    }
}

ReWmSh::~ReWmSh()
{
}

int main(int argc, char **argv)
{
    ReWmSh app(argc, argv);

    return 0;
}