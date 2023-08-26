#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include <thread>
#include <chrono>
#include <ctime>
#include <map>
#include <mutex>
#include <csignal>
#include <functional>
#include <iomanip>
//#include <boost/property_tree/ptree.hpp>
#include "sqlite3.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/ini_parser.hpp"

·



class Config {
public:
    Config(std::string  filePath) : filePath(std::move(filePath)) {
        loadConfig();
    }

    [[noreturn]] void reloadConfig() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            loadConfig();
        }
    }

    std::string getValue(const std::string& section, const std::string& key) {
        std::string sectionKey = section + "." + key;
        if (configMap.find(sectionKey) != configMap.end()) {
            return configMap[sectionKey];
        }
        return "";
    }

private:
    static std::string trim(const std::string& str) {
        const std::string spaces = " \t";
        std::size_t start = str.find_first_not_of(spaces);
        std::size_t end = str.find_last_not_of(spaces);
        if (start == std::string::npos || end == std::string::npos) {
            return "";
        }
        return str.substr(start, end - start + 1);
    }

    void loadConfig() {
        std::ifstream configFile(filePath);
        if (!configFile.is_open()) {
            std::cout << "Failed to open config file." << std::endl;
            return;
        }

        std::string line;
        std::string currentSection;
        while (std::getline(configFile, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue; // Ignore comments and empty lines
            }
            if (line[0] == '[' && line[line.length() - 1] == ']') {
                currentSection = line.substr(1, line.length() - 2);
            }
            else {
                std::size_t pos = line.find('=');
                if (pos != std::string::npos) {
                    std::string key = trim(line.substr(0, pos));
                    std::string value = trim(line.substr(pos + 1));
                    std::string sectionKey = currentSection;
                    sectionKey += "." ;
                    sectionKey += key;
                    configMap[sectionKey] = value;
                }
            }
        }

        configFile.close();
    }

    std::string filePath;
    std::map<std::string, std::string> configMap;
};


// Global variables
std::mutex mtx;
bool stopProcessing = false;
volatile std::sig_atomic_t stop_flag = 0;
auto daily_logger = spdlog::daily_logger_mt("stocktts", "logs/stocktts.log", 2, 30);
static const std::string CONFIG_FILE_NAME = "StockTool.conf";

void signalHandler(int signal) {
    stop_flag = 1;
}

// Function to check if a file exists
bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

// Function to read and process the file
void processFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cout << "[" << std::this_thread::get_id() << "]" << "Failed to open file: " << filename << std::endl;
        return;
    }
    // Open SQLite database
    sqlite3* db;
    int rc = sqlite3_open("mydb.db", &db);
    if (rc) {
        std::cout << "[" << std::this_thread::get_id() << "]" << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    // Check if stock_name table exists
    std::string checkTableQuery = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='stock_name';";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, checkTableQuery.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cout << "[" << std::this_thread::get_id() << "]" << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    rc = sqlite3_step(stmt);
    int result = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if (result == 0) {
        // Create stock_name table
        std::string createTableQuery = "CREATE TABLE stock_name (id varchar(20) PRIMARY KEY, name text(50),update_time text(20),upload_state varchar(20) );";
        rc = sqlite3_exec(db, createTableQuery.c_str(), nullptr, nullptr, nullptr);

        if (rc != SQLITE_OK) {
            std::cout << "[" << std::this_thread::get_id() << "]" << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return;
        }

        std::cout << "[" << std::this_thread::get_id() << "]" << "Created stock_name table." << std::endl;
    }


    std::string line;
    while (std::getline(file, line)) {
        // Process each line
        std::istringstream iss(line);

        std::string column1, column2, column3;
        std::getline(iss, column1, '|');
        std::getline(iss, column2, '|');
        std::getline(iss, column3, '|');

        if ("HEADER" == column1 || "TRAILER" == column1)
            continue;

        // Check if SecurityID exists in stock_name table
        std::string query = "SELECT id, name FROM stock_name WHERE id = " + column2 + ";";
        sqlite3_stmt* stmt;
        rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);

        if (rc != SQLITE_OK) {
            std::cout << "[" << std::this_thread::get_id() << "]" << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return;
        }

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string id(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            std::string name(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));

            if (name != column3) {
                // Update SymbolName in stock_name table
                query = "UPDATE stock_name SET name = '";
                query += column3 ;
                query += "' WHERE id = " ;
                query += id ;
                query += ";";
                rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);

                if (rc != SQLITE_OK) {
                    std::cout << "[" << std::this_thread::get_id() << "]" << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
                    sqlite3_close(db);
                    return;
                }

                std::cout << "[" << std::this_thread::get_id() << "]" << "Updated record  SecurityID: " << column2 << ", SymbolName: " << name << " to " << column3 << std::endl;
            }
        }
        else {
            // Insert new record into stock_name table
            query = std::string("INSERT INTO stock_name (id, name) VALUES (").append(column2);
            query +=", '" ;
            query += column3 ;
            query += "');";

            rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);

            if (rc != SQLITE_OK) {
                std::cout << "[" << std::this_thread::get_id() << "]" << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                return;
            }

            std::cout << "[" << std::this_thread::get_id() << "]" << "Inserted new record into stock_name table. SecurityID: " << column2 << ", SymbolName: " << column3 << std::endl;
        }

        sqlite3_finalize(stmt);

    }


    // 查询并打印数据
    //const char* selectQuery = "SELECT id,name FROM stock_name";
    //rc = sqlite3_exec(db, selectQuery, [](void* data, int argc, char** argv, char** columnNames) {
    //    std::cout << "id: " << argv[0] << ",name: "<<argv[1]<<std::endl;
    //    return 0;
    //    }, 0, 0);

    // Close SQLite database
    sqlite3_close(db);
}

