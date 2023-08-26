#include <bitset>
#include <iostream>
#include <list>
#include <vector>
#include <complex>

using namespace std;


void print()
{

}

template<typename... Types>
void print(const Types&... args)
{
	std::cout << sizeof ...(args) << std::endl;
}

template <typename T, typename... Types>
void print(const T& firstArg, const Types&... args)
{
	size_t n = sizeof ...(args);

	cout << firstArg << " " << n << endl;
	print(args...);
}

int main()
{
	/*01 - variadic template*/
	print(7.5, "hello", std::bitset<16>(377), 52);
	/*02 - space in template expression */
	vector<list<std::string>> vs1;
	/*03 - nullptr instead of 0 or NULL (nullptr)*/
	char* cnull = nullptr;
	/*04 - auto type */
	auto i = 42;
	auto l = [](int x)->bool { return x * x; };
	/*05 - uniform initialization 观念*/
	int values[]{ 1,2,3 };
	vector<int> v{2, 3, 9, 7, 11, 13, 19};
	vector<string> cities{"Peiking", "New Yourk", "Tokyo", "Cicago"};
	complex<double> cx{4.0, 3.0};
	int* q{};//q has a intialized nullptr;
	//int x{ 5.3 };  Error
	/*06 - initialzied list 背后是一个容器array */
	/* refer to the elements of this array without containing them*/
	/* https://en.cppreference.com/w/cpp/utility/initializer_list */

}