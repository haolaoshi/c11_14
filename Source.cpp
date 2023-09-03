#include <bitset>
#include <iostream>
#include <list>
#include <vector>
#include <complex>

using namespace std;


/*01 - variadic template*/
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
/*01 - end*/

/*07 - explicit for ctors taking more than one argu*/
struct Complex
{
	int real, img;

	explicit
	Complex(int re, int im = 0) :real(re), img(im) {}

	Complex operator+(const Complex& x)
	{
		return Complex((real + x.real), (img + x.img));
	}
};
/*07 - end */

/*09 - =default  =delete */
class Zoo {
public:
	Zoo(int il, int im) :d1(il), d2(im) {}
	//copy
	Zoo(const Zoo&) = delete;
	//move
	Zoo(Zoo&&) = default;
	

	//copy assignment
	Zoo& operator=(const Zoo&) = default;

	//moveassignment 
	Zoo& operator=(const Zoo&&) = delete;

private:
	int d1, d2;
};

class Foo
{
public:
	Foo(int i) :_i(i) {}
	Foo() = default;

	Foo(const Foo& x) :_i(x._i) {}//copy ctor
	//Foo(const Foo&) = delete;
	//Foo(const Foo&) = default;

	Foo& operator=(const Foo& x) { _i = x._i; return *this; }
	//Foo& operator=(const Foo& x) = default;
	//Foo& operator=(const Foo& x) = delete;

	//void func1() = default;
	void func2() = delete;

private:
	int _i;
};
/*09 - end */


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
	vector<int> v1 {2, 5, 7, 9, 11, 13, 17, 19};
	vector<int> v2({ 10,30,40,50,60,70 });
	vector<int> v3;
	v3 = { 1,2,3,4,5 };
	v3.insert(v3.begin() + 2, { 0,1,2,3,4 });
	for (auto i : v3)
		cout << i << ' ';
	cout << endl;
	cout << max({ string("Ace"),string("Stratege"),string("Sabmar"),string("Barkly"),string("Black") })<<endl;
	cout << min({ string("Ace"),string("Stratege"),string("Sabmar"),string("Barkly"),string("Black") })<<endl;	 
	cout << max({ 54,16,28,99,5 })<<endl;
	cout << min({ 54,16,28,99,5 })<<endl;
	/*07 - explicit for ctors taking more than one argu*/
	/* one arument non explicti const before */
	Complex c1(12, 6);
	//Complex c2 =  c1 + 5;

	/*08 - for */
	for (auto ele : v3) {
		cout << ele << ' ';
	}
	cout << endl;

	for (auto& ele : v3) {
		ele *= 3;
		cout << ele << ' ';
	}
	cout << endl;
	/*09 - BigThree =default  =delete */
	//delete 可以用于任何函数 ，=0只能用在虚函数 上
	Foo f1(5);

}