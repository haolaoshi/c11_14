#include <iostream>
#include <fstream>
#include <chrono>
/**
#define _CRT_SECURE_NO_WARNINGS 
1 > D:\app\pg\Project1\exclusive.cpp(11, 69) : error C4996 : 'localtime' : 
    This function or variable may be unsafe.
    Consider using localtime_s instead.To disable deprecation, use _CRT_SECURE_NO_WARNINGS.See online help for details.
    **/
#include <ctime>
#include <thread>

// �������ڻ�ȡ��ǰʱ����ַ�����ʾ
std::string getCurrentTime() {
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buffer;
}

// ��������д����־�ļ�
void writeLog(const std::string& filename) {
    std::ofstream file;
    //file.open(filename, std::ios::app);  // ��׷�ӷ�ʽ���ļ�
    file.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        std::cout << "�޷�����־�ļ�" << std::endl;
        return;
    }

    while (true) {
        //std::this_thread::sleep_for(std::chrono::hours(1));  // ÿ��һ��Сʱ
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::string currentTime = getCurrentTime();
        file << currentTime << " - ��־����" << std::endl;
        file.flush();  // ˢ�»�����

        std::cout << "��д����־��" << currentTime << std::endl;
    }

    file.close();
}

int main() {
    std::string filename = "logfile.txt";

    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        std::cout << "�޷����������־�ļ�" << std::endl;
        return 1;
    }

    file.close();  // �ر��ļ�

    std::thread logThread(writeLog, filename);

    std::cout << "�� Enter ���˳�����..." << std::endl;
    std::cin.ignore();  // ��ͣ���򣬵ȴ��û��� Enter ��

    return 0;
}