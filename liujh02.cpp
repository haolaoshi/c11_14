#include <stdio.h>

//8位二进制数据的补码
//同1为1，否则为0。

void dis32bin2(int data)
{
	int i = 32;
	while (i--)
	{
		int d = 1 << i; //左第i位置1
		if (data & (1 << i))
			printf("1");
		else
			printf("0");
		if (i % 4 == 0)
		{
			if (i % 8 == 0 )
				printf(" ");
			else
				printf("-");

		}
	}
	putchar(10);
}
int main()
{
	int a = 3;
	dis32bin2(a);
	dis32bin2(9);
	dis32bin2(a|9);
	dis32bin2(~0);
	dis32bin2(~1);
	//相异者1，相同者0
	dis32bin2(8 ^ 9);
	return 0;
}