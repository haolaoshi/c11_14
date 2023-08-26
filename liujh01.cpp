#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <ctime>
#include <sqlite3.h>  // 假设使用SQLite作为数据库 
#include <filesystem> 


// 添加其他所需的头文件
// 
namespace fs = std::filesystem;

// 数据库连接信息
const std::string dbPath = "mydatabase.db";  // 数据库文件路径
sqlite3* db;  // 全局数据库连接对象
// 定义文件名列表
std::vector<std::string> fileList = { "f1.txt", "f2.txt", "f3.txt", "f4.txt" };

// 打开数据库连接
bool openDatabase() {
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cout << "无法打开数据库：" << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    return true;
}

// 关闭数据库连接
void closeDatabase() {
    sqlite3_close(db);
}

void updateOneFile(std::string filename)
{
    std::ifstream file(filename);
    std::string line;

    if (file.is_open()) {
        // 读取文件内容逐行校验
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string column1;
            std::string column2;
            if (std::getline(iss, column1, '|')) {
              

                if (std::getline(iss, column2, '|')) {
                     
                    std::cout << "Number: " << column1 << std::endl;
                    std::cout << "Name: " << column2 << std::endl;
                }
                else {
                    std::cerr << "Error: Unable to read the second column." << std::endl;
                }
            }
            else {
                std::cerr << "Error: Unable to read the first column." << std::endl;
            }

            // 解析文件行内容的前两列
            // 比较或检查数据库中的数据
            // 如果不同或数据不存在，进行更新操作
            // 示例：假设数据库表名为"files"，有两列"column1"和"column2"
            std::string query = "SELECT * FROM files WHERE column1='" + column1 + "' AND column2='" + column2 + "'";
            sqlite3_stmt* stmt;
            int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
            if (rc == SQLITE_OK) {
                if (sqlite3_step(stmt) == SQLITE_DONE) {
                    // 数据不存在，执行插入操作
                    query = "INSERT INTO files (column1, column2) VALUES ('" + column1 + "', '" + column2 + "')";
                    rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
                    if (rc != SQLITE_OK) {
                        std::cout << "插入数据失败：" << sqlite3_errmsg(db) << std::endl;
                    }
                }
            }
            else {
                std::cout << "查询数据库失败：" << sqlite3_errmsg(db) << std::endl;
            }

            sqlite3_finalize(stmt);
        }

        // 关闭文件
        file.close();
    }
}


// 检查文件并更新数据库
void checkAndSyncFiles() {
    // 遍历文件列表
    for (const auto& filename : fileList) {
      
        // 打开文件
        updateOneFile(filename);
    }
}
//// 函数：检查文件是否被修改或更新
//bool isFileModified(const std::string& filename) {
//    fs::file_time_type lastWriteTime = fs::last_write_time(filename);
//
//    // 根据需要进行文件修改或更新的判断
//    // 比较文件的修改时间等相关信息
//    // 如果文件被修改或更新过，返回true，否则返回false
//}
// 检查文件是否被修改或更新
bool isFileModified(const std::string& filename) {
    // 比较文件的修改时间或相关元数据
    // 示例：假设比较文件的修改时间来判断文件是否被修改或更新过
    std::ifstream file(filename);
    if (file.is_open()) {
        fs::file_time_type lastWriteTime = fs::last_write_time(filename);
        //std::time_t lastModificationTime = std::filesystem::last_write_time(filename);
        // 与数据库中保存的时间进行比较
        // 如果文件被修改，返回true
        // 否则返回false
        return true;
    }

    return false;
}

// 文件监测和定时器判断
void fileMonitoring() {
    // 判断当前时间是否处于设定的运行时间段内
    // 省略细节，假设时间符合要求

    while (true) {
        // 执行文件监测操作
        for (const auto& filename : fileList) {
            if (isFileModified(filename)) {
                // 执行相应的操作
                // 示例：打印文件被修改的消息
                std::cout << "文件 " << filename << " 已被修改或更新" << std::endl;
                updateOneFile(filename);
            }
        }

        // 休眠一段时间后再次执行监测操作
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int main() {
    // 打开数据库连接
    if (!openDatabase()) {
        return 1;
    }

    // 执行文件检查和更新操作
    checkAndSyncFiles();

    // 执行文件监测操作
    fileMonitoring();

    // 关闭数据库连接
    closeDatabase();

    return 0;
}