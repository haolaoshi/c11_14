#pragma once 
#ifndef WORK_THREAD_C_H  
#define WORK_THREAD_C_H  

class WorkThreadC {
public:
    WorkThreadC(); // 构造函数  
    ~WorkThreadC(); // 析构函数  
    void Start(); // 启动线程  
private:
    void ThreadFunc(); // 线程函数  
    bool IsRunning(); // 检查线程是否正在运行  
};

#endif // WORK_THREAD_C_H