// Function to monitor and process the configuration file
/**
Error	C4996	'localtime': This function or variable may be unsafe.Consider using localtime_s instead.
To disable deprecation, use _CRT_SECURE_NO_WARNINGS.See

**/
void monitorConfigFile() {
    std::time_t startTime = 0, endTime = 0;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));

        // Check if it's time to start processing
        std::time_t now = std::time(nullptr);
        std::tm* tm = std::localtime(&now);
        int hour = tm->tm_hour;
        int minute = tm->tm_min;
        int second = tm->tm_sec;
        int year = tm->tm_year;
        int month = tm->tm_mon;
        int day = tm->tm_mday;

        // Read start and end times from configuration file
        std::ifstream configFile(CONFIG_FILE_NAME);
        std::string line;
        bool processFiles = false;
        spdlog::info("monitorConfigFile");
        while (std::getline(configFile, line)) {
            if (line.find("[timer]") != std::string::npos) {

                std::getline(configFile, line);  // Read the next line after "[timer]"

                while (line.find('[') == std::string::npos) {
                    std::string key, value;
                    std::istringstream iss(line);
                    if (iss >> key >> value) {
                        if (value == "=")
                            iss >> value;
                        if (key == "start_time") {
                            std::tm startTm{};
                            std::istringstream iss(value);
                            iss >> std::get_time(&startTm, "%H:%M");
                            startTm.tm_year = year;
                            startTm.tm_mon = month;
                            startTm.tm_mday = day;

                            startTime = std::mktime(&startTm);
                            //std::cout << iss.str() << "," << startTime << std::endl;

                        }
                        else if (key == "end_time") {
                            std::tm endTm{};
                            std::istringstream iss(value);
                            iss >> std::get_time(&endTm, "%H:%M");
                            endTm.tm_year = year;
                            endTm.tm_mon = month;
                            endTm.tm_mday = day;
                            endTime = std::mktime(&endTm);
                            //std::cout << iss.str() << "," << endTime << std::endl;
                        }
                    }
                    if (!std::getline(configFile, line)) {
                        break;  // Exit the loop when there are no more lines to read
                    }
                }
                // break;  // Exit the loop after processing the "timer" section
            }
        }

        if (now >= startTime && now < endTime) {
            if (!processFiles) {
                std::cout << "[" << std::this_thread::get_id() << "]" << "Processing started at: " << hour << ":" << minute << ":" << second << std::endl;
                processFiles = true;
            }

            // Read filenames from configuration file
            configFile.clear();  // Clear the end-of-file flag
            configFile.seekg(0, std::ios::beg);  // Reset file pointer to the beginning

            std::vector<std::string> filenames;
            bool foundFilenames = false;
            while (std::getline(configFile, line)) {
                if (line.empty()) {
                    continue;  // 跳过空行
                }
                else if (line[0] == '[') {
                    if (line.find("[filename]") != std::string::npos) {
                        foundFilenames = true;
                    }
                    else {
                        foundFilenames = false;
                        break;  // 遇到新的配置项，结束文件路径配置识别
                    }
                }
                else if (foundFilenames) {
                    filenames.push_back(line);
                }
            }

            // Process each file
            for (const std::string& filename : filenames) {
                if (fileExists(filename)) {
                    std::cout << "[" << std::this_thread::get_id() << "]" << "Processing file: " << filename <<"  Begin."<< std::endl;
                    processFile(filename);
                    std::cout << "[" << std::this_thread::get_id() << "]" << "Processing file: " << filename <<"  End. "<< std::endl;
                }
                else {
                    std::cout << "File not found: " << filename << std::endl;
                }
            }
        }
        else {
            std::time_t t2 = std::time(nullptr);
            std::cout << "[" << std::this_thread::get_id() << "]" << std::put_time(std::localtime(&t2), "%Y-%m-%d %H.%M.%S") << "  check timer."<<std::endl;
            if (processFiles) {
                std::cout << "["<<std::this_thread::get_id()<<"]" << "Processing stopped at: " << hour << ":" << minute << ":" << second << std::endl;
                processFiles = false;
            }
        }

        if (stopProcessing)
            break;
    }
}


int main3() {

    daily_logger->info("程序启动");

    // 注册SIGTERM信号处理函数
    std::signal(SIGTERM, signalHandler);

    //std::thread configFileThread(monitorConfigFile);

    while (!stop_flag) {
        // Wait for the user to stop the program or receive a signal
        std::string input;
        while (std::cin >> input) {
            if (input == "stop") {
                spdlog::info("you inpuy stop");
                daily_logger->info("程序停止");
                stop_flag = 1;
                break;
            }
        }
    }

    //configFileThread.join();

    return 0;
}

int main() {
    boost::property_tree::ptree pt;
    try {

        boost::property_tree::ini_parser::read_ini("config.ini", pt);
    }
    catch (std::exception& e)
    {
        std::cout << "不能识别的配置项:" << e.what() << std::endl;
        daily_logger->info("不能识别的配置项:{}",e.what());
        return 1;
    }


    std::string value1 = pt.get<std::string>("filename.file1");
    std::string value2 = pt.get<std::string>("filename.file2");

    std::cout << value1 << value2 << std::endl;
    daily_logger->info("行情配置1={}", value1);
    daily_logger->info("行情配置2={}", value2);
    return 0;
}