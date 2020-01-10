#ifndef _MEMORY_H_
#define _MEMORY_H_

#define MEMORY_MAP_ADDRESS	0x600000
#define PT_3_ADDR			0x407000

#define FRAME_SIZE		4096
#define PAGE_SIZE		4096

#define MASK_BIT_0		0x0
#define MASK_BIT_1		0x1
#define MASK_BIT_2		0x2
#define MASK_BIT_3		0x4
#define MASK_BIT_4		0x8
#define MASK_BIT_5		0x10
#define MASK_BIT_6		0x20
#define MASK_BIT_7		0x80
//#define MASK_BIT_63		0x8000000000000000

#define MAX_BLOCKS		0xff

#define U_BOOL			2

#define BITS_8			8

#include "main.h"

#pragma pack(push)
#pragma pack(1)
struct bitmap
{
	size_t	bit_cnt;	/* number of bits */
	BYTE	bits[64];		/* Elements that represents bits */
};

typedef struct _MEMORY_MAP
{
	BYTE*				Address;
	QWORD				Size;
	BOOLEAN				IsFree;
	//struct _MEMORY_MAP* Prev;
	//struct _MEMORY_MAP*	Next;
} MEMORY_MAP;

typedef struct _MEM_LIST_ELEM
{
	MEMORY_MAP*				MapElem;
	struct _MEM_LIST_ELEM*	Next;
	struct _MEM_LIST_ELEM*	Prev;
} MEM_LIST_ELEM;

typedef struct _MEM_LIST
{
	MEM_LIST_ELEM	Head;
	MEM_LIST_ELEM	Tail;
}MEM_LIST;

#pragma pack(pop)

struct bitmap BitMap;
MEMORY_MAP MemMap[MAX_BLOCKS];
MEM_LIST_ELEM AvailableBlocks[MAX_BLOCKS];
MEM_LIST FreeBlocks;
MEM_LIST OccupiedBlocks;

void InitMemory();

QWORD* GetFirstFreePage();

int GetFirstFreePageNumber();

int GetFreePagesNumberStart(int numberOfPages);

QWORD* PageAlloc(int numberOfPages);

void PageFree(QWORD* page);

QWORD* MemBlockAlloc(int size);

void MemBlockFree(QWORD* address);

#endif // _MEMEORY_H_