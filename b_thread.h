#pragma once
#pragma once
#ifndef WORK_THREAD_B_H  
#define WORK_THREAD_B_H  

class WorkThreadB {
public:
    WorkThreadB(); // 构造函数  
    ~WorkThreadB(); // 析构函数  
    void Start(); // 启动线程  
private:
    void ThreadFunc(); // 线程函数  
    bool IsRunning(); // 检查线程是否正在运行  
};

#endif // WORK_THREAD_B_H