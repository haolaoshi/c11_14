#include "config.h"  

Config* Config::instance = nullptr;
std::mutex Config::mutex;

//Config::Config(const std::string& configPath)
//{
//}
Config::Config(const std::string& configFilePath) {
    file.open(configFilePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << configFilePath << std::endl;
        throw std::runtime_error("Failed to open config file: " + configFilePath);
    }

    std::string line, section;
    while (std::getline(file, line)) {
        if (line.empty() || line.at(0) == ';' || line.at(0) == '#' || line.at(0) == '/') {
            continue;
        }

        if (line[0] == '[' && line[line.size() - 1] == ']') {
            section = line.substr(1, line.size() - 2);
        }
        else {
            std::stringstream ss(line);
            std::string key, value;
            if (std::getline(ss, key, '=') && std::getline(ss, value)) {
                if (section.compare("dictionary") != 0)
                {
                    key = trim(key);
                    value = trim(value);
                }
                configOptions[section][key] = value;
            }
        }
    }
    file.close();
}