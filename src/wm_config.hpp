#ifndef WM_CONFIG_HPP
#define WM_CONFIG_HPP

#include <string>
#include <string_view>
#include <unordered_map>
#include <toml++/toml.h>
#include <vector>

class WMConfig {
public:
    WMConfig(std::string file_path);
    virtual ~WMConfig();

    toml::table get_table();
    int get_int(std::string_view path);
    bool get_bool(std::string_view path);
    std::string get_string(std::string_view path);
    unsigned long get_color_ulong(std::string_view path);
    std::vector<std::string> get_string_array(std::string_view path);
private:
    toml::table m_parsed_file;
};

extern WMConfig *config;

#endif