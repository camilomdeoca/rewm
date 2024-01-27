#include "assets.hpp"
#include "wm.hpp"
#include "wm_config.hpp"
#include "xapp.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    assets = new Assets();
    config = new WMConfig("config/config.toml");
    wm = new WindowManager();
    xapp = wm;

    wm->setup();

    wm->main_loop();

    delete assets;
    delete wm;
    delete config;

    return 0;
}
