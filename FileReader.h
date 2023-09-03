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
            //cond_.wait(lock, [this]() { return !isRunning_.load(); }); // �ȴ�ֱ���߳�ֹͣ  
            // ��ȡ����ѡ��  
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

            std::this_thread::sleep_for(std::chrono::seconds(3)); // ʾ���߼�������ʵ������޸�    
        }
    }

    int processFile(const std::string);

    std::mutex mutex_;
    std::condition_variable cond_;
    std::thread thread_;
    std::atomic<bool> isRunning_{ false }; // ��ʾ�߳��Ƿ��������еı�־λ��ʹ��std::atomicȷ�����̰߳�ȫ    
};