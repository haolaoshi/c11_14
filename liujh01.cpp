#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <ctime>
#include <sqlite3.h>  // ����ʹ��SQLite��Ϊ���ݿ� 
#include <filesystem> 


// ������������ͷ�ļ�
// 
namespace fs = std::filesystem;

// ���ݿ�������Ϣ
const std::string dbPath = "mydatabase.db";  // ���ݿ��ļ�·��
sqlite3* db;  // ȫ�����ݿ����Ӷ���
// �����ļ����б�
std::vector<std::string> fileList = { "f1.txt", "f2.txt", "f3.txt", "f4.txt" };

// �����ݿ�����
bool openDatabase() {
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cout << "�޷������ݿ⣺" << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    return true;
}

// �ر����ݿ�����
void closeDatabase() {
    sqlite3_close(db);
}

void updateOneFile(std::string filename)
{
    std::ifstream file(filename);
    std::string line;

    if (file.is_open()) {
        // ��ȡ�ļ���������У��
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

            // �����ļ������ݵ�ǰ����
            // �Ƚϻ������ݿ��е�����
            // �����ͬ�����ݲ����ڣ����и��²���
            // ʾ�����������ݿ����Ϊ"files"��������"column1"��"column2"
            std::string query = "SELECT * FROM files WHERE column1='" + column1 + "' AND column2='" + column2 + "'";
            sqlite3_stmt* stmt;
            int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
            if (rc == SQLITE_OK) {
                if (sqlite3_step(stmt) == SQLITE_DONE) {
                    // ���ݲ����ڣ�ִ�в������
                    query = "INSERT INTO files (column1, column2) VALUES ('" + column1 + "', '" + column2 + "')";
                    rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
                    if (rc != SQLITE_OK) {
                        std::cout << "��������ʧ�ܣ�" << sqlite3_errmsg(db) << std::endl;
                    }
                }
            }
            else {
                std::cout << "��ѯ���ݿ�ʧ�ܣ�" << sqlite3_errmsg(db) << std::endl;
            }

            sqlite3_finalize(stmt);
        }

        // �ر��ļ�
        file.close();
    }
}


// ����ļ����������ݿ�
void checkAndSyncFiles() {
    // �����ļ��б�
    for (const auto& filename : fileList) {
      
        // ���ļ�
        updateOneFile(filename);
    }
}
//// ����������ļ��Ƿ��޸Ļ����
//bool isFileModified(const std::string& filename) {
//    fs::file_time_type lastWriteTime = fs::last_write_time(filename);
//
//    // ������Ҫ�����ļ��޸Ļ���µ��ж�
//    // �Ƚ��ļ����޸�ʱ��������Ϣ
//    // ����ļ����޸Ļ���¹�������true�����򷵻�false
//}
// ����ļ��Ƿ��޸Ļ����
bool isFileModified(const std::string& filename) {
    // �Ƚ��ļ����޸�ʱ������Ԫ����
    // ʾ��������Ƚ��ļ����޸�ʱ�����ж��ļ��Ƿ��޸Ļ���¹�
    std::ifstream file(filename);
    if (file.is_open()) {
        fs::file_time_type lastWriteTime = fs::last_write_time(filename);
        //std::time_t lastModificationTime = std::filesystem::last_write_time(filename);
        // �����ݿ��б����ʱ����бȽ�
        // ����ļ����޸ģ�����true
        // ���򷵻�false
        return true;
    }

    return false;
}

// �ļ����Ͷ�ʱ���ж�
void fileMonitoring() {
    // �жϵ�ǰʱ���Ƿ����趨������ʱ�����
    // ʡ��ϸ�ڣ�����ʱ�����Ҫ��

    while (true) {
        // ִ���ļ�������
        for (const auto& filename : fileList) {
            if (isFileModified(filename)) {
                // ִ����Ӧ�Ĳ���
                // ʾ������ӡ�ļ����޸ĵ���Ϣ
                std::cout << "�ļ� " << filename << " �ѱ��޸Ļ����" << std::endl;
                updateOneFile(filename);
            }
        }

        // ����һ��ʱ����ٴ�ִ�м�����
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int main() {
    // �����ݿ�����
    if (!openDatabase()) {
        return 1;
    }

    // ִ���ļ����͸��²���
    checkAndSyncFiles();

    // ִ���ļ�������
    fileMonitoring();

    // �ر����ݿ�����
    closeDatabase();

    return 0;
}