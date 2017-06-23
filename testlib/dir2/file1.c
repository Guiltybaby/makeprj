#include "head2.h"
#include <stdio.h>
#include "comm.h"

int file2_func()
{
	printf("%s : %s\n",__FILE__,__func__);
	my_cmm_func();
	printf("%s : %s\n",__FILE__,__func__);
	return 0;
}
