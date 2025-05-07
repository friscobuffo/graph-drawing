#include "config.hpp"

#include <iostream>
#include <fstream>

std::unordered_map<std::string, std::string> parse_config(const std::string& filename) {
    std::unordered_map<std::string, std::string> config;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue; // Skip comments/empty lines
        int delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(0, delimiter_pos);
            std::string value = line.substr(delimiter_pos + 1);
            config[key] = value;
        }
    }
    return config;
}
