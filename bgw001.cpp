/*
八股文001
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
	std::string 有两个API，resize和reserve，你知道它们之间的区别吗？

	resize对应的是size，resize可以改变字符串的大小。reserve对应的是capacity，reserve只能改变capacity的大小。
	当resize传入的参数小于字符串的szie时，多余的字符串会被截取。当reserve传入的参数小于capacity时，reserve什么也不会做。
	当resize传入的参数大于字符串的szie时，增加的字符串会被默认初始化。当reserve传入的参数大于capacity时，capacity会被扩容。

	*/
	s2.resize(10);
	s2.reserve(1);

	/*
	主要是因为性能上的考量。at虽然保证了不会超出字符串范围（超出范围抛出异常），
	但是性能低于下标操作。这就是有舍有得。为了安全使用at，为了性能使用下标操作。C++给了你多个选择，如何选择看你的需求。

	*/
	std::cout << s2.at(20);



}