#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern void heap_check_list();
char str[10] = {0x0,0x1,0x2,0x3,0x4, 0x5,0x6,0x7,0x8,0x9,};


char *strncpy_2(char *dest, const char *src, size_t n)
{
   size_t i;

   for (i = 0; i < n && src[i] != '\0'; i++)
       dest[i] = src[i];
   for ( ; i < n; i++)
       dest[i] = '\0';

   return dest;
}


char* sl_wrap_strncpy(char* dest, const char* src, size_t len)  
{  
	//assert(dest!=NULL && src!=NULL);  
	char* temp=dest;  
	int i=0;  
	printf("temp = %p src = %p len = %zd \n",temp,src,len);
	while(i++ < len  && (*temp++ = *src++)!='\0')  
	{}  
	printf("temp = %p src = %p len = %zd \n",temp,src,len);

	if(*(temp)!='\0')  
		*temp='\0';  
	return dest;  
} 

void pb_session_set_int(int a,char* b,int c)
{
	int len = strlen(b) + 1;
	printf("strlen = %d \n",len);
	char* strbuf = (char*)malloc(len);
	strncpy(strbuf,b,len);
	heap_check_list();
	strncpy_2(strbuf,b,len);
	heap_check_list();
	sl_wrap_strncpy(strbuf,b,len);
	heap_check_list();
}


void main()
{
	int i;
	int*ptr = (int*)&str[1];

		pb_session_set_int(0, "pb.silead.neo.far.shift", 3); //default 3


	printf("str = %p ptr = %p ptr = %x\n",str,ptr,*ptr);
//	*ptr = 0xdeadbeef;
	int head = 0xdeadbeef;
	memcpy(ptr,&head,4);
	for(i = 0; i < 10; i++)
		printf("str[%d] = 0x%x \n",i,str[i]);

	int*p = malloc(5);
	memset(p,0,6);
	heap_check_list();
	free(p);
}


