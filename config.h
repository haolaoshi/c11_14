#pragma once

#ifndef CONFIG_H  
#define CONFIG_H  

#include <map>  
#include <fstream>  
#include <sstream>  
#include <string>  
#include <iostream>
#include <mutex>

class Config { 
public:
    static Config* getInstance(const std::string& configFilePath = "config.ini") {
        if (instance == nullptr) {
            std::lock_guard<std::mutex> lock(mutex);
            if (instance == nullptr) {
                instance = new Config(configFilePath);
            }
        }
        return instance;
    } 

    //~Config() {}

    std::map<std::string, std::string> getConfigOptions(const std::string& section) {
        return configOptions[section];
    }

private:
    std::ifstream file;
    std::map<std::string, std::map<std::string, std::string>> configOptions;
    static Config* instance;
    static std::mutex mutex;

    std::string trim(const std::string& str) {
        std::string trimmed = str;
        trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(), [](int c) {
            return !std::isspace(c);
            }));
        trimmed.erase(std::find_if(trimmed.rbegin(), trimmed.rend(), [](int c) {
            return !std::isspace(c);
            }).base(), trimmed.end());
        return trimmed;
    }

    Config(const std::string& configPath);
}; 



#endif // CONFIG_H