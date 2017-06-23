/*  ====================================================================================================
**
**  ---------------------------------------------------------------------------------------------------
**
**    File Name:      aheap.cpp
**    
**    Description:    This file contains the implementation of OS wrapper.
**
**                    this is kernal code of SW framework.
**                    It contributes one of functionalities of SW Platform. 
**                    If the checkin is CR not PR, to add change History to this file head part 
**                    will be appreciated.
**
**  ---------------------------------------------------------------------------------------------------
**
**  Author:			Warren Zhao
**
** -------------------------------------------------------------------------
**
**    Change History:
**    
**    Initial revision
**
**====================================================================================================*/

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "aheap.h"
#include "alog.h"

//******************************************************************************
//
// Function Name:    AHEAP_Init
// 
// Description:        This function creates and initializes the system heap.
//
// Notes:
//
//****************************************************************************** 
void AHEAP_Init(
    void    *HeapStart,
    UInt32    HeapSize
    )
{
}

//******************************************************************************
//
// Function Name:    AHEAP_Alloc
// 
// Description:        This function allocates a block of memory from the heap.
//
// Notes:
//
//*****************************************************************************
void *dbAHEAP_Alloc(size_t size, const char* file, UInt16 line)
{
    return amem_alloc(size, file, (int)line);
}

//******************************************************************************
//
// Function Name:    AHEAP_Delete
// 
// Description:        This function frees the block of memory associated with the
//                    passed pointer.
//
// Notes:
//
//******************************************************************************
void dbAHEAP_Delete(void* ptr, const char* file, UInt16 line)
{
    return amem_free(ptr, file, (int)line);
}

//******************************************************************************
//
// Function Name: AHEAP_TraceHeapUsage
// 
// Description: This function display the Heap Usage.
//
// Notes:
//
//******************************************************************************
void AHEAP_TraceHeapUsage(
    UInt8  *str
    )
{
}

void AHEAP_EnableTrace(void)
{
}

//******************************************************************************
//
// Function Name: AHEAP_ConsumptionReport
// 
// Description: This function returns all memory pool consumption status
//
// Notes:
//
//******************************************************************************
void AHEAP_ConsumptionReport(
    UInt32    *mem_pool_size,           // Total heap size in memory pool
    UInt32    *mem_pool_free_size,      // Current free size in memory pool
    UInt32    *partition_pool_size,     // Total heap size in partition pool
    UInt8    *partition_pool_info       // This one should be big enough to hold
                                        // all partition data the format is
                                        // [ pool_size(num Of 64), Max_number, left_number ]
                                        // if pool_size=0 the parameter end.
                                        // ( maximum buffer size = 3*10 )
    )
{
}

void AHEAP_InitTraceMem( 				// Initialize trace heap memory
	void   *HeapStart,   				// Pointer to start of heap
	UInt32 HeapSize						// Number of bytes in heap
	);

void *AHEAP_AllocTraceMem(				// Allocates N bytes from the heap
	UInt32 size	 						// Number of bytes to allocate
	)
{
	return NULL;
}

void AHEAP_DeleteTraceMem(				// Deallocates the block pointed to by ptr
	void *ptr							// Pointer to block to deallocate
	)
{
}
//******************************************************************************
//
// Function Name: AHEAP_MediaZoneFree
//
// Description: This function returns media memory free status
//
// Notes:
//
//******************************************************************************
UInt32 AHEAP_MediaZoneFree(void)
{
	FILE *free_pages_fp;
	char free_pages_str[128];
	char *free_pages_pos;
	const char* free_pages_cmd = "cat /proc/zoneinfo |grep nr_free|tail -1";
	int free_mem_size;
	const int page_size = 0x1000;//4K

	if((free_pages_fp = popen(free_pages_cmd, "r")) == NULL)
	{
		A_LOGD("popen() error!\n");
		return 0;
	}

	while(fgets(free_pages_str, sizeof(free_pages_str), free_pages_fp))
	{
		A_LOGD("%s",free_pages_str);
	}
	pclose(free_pages_fp);

	free_pages_pos = strstr(free_pages_str, "nr_free_pages ");
	if(NULL == free_pages_pos)
	{
		A_LOGD("get a wrong position.\n");
		return 0;
	}
	sscanf(free_pages_pos, "nr_free_pages %d", &free_mem_size);

	return (free_mem_size * page_size);
}
//******************************************************************************
//
// Function Name: 
// 
// Description: 
//
// Notes:
//
//******************************************************************************
#ifdef  MMI_DEBUG
#define NUM_TABLESMax   2048
#define NUM_DSPTSMax    10240
#define hash(p,t)       (((unsigned long)(p)>>3) & (sizeof (t)/sizeof ((t)[0])-1))

static struct descriptor
{
    struct descriptor *link;
    const void *ptr;
    unsigned long size;
    const char *file;
    unsigned int line;
} *htab[NUM_TABLESMax];

