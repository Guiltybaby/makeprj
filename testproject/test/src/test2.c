#include <stdio.h>
enum testenum_t{
	ONE = 1,
	TWO,
	THREE,
};

int main(int argc, char* argv[])
{
//	test1();
//	test2();
//	test3();
	printf("%d \n",sizeof(enum testenum_t));
}

