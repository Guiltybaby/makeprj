#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <string>
#include "testcpp.h"
#include "test1.h"

using namespace std;

class A{
	public:
	A()
	{

	}
	virtual void print() = 0;// { printf("this is class A \n");}
	int a;
};
class B{
	public:
	B(){};
	virtual void print() { printf("this is class B \n");}
	int b;
};

class C : public A
{
	public:
	C(): A()
	{
	}
	void print() { printf("this is class C \n"); }	
	
};
class D
{
	int a;
	void print() { printf("this is class C \n"); }	

};

static void usage()
{
	printf("-addr \n");
	printf("-class \n");
	printf("-child \n");
}

void myprint(char* tmp)
{
	printf("pid = %d tid =%ld str = %s \n",getpid(), gettid(), tmp);
}

static void testaddr(int a, int b)
{
	C c;
	string str;
//	cin >> str;
	cout << str << "\n";
	char* ptmp = (char *)"asdfasdf";
	char ptmp1[20] = "adsf";
	printf("ptmp = %p \n&ptmp = %p \n",ptmp,&ptmp);
	printf("ptmp1 = %p \n&ptmp1 = %p \n",ptmp1,&ptmp1);
	printf("a = %p \nb = %p\n",&a,&b);
	int tmp = 0;
	static int tmp1;
	ptmp = new char;
	int* ptmp2 = (int *)malloc(4);
	printf("stack = %p \nstatic = %p \ndynamic1 = %p \ndymamic2 = %p \n",&tmp,&tmp1,ptmp,ptmp2);	
	delete ptmp;
	free(ptmp2);
	printf("function = %p \n",myprint);

}

static void testclass()
{
	Time time;
	time.Show();
	printf("stack class %p \n",&time);	
	printf("stack class method %p \n",reinterpret_cast<void*>(&Time::Show));	
	printf("stack class data %p \n",&time.test);	
	printf("class size = %lu \n",sizeof(D));
	Time time1(8,0);
	Time time2(2,30);
	Time total(11,20);
	total = time2.Sum(time1);
	total.Show();
}

static void test_child()
{
	C c;
	c.print();
}
int main(int argc, char* argv[])
{
	int i;
	int usageflag = 1;
	for(i = 0; i < argc; i++)
	{
		if(strcmp(argv[i],"-addr") == 0)
		{
			usageflag = 0;
			testaddr(1,2);
			break;
		}
		else if(strcmp(argv[i],"-class") == 0)
		{
			usageflag = 0;
			testclass();
		}
		else if(strcmp(argv[i],"-child") == 0)
		{
			usageflag = 0;
			test_child();
			break;
		}
//		else if(strcmp(argv[i],"-h") == 0 || argc <= 1)
		{
		}
	}	
	if(usageflag)
	{
		usage();
	}
	return 1;
}

