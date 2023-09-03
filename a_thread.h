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

    std::mutex mtx_; // �����������ڱ���������Դ  
    std::condition_variable cv_; // ��������������ʵ���̼߳��ͨ��  
    std::atomic<bool> running_; // ԭ�ӱ���������߳��Ƿ���������  
};

#endif // WORK_THREAD_A_H