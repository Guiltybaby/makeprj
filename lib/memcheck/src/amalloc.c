#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MEM_TEST_FTA

#ifdef MEM_TEST_FTA
#define   printf printf
#define   A_LOGD printf
#define   A_LOGE printf
static unsigned int allMallocSize = 0;
#define CHECK_SIZE 4
#endif

#define malloc __real_malloc
#define free __real_free
 
static int log_switch = 0;
void mem_log_switch()
{
	log_switch = !log_switch;
}

typedef struct MemoryBlock{
	struct MemoryBlock* next;
	void* p;
}MemoryBlock;

static struct MemoryBlock* ListHead = NULL;

static void check_mem(void *mem);

void heap_check_list()
{
	A_LOGD("check enter \n");
	struct MemoryBlock* list = ListHead;
	while(list = list->next)
	{
		check_mem(list->p);
	}
	A_LOGD("check finish \n");
}


static void heap_list_remove(void*p,struct MemoryBlock* listhead)
{
	struct MemoryBlock* list;
	struct MemoryBlock* prev;
	prev = listhead;
	list = listhead->next;
	while(list){
		if(list->p == p){
			prev->next = list->next;
			free(list);
		}
		prev = list;
		list = list->next;
	}
}

static void heap_list_add_head(void*p,struct MemoryBlock* listhead)
{
	struct MemoryBlock* head_next = NULL;
	struct MemoryBlock* list = NULL;
	list = malloc(sizeof(struct MemoryBlock));
	memset(list,0,sizeof(struct MemoryBlock));
	list->p = p;
	list->next = listhead->next;
	listhead->next = list;
}

inline static void heap_list_head_init()
{
	if(!ListHead){
		ListHead = malloc(sizeof(struct MemoryBlock));
		ListHead->p = (void*)(-1);
		ListHead->next = NULL;
	}
}

const int Check_Head_Int = 0xdeadbeef;
const int Check_Tail_Int = 0xbabeface;
static void check_mem(void *mem)
{
	char * realAddr = (char*)mem;
	int* temp = 0;
	int *p = (int *) realAddr;

	temp = (int*)(realAddr + 4);

	if(memcmp(temp,&Check_Head_Int,sizeof(int)))
	{
		A_LOGE("check buff begin corupt error jeff allMallocSize = %d \n",allMallocSize);
		A_LOGE("check buff begin corupt error jeff *temp = %x \n",*temp);
	}

	int i;
	for(i = 0; i < CHECK_SIZE ; i ++)
	{
		temp = (int*)(realAddr + *p + 8 + i * 4);
		if(memcmp(temp,&Check_Tail_Int,sizeof(int)))
		{
			A_LOGE("check buff end corupt error jeff allMallocSize = %d realAddr = %p\n",allMallocSize,realAddr);
			A_LOGE("check buff end corupt error jeff *temp = %x i = %d size = %d \n",*temp,i,*p);
		}
	}
}
void* __wrap_malloc(size_t size)
{
    if (size == 0) {
        A_LOGE("malloc size error:%d", size);
		return NULL;
    }
	heap_list_head_init();
#ifdef MEM_TEST_FTA
    char * p =  (char *)malloc(size + 8 + CHECK_SIZE*4);
    if (p == NULL) {
        //A_LOGD("jeff malloc failed size :%d addr: %x", size, p);
    }
	heap_list_add_head((void*)p,ListHead);
	if((int)p == 0x600390)
		A_LOGD("xxxjeff malloc allMallocSize = %d addr = %p size = %d\n",allMallocSize,p,size);
    int *sizeaddr = (int*)p;
    *sizeaddr = size;
    allMallocSize += size;
	sizeaddr = (int*)(p + 4);
	memcpy(sizeaddr,&Check_Head_Int,sizeof(int));
	int i;
	for(i = 0; i < CHECK_SIZE ; i ++){
		sizeaddr = (int*)(p + size + 8 + i * 4);
		memcpy(sizeaddr,&Check_Tail_Int,sizeof(int));
	}
    p += 8;
	//A_LOGD("enter");
	if(log_switch)
		A_LOGD("jeff malloc allMallocSize = %d addr = %p size = %d\n",allMallocSize,sizeaddr,size);
    return (void*)p;
#else
    return malloc(size);
#endif
}


void __wrap_free(void* mem)
{
    if (mem == NULL) {
        return;
    }

#ifdef MEM_TEST_FTA
    char * realAddr = (char*)mem;
    realAddr -= 8;
	int *p = (int *) realAddr;
	allMallocSize -= *p;
	check_mem((void*)realAddr);
	if(log_switch)
		A_LOGD("jeff free allMallocSize = %d addr = %p \n",allMallocSize,realAddr);
	heap_list_remove((void*)realAddr,ListHead);
    free((void*)realAddr);
    return;
#else
    free(mem);
    return;
#endif
}

