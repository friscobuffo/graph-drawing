#include "config.hpp"

#include <fstream>
#include <stdexcept>

Config::Config(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue; // Skip comments/empty lines
        int delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(0, delimiter_pos);
            std::string value = line.substr(delimiter_pos + 1);
            m_config_map[key] = value;
        }
    }
}

const std::string& Config::get(const std::string& key) const {
    if (!m_config_map.contains(key))
        throw std::runtime_error("Config: key not found" + key);
    return m_config_map.at(key);
}