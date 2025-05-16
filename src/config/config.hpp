#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>

class Config {
private:
    std::unordered_map<std::string, std::string> m_config_map;
public:
    Config(const std::string& filename);
    const std::string& get(const std::string& key) const;
};

#endif
