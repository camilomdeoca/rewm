#include "managed_window.hpp"
#include "client.hpp"
#include "client_component.hpp"
#include "container.hpp"
#include "monitor.hpp"
#include "wm_config.hpp"
#include "xapp.hpp"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <Imlib2.h>
#include <climits>
#include <string>

ManagedWindow::ManagedWindow(Window window) : ClientComponent(window)
{
}

ManagedWindow::~ManagedWindow()
{
}

void ManagedWindow::create()
{
    set_event_mask(StructureNotifyMask | EnterWindowMask | PropertyChangeMask);
    set_parent(get_parent());
    show();
}

std::string ManagedWindow::get_title()
{
    std::string title_text = "Couldn't get title...";
    char **text_list = nullptr;
    int text_list_length;
    XTextProperty text_property;
    int status = XGetTextProperty(xapp->display(),
                                  static_cast<Client *>(get_primary_container())->get_managed()->get_window(),
                                  &text_property,
                                  xapp->atom(XApp::AtomType::net_wm_name));
    if (status && text_property.nitems != 0) {
        if (XmbTextPropertyToTextList(xapp->display(), &text_property, &text_list, &text_list_length)
                == Success
            && text_list_length >= 1 && text_list != nullptr && *text_list != nullptr)
        {
            title_text = text_list[0];
        }
        XFreeStringList(text_list);
    }
    XFree(text_property.value);
    return title_text;
}

XImage *ManagedWindow::get_icon()
{
    XImage *image = nullptr;
    Atom type;
    int format;
    unsigned long num_of_items, bytes_after;
    long* data = nullptr;
    if (XGetWindowProperty(xapp->display(), get_window(), xapp->atom(XApp::AtomType::net_wm_icon),
            0, LONG_MAX, False, XA_CARDINAL, &type, &format, &num_of_items, &bytes_after,
            (unsigned char **)&data)) {
        return nullptr;
    }

    if (num_of_items == 0 || format != 32) {
        XFree(data);
        return nullptr;
    }

    // First two long are width and height and the rest are the image pixels, and then another icon
    for (long *e = data;
            e + 2 < data + num_of_items && e[0] > 0 && e[1] > 0;
            e += 2 + e[0] * e[1]) {
        long w = e[0];
        long h = e[1];
        long *pixels = e + 2;

        Imlib_Image original = imlib_create_image(w, h);
        if (original == nullptr) {
            return nullptr;
        }
        imlib_context_set_image(original);
        imlib_image_set_has_alpha(1);
        DATA32 *original_data = imlib_image_get_data();
        DATA32 *original_data_end = original_data + w * h;
        long *p = pixels;
        for (DATA32 *d = original_data; d < original_data_end; d++, p++) {
            *d = (DATA32) *p;
        }

        constexpr int icon_size = 16;
        Imlib_Image scaled = imlib_create_cropped_scaled_image(0, 0, w , h, icon_size, icon_size);
        imlib_free_image_and_decache();
        if (scaled == nullptr) {
            return nullptr;
        }

        imlib_context_set_image(scaled);
        imlib_image_set_has_alpha(1);

        char *scaled_data = (char *)imlib_image_get_data_for_reading_only();
        image = XCreateImage(xapp->display(), xapp->visual(32), 32, ZPixmap, 0, scaled_data, icon_size, icon_size, 32, icon_size * 4);
    }

    XFree(data);
    return image;
}

void ManagedWindow::handle_enter(XEnterWindowEvent *event)
{
    get_primary_container()->child_hovered();
}

void ManagedWindow::handle_button(XButtonEvent *event)
{
    get_primary_container()->child_pressed();
}

void ManagedWindow::handle_client_message(XClientMessageEvent *event)
{
    if (event->message_type == xapp->atom(XApp::AtomType::net_wm_state)) {
        enum Action {remove = 0, add = 1, toggle = 2};
        Action action = Action(event->data.l[0]);

        if (static_cast<Atom>(event->data.l[1]) == xapp->atom(XApp::AtomType::net_wm_fullscreen)) {
            if (action == add || (!get_client()->is_fullscreen() && action == toggle)) {
                get_client()->toggle_fullscreen();
            } else if (action == remove || (get_client()->is_fullscreen() && action == toggle)) {
                get_client()->toggle_fullscreen();
            }
        }
    }
    else if (event->message_type == xapp->atom(XApp::AtomType::net_wm_desktop)) {
        Monitor *monitor = get_client()->get_workspace()->get_monitor();
        monitor->move_client_to_workspace(get_client(), monitor->get_workspace(event->data.l[0]));
    }
}

void ManagedWindow::handle_property_notify(XPropertyEvent *event)
{
    if (event->atom == XA_WM_NAME || event->atom == xapp->atom(XApp::AtomType::net_wm_name)) {
        get_client()->update_title();
    } else if (event->atom == xapp->atom(XApp::AtomType::net_wm_window_type)) {
        //get_client()->update_window_type();
    } else if (event->atom == xapp->atom(XApp::AtomType::net_wm_icon)) {
        get_client()->update_icon();
    }
    //switch (event->atom) {
    //case XA_WM_HINTS:
    //    get_client()->update_size_hints();
    //}
}

void ManagedWindow::handle_destroy_notify(XDestroyWindowEvent *event)
{
    set_window_deleted();
    delete get_primary_container();
}

void ManagedWindow::handle_unmap_notify(XUnmapEvent *event)
{
    set_window_deleted();
    delete get_primary_container();
}
