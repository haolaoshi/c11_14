#include "b_thread.h"  

// ���캯��  
WorkThreadB::WorkThreadB() {
    // ��ʼ�������߳�A�ĳ�Ա����  
}

// ��������  
WorkThreadB::~WorkThreadB() {
    // �������߳�A����Դ  
}

// �����߳�  
void WorkThreadB::Start() {
    std::thread t(&WorkThreadB::ThreadFunc, this); // �����̣߳�ִ��ThreadFunc����  
    t.detach(); // �����̶߳�������  
}

// �̺߳���  
void WorkThreadB::ThreadFunc() {
    // ������ִ�й����߳�A���߼�  
}

// ����߳��Ƿ���������  
bool WorkThreadB::IsRunning() {
    // ���������߳��Ƿ��������У�������Ӧ�Ľ��  
    return true; // �����߳���������  
}