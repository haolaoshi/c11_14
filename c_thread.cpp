#include "c_thread.h"  


// ���캯��  
WorkThreadC::WorkThreadC() {
    // ��ʼ�������߳�A�ĳ�Ա����  
}

// ��������  
WorkThreadC::~WorkThreadC() {
    // �������߳�A����Դ  
}

// �����߳�  
void WorkThreadC::Start() {
    std::thread t(&WorkThreadC::ThreadFunc, this); // �����̣߳�ִ��ThreadFunc����  
    t.detach(); // �����̶߳�������  
}

// �̺߳���  
void WorkThreadC::ThreadFunc() {
    // ������ִ�й����߳�A���߼�  
}

// ����߳��Ƿ���������  
bool WorkThreadC::IsRunning() {
    // ���������߳��Ƿ��������У�������Ӧ�Ľ��  
    return true; // �����߳���������  
}