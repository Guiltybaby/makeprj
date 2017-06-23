#include <stdio.h>
static int a = 0;
static int a1 = 1;
static int a2;
static const int con = 10;
static const int con1;
const int con2 = 10;
const int con3;
int b = 0;
int b1 = 10;
int b2;
const char* str = "hell";
void func(int t){
	printf("%d \n",t);
	printf("%s \n",str);
}
int main()
{
	static int c = 0;
	static int c1 = 10;
	static int c2;
	int d = 0;
	int d2;
	func(c + c1 + d + d2);

}
