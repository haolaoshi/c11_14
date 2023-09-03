#ifndef WORK_THREAD_A_H  
#define WORK_THREAD_A_H  

#include <mutex>  
#include <condition_variable>  
#include <future>  
#include <thread>  
#include <atomic>  

class WorkThreadA {
public:
    WorkThreadA();
    ~WorkThreadA();
    void Start();
private:
    void ThreadFunc();
    bool IsRunning();

    std::mutex mtx_; // 互斥锁，用于保护共享资源  
    std::condition_variable cv_; // 条件变量，用于实现线程间的通信  
    std::atomic<bool> running_; // 原子变量，标记线程是否正在运行  
};

#endif // WORK_THREAD_A_H