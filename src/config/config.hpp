#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>

std::unordered_map<std::string, std::string> parse_config(const std::string& filename);

#endif
