/*
�˹���001
https://zhuanlan.zhihu.com/p/637992012
*/

#include <string>
#include <iostream>

int main()
{
	//test01
	std::string s1 = std::string("hello") + "world";
	std::string s2 = "hello" + std::string("world");
	std::string s3 = std::string("hello") + std::string("world");
	/*
	std::string ������API��resize��reserve����֪������֮���������

	resize��Ӧ����size��resize���Ըı��ַ����Ĵ�С��reserve��Ӧ����capacity��reserveֻ�ܸı�capacity�Ĵ�С��
	��resize����Ĳ���С���ַ�����szieʱ��������ַ����ᱻ��ȡ����reserve����Ĳ���С��capacityʱ��reserveʲôҲ��������
	��resize����Ĳ��������ַ�����szieʱ�����ӵ��ַ����ᱻĬ�ϳ�ʼ������reserve����Ĳ�������capacityʱ��capacity�ᱻ���ݡ�

	*/
	s2.resize(10);
	s2.reserve(1);

	/*
	��Ҫ����Ϊ�����ϵĿ�����at��Ȼ��֤�˲��ᳬ���ַ�����Χ��������Χ�׳��쳣����
	�������ܵ����±����������������еá�Ϊ�˰�ȫʹ��at��Ϊ������ʹ���±������C++��������ѡ�����ѡ���������

	*/
	std::cout << s2.at(20);



}