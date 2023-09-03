#include "a_thread.h"  
#include <sqlite3.h>  
#include <fstream>  
#include <sstream>  
#include <string>  

// 假设这是全局单例的Config类  
#include "config.h"  

// 构造函数  
WorkThreadA::WorkThreadA() {
}

// 析构函数  
WorkThreadA::~WorkThreadA() {
}

// 启动线程  
void WorkThreadA::Start() {
    std::thread t(&WorkThreadA::ThreadFunc, this); // 创建线程，执行ThreadFunc函数  
    t.detach(); // 允许线程独立运行  
}

// 线程函数  
void WorkThreadA::ThreadFunc() {
    // 读取配置文件列表中的文件1  
    std::string file1 = config::Instance().GetFilename("file1");
    std::ifstream file(file1);
    std::stringstream buffer;
    buffer.rdbuf(file.rdbuf());
    std::string content = buffer.str();
    file.close();

    // 从content中解析出列2和列3的数据，并插入到sqlite3数据库中  
    std::vector<std::string> lines = std::istringstream(content).readlines();
    for (const auto& line : lines) {
        std::vector<std::string> columns = std::istringstream(line).split(',');
        if (columns.size() >= 2) {
            std::string stock_name = columns[1];
            std::string price = columns[2];

            // 插入数据到sqlite3数据库中  
            sqlite3* db;
            int rc = sqlite3_open(":memory:", &db); // 在内存中创建一个sqlite3数据库（为了简化代码，这里不在磁盘上创建数据库）  
            if (rc != SQLITE_OK) {
                std::cerr << "无法打开数据库: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                continue;
            }
            std::string sql = "INSERT INTO stock_name (name, price) VALUES (?, ?)";
            sqlite3_stmt* stmt;
            rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr, nullptr);
            if (rc != SQLITE_OK) {
                std::cerr << "无法准备SQL语句: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                continue;
            }
            rc = sqlite3_bind_text(stmt, 1, stock_name.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK) {
                std::cerr << "无法绑定SQL语句的第一个参数: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                continue;
            }
            rc = sqlite3_bind_text(stmt, 2, price.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK) {
                std::cerr << "无法绑定SQL语句的第二个参数: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                continue;
            }
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                std::cerr << "无法执行SQL语句: " << sqlite3_errmsg(db) << std::endl;
            }
            sqlite3_finalize(stmt);
            sqlite3_close(db);
        }
    }
}