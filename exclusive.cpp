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

// 函数用于获取当前时间的字符串表示
std::string getCurrentTime() {
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buffer;
}

// 函数用于写入日志文件
void writeLog(const std::string& filename) {
    std::ofstream file;
    //file.open(filename, std::ios::app);  // 以追加方式打开文件
    file.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        std::cout << "无法打开日志文件" << std::endl;
        return;
    }

    while (true) {
        //std::this_thread::sleep_for(std::chrono::hours(1));  // 每隔一个小时
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::string currentTime = getCurrentTime();
        file << currentTime << " - 日志内容" << std::endl;
        file.flush();  // 刷新缓冲区

        std::cout << "已写入日志：" << currentTime << std::endl;
    }

    file.close();
}

int main() {
    std::string filename = "logfile.txt";

    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        std::cout << "无法创建或打开日志文件" << std::endl;
        return 1;
    }

    file.close();  // 关闭文件

    std::thread logThread(writeLog, filename);

    std::cout << "按 Enter 键退出程序..." << std::endl;
    std::cin.ignore();  // 暂停程序，等待用户按 Enter 键

    return 0;
}