static struct
{
    struct descriptor dspts[NUM_DSPTSMax];
    unsigned long idx;
} dspt_arr;

static struct descriptor* freelist = NULL;

static struct descriptor* tab_find(const void *ptr)
{
    struct descriptor *bp = htab[hash(ptr, htab)];
    while (bp && bp->ptr != ptr)
    {
        bp = bp->link;
    }
    return bp;
}

static void tab_put(struct descriptor* dspt)
{
    assert(dspt);
    struct descriptor** pp = &htab[hash(dspt->ptr, htab)];
    dspt->link = *pp;
    *pp = dspt;
}

static void tab_del(const void* ptr)
{
    assert(ptr);
    struct descriptor* bp = htab[hash(ptr, htab)];
    struct descriptor** pp;
    for (pp = &htab[hash(ptr, htab)]; *pp; pp = &(*pp)->link)
    {
        if (ptr == (*pp)->ptr)
        {
            struct descriptor* p = *pp;
            *pp = p->link;
            p->link  = freelist;
            freelist = p;
            goto EXIT;
        }
    }
    /// assert(0);

EXIT:
    return;
}

static struct descriptor* freedspt_get()
{
    struct descriptor* p = NULL;
    if (freelist != NULL)
    {
        p = freelist;
        freelist = p->link;
    }
    else
    {
        p = &dspt_arr.dspts[dspt_arr.idx++];
        if (dspt_arr.idx == NUM_DSPTSMax)
        {
            assert(0);
        }
    }
    return p;
}
#endif

void amem_info()
{
#ifdef MMI_DEBUG
    int i = 0;
    struct descriptor** pp;

    A_LOGD("---------------------------begin------------------------------\n");
    A_LOGD("mem used cnt: %d\n", dspt_arr.idx);
    A_LOGD("address    size     line     file\n");
    for (; i < NUM_TABLESMax; i++)
    for (pp = &htab[i]; *pp; pp = &(*pp)->link)
    {
        A_LOGD("0x%08x %-8d %-8d %s\n", 
                (*pp)->ptr,
                (*pp)->size,
                (*pp)->line,
                (*pp)->file);
    }
    A_LOGD("----------------------------end-------------------------------\n\n\n");

#endif
}

void* amem_alloc(size_t nbytes, const char *file, UInt32 line)
{
    void *ptr;
    /* assert(nbytes > 0); */

    ptr = malloc(nbytes == 0 ? 1 : nbytes);
    if (ptr == NULL)
    {
        A_LOGD("Allocation Failed\n");
        if (file == NULL)
        {
            A_LOGD("Allocation Failed\n");
        }
        else
        {
            A_LOGD("Allocation Failed f:%s; line:%lu \n", file, line);
        }
        abort();
    }
    else
    {
#ifdef MMI_DEBUG
        struct descriptor* p = freedspt_get();
        p->ptr  = ptr;
        p->size = (nbytes == 0) ? 1 : nbytes;
        p->file = file;
        p->line = line;
        tab_put(p);
#endif
    }
    return ptr;
}

void *amem_calloc(UInt32 count, UInt32 nbytes, const char *file, UInt32 line)
{
    void *ptr;
    assert(count > 0);
    assert(nbytes > 0);
    ptr = calloc(count, nbytes);
    if (ptr == NULL)
    {
        A_LOGD("Allocation Failed\n");
        if (file == NULL)
        {
            A_LOGD("Allocation Failed\n");
        }
        else
        {
            A_LOGD("Allocation Failed f:%s; line:%lu \n", file, line);
        }
        abort();
    }
    else
    {
#ifdef MMI_DEBUG
        struct descriptor* p = freedspt_get();
        p->ptr  = ptr;
        p->size = nbytes * count;
        p->file = file;
        p->line = line;
        tab_put(p);
#endif
    }
    return ptr;
}

void amem_free(void *ptr, const char *file, UInt32 line)
{
    if (ptr)
    {
        free(ptr);
#ifdef MMI_DEBUG
        tab_del(ptr);
#endif
    }
}

void *amem_resize(void *ptr, UInt32 nbytes, const char *file, UInt32 line)
{
    assert(ptr);
    assert(nbytes > 0);
    ptr = realloc(ptr, nbytes);
    if (ptr == NULL)
    {
        A_LOGD("Allocation Failed\n");
        if (file == NULL)
        {
            A_LOGD("Allocation Failed\n");
        }
        else
        {
            A_LOGD("Allocation Failed f:%s; line:%lu \n", file, line);
        }
        abort();
    }
    else
    {
#ifdef MMI_DEBUG
        tab_del(old);
        struct descriptor* p = freedspt_get();
        p->ptr  = ptr;
        p->size = nbytes;
        p->file = file;
        p->line = line;
        tab_put(p);
#endif
    }
    return ptr;
}


