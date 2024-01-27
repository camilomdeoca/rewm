#include "wm_config.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

WMConfig *config;

WMConfig::WMConfig(std::string file_path)
{
    try {
        m_parsed_file = toml::parse_file(file_path);
    } catch (const toml::parse_error &error) {
        std::cerr << "Parsing config file failed: " << error << std::endl
                  << "Using fallback" << std::endl;
        try {
            // TODO: create default config and parse it here
            // m_parsed_file = toml::parse_file()
        } catch (const toml::parse_error &error) {
            throw std::runtime_error("Parsing default config file failed: "
                                     + std::string(error.description()));
        }
    }
}

WMConfig::~WMConfig()
{
}

int WMConfig::get_int(std::string_view path)
{
    // if (m_parsed_file.contains(path) == false) {
    //     throw std::runtime_error(std::string(path) + " option does not exist in the config.");
    // }
    if (!m_parsed_file.at_path(path).is_integer()) {
        throw std::runtime_error(std::string(path) + " in config is not an integer.");
    }

    return m_parsed_file.at_path(path).as_integer()->get();
}

bool WMConfig::get_bool(std::string_view path)
{
    if (!m_parsed_file.at_path(path).is_boolean()) {
        throw std::runtime_error(std::string(path) + " in config is not a boolean.");
    }

    return m_parsed_file.at_path(path).as_boolean()->get();
}

std::string WMConfig::get_string(std::string_view path)
{
    // if (m_parsed_file.contains(path) == false) {
    //     throw std::runtime_error(std::string(path) + " option does not exist in the config.");
    // }
    if (!m_parsed_file.at_path(path).is_string()) {
        throw std::runtime_error(std::string(path) + " in config is not a string.");
    }

    return m_parsed_file.at_path(path).as_string()->get();
}

toml::table WMConfig::get_table()
{
    return m_parsed_file;
}

unsigned long WMConfig::get_color_ulong(std::string_view path)
{
    // if (m_parsed_file.contains(path) == false) {
    //     throw std::runtime_error(std::string(path) + " option does not exist in the config.");
    // }
    if (!m_parsed_file.at_path(path).is_string()) {
        throw std::runtime_error(std::string(path) + " in config is not a string.");
    }

    std::string color_string = m_parsed_file.at_path(path).as_string()->get();

    if (color_string[0] != '#') {
        throw std::runtime_error(std::string(path)
                                 + " in config is not a color (in '#RRGGBBAA' format).");
    }

    color_string = color_string.erase(0, 1); /* Delete '#' character */

    unsigned long color = std::stoul(color_string, nullptr, 16); /* Parse as hex num*/

    color = (color >> 8 | color << 24); /* Convert from RGBA to ARGB */

    return color;
}

std::vector<std::string> WMConfig::get_string_array(std::string_view path)
{
    toml::array *array = m_parsed_file.at_path("workspace.names").as_array();
    std::vector<std::string> vector(array->size());
    int i = 0;
    for (toml::node &elem : *array) {
        vector[i++] = elem.as_string()->get();
    }
    return vector;
}
