#pragma once
#pragma once
#ifndef WORK_THREAD_B_H  
#define WORK_THREAD_B_H  

class WorkThreadB {
public:
    WorkThreadB(); // ���캯��  
    ~WorkThreadB(); // ��������  
    void Start(); // �����߳�  
private:
    void ThreadFunc(); // �̺߳���  
    bool IsRunning(); // ����߳��Ƿ���������  
};

#endif // WORK_THREAD_B_H