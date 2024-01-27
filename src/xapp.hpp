#ifndef APP_HPP
#define APP_HPP

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

class XApp {
public:
    enum class AtomType {
        wm_protocols,
        wm_delete,
        wm_state,
        wm_take_focus,

        /* - Net Protocols - */
        net_active_window,
        net_wm_desktop,
        net_current_desktop,
        net_number_of_desktops,
        net_supported,
        net_wm_name,
        net_wm_icon,
        net_wm_check,

        /* States */
        net_wm_state,
        net_wm_fullscreen,

        /* Window types */
        net_wm_window_type,
        net_wm_window_type_dialog,

        utf8_string,

        last
    };

    enum class CursorType {
    	/* cursor array indices */
    	normal,                          /* regular cursor */
    	move,                            /* arrow-cross cursor */
    	nw,                              /* north-west pointing cursor */
    	ne,                              /* north-east pointing cursor */
    	sw,                              /* south-west pointing cursor */
    	se,                              /* south-east pointing cursor */
    	n,                               /* north pointing cursor */
    	s,                               /* south pointing cursor */
    	w,                               /* west pointing cursor */
    	e,                               /* east pointing cursor */
    	v,                               /* vertical arrow cursor */
    	h,                               /* horizontal arrow cursor */
    	hand,                            /* hand cursor */
    	pirate,                          /* pirate-cross cursor */
    	last
    };

    XApp();
    virtual ~XApp();

    Display *display();
    int screen();
    Window root();
    Visual *visual();
    Visual *visual(const unsigned int depth);
    Colormap colormap();
    Colormap colormap(const unsigned int depth);
    XRenderPictFormat *format(const unsigned int depth);
    Atom atom(AtomType type);
    Cursor cursor(CursorType type); 

    void sync();
    void flush();

    int get_screen_width();
    int get_screen_height();

private:
    void init_atoms();
    void create_cursors();
    Visual *find_visual(const unsigned int depth);

    Display *m_display;
    Window m_root, m_wm_check_window;
    int m_screen;
    Visual *m_visual_default, *m_visual24, *m_visual32;
    Colormap m_colormap24, m_colormap32;
    XRenderPictFormat *m_format_24, *m_format_32;
    int m_screen_width, m_screen_height;

    Atom m_atoms[static_cast<int>(AtomType::last)];
    Cursor m_cursors[static_cast<int>(CursorType::last)];
};

extern XApp* xapp;

#endif
