#pragma once 
#ifndef WORK_THREAD_C_H  
#define WORK_THREAD_C_H  

class WorkThreadC {
public:
    WorkThreadC(); // ���캯��  
    ~WorkThreadC(); // ��������  
    void Start(); // �����߳�  
private:
    void ThreadFunc(); // �̺߳���  
    bool IsRunning(); // ����߳��Ƿ���������  
};

#endif // WORK_THREAD_C_H