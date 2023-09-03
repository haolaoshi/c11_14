#pragma once

#include <iostream>    
#include <thread>    
#include <mutex>    
#include <condition_variable>    
#include <atomic>  
#include "config.h"

class FileReader {
public:
    static FileReader& Instance() {
        static FileReader instance;
        return instance;
    }

    void Start() {
        if (isRunning_.load()) {
            std::cout << "Thread is already running." << std::endl;
            return;
        }
        isRunning_.store(true);
        thread_ = std::thread(&FileReader::Run, this);
        thread_.detach();
    }

    void Stop() {
        if (!isRunning_.load()) {
            std::cout << "Thread is not running." << std::endl;
            return;
        }
        isRunning_.store(false);
        cond_.notify_one();  
    }

private:
    FileReader() {}
    ~FileReader() {}

    void Run() {
        while (isRunning_.load()) { 
            std::cout << "Processing file... " << std::this_thread::get_id() << std::endl;
             
            std::unique_lock<std::mutex> lock(mutex_);
            //cond_.wait(lock, [this]() { return !isRunning_.load(); }); // 等待直到线程停止  
            // 获取配置选项  
            std::map<std::string, std::string>  files;
            //auto files = Config::getInstance()->getConfigOptions("filename");
            try {
                Config* config = Config::getInstance();
                files = config->getConfigOptions("filename");
               
            }
            catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }

            for (auto f : files)
            {
                std::cout << "Processing file " << f.second << " ... " << std::this_thread::get_id() << std::endl;
                processFile(f.second); 
            } 

            std::this_thread::sleep_for(std::chrono::seconds(3)); // 示例逻辑，根据实际情况修改    
        }
    }

    int processFile(const std::string);

    std::mutex mutex_;
    std::condition_variable cond_;
    std::thread thread_;
    std::atomic<bool> isRunning_{ false }; // 表示线程是否正在运行的标志位，使用std::atomic确保多线程安全    
};