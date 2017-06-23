#include <stdlib.h>
#include <stdio.h>
void* __wrap_malloc(size_t size)
{
	printf("111 \n");
	return malloc(size);
}
