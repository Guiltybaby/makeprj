#include "head1.h"
#include <stdio.h>
#include "head3.h"

int file1_func()
{
	printf("%s : %s\n",__FILE__,__func__);
	file3_func();
	return 0;
}
