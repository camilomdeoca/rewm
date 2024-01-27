#include "xapp.hpp"
#include <X11/X.h>
#include <array>
#include <stdexcept>
#include <X11/cursorfont.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <vector>

XApp *xapp;

XApp::XApp()
{
    m_display = XOpenDisplay(NULL);
    if (display() == NULL) {
        throw std::runtime_error("Could not open display.");
    }

    #if DEBUG
        XSynchronize(display(), True);
    #endif

    m_screen = DefaultScreen(display());
    m_root = RootWindow(display(), screen());
    m_visual_default = DefaultVisual(display(), screen());
    m_screen_width = DisplayWidth(display(), screen());
    m_screen_height = DisplayHeight(display(), screen());

    m_visual24 = find_visual(24);
    m_visual32 = find_visual(32);
    m_colormap24 = XCreateColormap(display(), root(), visual(24), AllocNone);
    m_colormap32 = XCreateColormap(display(), root(), visual(32), AllocNone);
    m_format_24 = XRenderFindVisualFormat(display(), visual(24));
    m_format_32 = XRenderFindVisualFormat(display(), visual(32));

    int event_base, error_base;

    if (!XRenderQueryExtension(display(), &event_base, &error_base)) {
        XCloseDisplay(m_display);
        throw std::runtime_error("Xrender-extension not found.");
    }

    init_atoms();
    create_cursors();
}

XApp::~XApp()
{
    for (Cursor cursor : m_cursors) {
        XFreeCursor(display(), cursor);
    }

    XFreeColormap(display(), m_colormap24);
    XFreeColormap(display(), m_colormap32);
    XCloseDisplay(display());
}

Display *XApp::display()
{
    return m_display;
}

int XApp::screen()
{
    return m_screen;
}

Window XApp::root()
{
    return m_root;
}

Visual *XApp::find_visual(const unsigned int depth)
{
    XVisualInfo visual_info;
    if (!XMatchVisualInfo(display(), XDefaultScreen(display()), depth, TrueColor, &visual_info)) {
        throw std::runtime_error("No " + std::to_string(depth) + "bit color depth visual.");
    }
    return visual_info.visual;
}

void XApp::init_atoms()
{
    struct {
        AtomType atom;
        const char *name;
    } atom_names[] = {
        {AtomType::wm_protocols,              "WM_PROTOCOLS"},
        {AtomType::wm_delete,                 "WM_DELETE_WINDOW"},
        {AtomType::wm_state,                  "WM_STATE"},
        {AtomType::wm_take_focus,             "WM_TAKE_FOCUS"},

        {AtomType::net_active_window,         "_NET_ACTIVE_WINDOW"},
        {AtomType::net_wm_desktop,            "_NET_WM_DESKTOP"},
        {AtomType::net_current_desktop,       "_NET_CURRENT_DESKTOP"},
        {AtomType::net_number_of_desktops,    "_NET_NUMBER_OF_DESKTOPS"},
        {AtomType::net_supported,             "_NET_SUPPORTED"},
        {AtomType::net_wm_name,               "_NET_WM_NAME"},
        {AtomType::net_wm_icon,               "_NET_WM_ICON"},
        {AtomType::net_wm_state,              "_NET_WM_STATE"},
        {AtomType::net_wm_check,              "_NET_SUPPORTING_WM_CHECK"},
        {AtomType::net_wm_fullscreen,         "_NET_WM_STATE_FULLSCREEN"},
        {AtomType::net_wm_window_type,        "_NET_WM_WINDOW_TYPE"},
        {AtomType::net_wm_window_type_dialog, "_NET_WM_WINDOW_TYPE_DIALOG"},

        {AtomType::utf8_string,               "UTF8_STRING"}
    };

    for (const auto &[atom, name] : atom_names) {
        m_atoms[static_cast<int>(atom)] = XInternAtom(display(), name, False);
    }

    m_wm_check_window = XCreateSimpleWindow(display(), root(), 0, 0, 1, 1, 0, 0, 0);

    XChangeProperty(display(), m_wm_check_window, atom(AtomType::net_wm_check), XA_WINDOW, 32,
		    PropModeReplace, (unsigned char *) &m_wm_check_window, 1);

    XChangeProperty(display(), m_wm_check_window, atom(AtomType::net_wm_name),
            atom(AtomType::utf8_string), 8,	PropModeReplace, (unsigned char *) "rewm", 4);
    
    XChangeProperty(display(), root(), atom(AtomType::net_wm_check), XA_WINDOW, 32,
		    PropModeReplace, (unsigned char *) &m_wm_check_window, 1);

    /* Set supported NET protocols */
    const std::vector<Atom> net_atoms = {
        atom(AtomType::net_active_window),
        atom(AtomType::net_supported),
        atom(AtomType::net_wm_name),
        atom(AtomType::net_wm_check),
        atom(AtomType::net_wm_fullscreen),
        atom(AtomType::net_wm_window_type),
        atom(AtomType::net_wm_window_type_dialog)
    };
    XChangeProperty(display(), root(), atom(AtomType::net_supported), XA_ATOM, 32,
            PropModeReplace, (unsigned char *) net_atoms.data(), net_atoms.size());
}

void XApp::create_cursors()
{
    struct {
        CursorType type;
        unsigned int cursor;
    } cursors[] = {
        { CursorType::normal, XC_left_ptr            },
        { CursorType::move,   XC_fleur               },
        { CursorType::n,      XC_top_side            },
        { CursorType::w,      XC_left_side           },
        { CursorType::e,      XC_right_side          },
        { CursorType::s,      XC_bottom_side         },
        { CursorType::nw,     XC_top_left_corner     },
        { CursorType::ne,     XC_top_right_corner    },
        { CursorType::sw,     XC_bottom_left_corner  },
        { CursorType::se,     XC_bottom_right_corner },
        { CursorType::v,      XC_sb_v_double_arrow   },
        { CursorType::h,      XC_sb_h_double_arrow   },
        { CursorType::hand,   XC_hand1               },
        { CursorType::pirate, XC_pirate              },
    };

    for (const auto [type, cursor] : cursors) {
        m_cursors[static_cast<int>(type)] = XCreateFontCursor(display(), cursor);
    }
}

Cursor XApp::cursor(CursorType cursor_type)
{
    return m_cursors[static_cast<int>(cursor_type)];
}

Visual *XApp::visual()
{
    return m_visual_default;
}

Visual *XApp::visual(const unsigned int depth)
{
    return depth == 32 ? m_visual32 : depth == 24 ? m_visual24 : nullptr;
}

Colormap XApp::colormap()
{
    return DefaultColormap(display(), screen());
}

Colormap XApp::colormap(const unsigned int depth)
{
    return depth == 32 ? m_colormap32 : depth == 24 ? m_colormap24 : None;
}

XRenderPictFormat *XApp::format(const unsigned int depth)
{
    return depth == 32 ? m_format_32 : depth == 24 ? m_format_24 : nullptr;
}

Atom XApp::atom(AtomType type)
{
    return m_atoms[static_cast<int>(type)];
}

void XApp::sync()
{
    XSync(display(), False);
}

void XApp::flush()
{
    XFlush(display());
}

int XApp::get_screen_width()
{
    return m_screen_width;
}

int XApp::get_screen_height()
{
    return m_screen_height;
}
