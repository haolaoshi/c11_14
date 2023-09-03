#include "a_thread.h"  
#include <sqlite3.h>  
#include <fstream>  
#include <sstream>  
#include <string>  

// ��������ȫ�ֵ�����Config��  
#include "config.h"  

// ���캯��  
WorkThreadA::WorkThreadA() {
}

// ��������  
WorkThreadA::~WorkThreadA() {
}

// �����߳�  
void WorkThreadA::Start() {
    std::thread t(&WorkThreadA::ThreadFunc, this); // �����̣߳�ִ��ThreadFunc����  
    t.detach(); // �����̶߳�������  
}

// �̺߳���  
void WorkThreadA::ThreadFunc() {
    // ��ȡ�����ļ��б��е��ļ�1  
    std::string file1 = config::Instance().GetFilename("file1");
    std::ifstream file(file1);
    std::stringstream buffer;
    buffer.rdbuf(file.rdbuf());
    std::string content = buffer.str();
    file.close();

    // ��content�н�������2����3�����ݣ������뵽sqlite3���ݿ���  
    std::vector<std::string> lines = std::istringstream(content).readlines();
    for (const auto& line : lines) {
        std::vector<std::string> columns = std::istringstream(line).split(',');
        if (columns.size() >= 2) {
            std::string stock_name = columns[1];
            std::string price = columns[2];

            // �������ݵ�sqlite3���ݿ���  
            sqlite3* db;
            int rc = sqlite3_open(":memory:", &db); // ���ڴ��д���һ��sqlite3���ݿ⣨Ϊ�˼򻯴��룬���ﲻ�ڴ����ϴ������ݿ⣩  
            if (rc != SQLITE_OK) {
                std::cerr << "�޷������ݿ�: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                continue;
            }
            std::string sql = "INSERT INTO stock_name (name, price) VALUES (?, ?)";
            sqlite3_stmt* stmt;
            rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr, nullptr);
            if (rc != SQLITE_OK) {
                std::cerr << "�޷�׼��SQL���: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                continue;
            }
            rc = sqlite3_bind_text(stmt, 1, stock_name.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK) {
                std::cerr << "�޷���SQL���ĵ�һ������: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                continue;
            }
            rc = sqlite3_bind_text(stmt, 2, price.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK) {
                std::cerr << "�޷���SQL���ĵڶ�������: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                continue;
            }
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                std::cerr << "�޷�ִ��SQL���: " << sqlite3_errmsg(db) << std::endl;
            }
            sqlite3_finalize(stmt);
            sqlite3_close(db);
        }
    }
}