#pragma once
#include <iostream>  
#include <thread>  
#include <mutex>  
#include <condition_variable>  

class TextToSpeech{
public:
    static TextToSpeech& Instance() {
        static TextToSpeech instance;
        return instance;
    }

    void Start() {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!isRunning_) {
            std::cout << "TextToSpeech starting by :" << std::this_thread::get_id() << std::endl;
            isRunning_ = true;
            thread_ = std::thread(&TextToSpeech::Run, this);
            thread_.detach();
        }
    }

    void Stop() {
        std::unique_lock<std::mutex> lock(mutex_);
        if (isRunning_) {
            isRunning_ = false;
            cond_.notify_one(); // 通知正在等待的线程退出  
        }
    }

private:
    TextToSpeech() {}
    ~TextToSpeech() {}

    void Run() {
        while (isRunning_) {
            // 线程运行的逻辑  
            std::cout << "TextToSpeech file... " << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(3)); // 示例逻辑，根据实际情况修改  
        }
    }

    std::mutex mutex_;
    std::condition_variable cond_;
    std::thread thread_;
    bool isRunning_{ false }; // 表示线程是否正在运行的标志位  
};