#include "memory.h"

static QWORD unusedBlockIndex = 0;
static QWORD freeBlocks = 0;
static QWORD occupiedBlocks = 0;

void InitMemory()
{
	QWORD i;
	BitMap.bit_cnt	= 64 * 8;

	for (i = 0; i < MAX_BLOCKS; i++)
	{
		MemMap[i].Address	= NULL;
		MemMap[i].Size		= 0;
		MemMap[i].IsFree	= U_BOOL;

		AvailableBlocks[i].MapElem = MemMap + i;
		AvailableBlocks[i].Next = NULL;
		AvailableBlocks[i].Prev = NULL;
		
	}
	FreeBlocks.Head.Prev		= NULL;
	FreeBlocks.Head.Next		= &FreeBlocks.Tail;
	FreeBlocks.Tail.Prev		= &FreeBlocks.Head;
	FreeBlocks.Tail.Next		= NULL;
	FreeBlocks.Head.MapElem	= NULL;

	OccupiedBlocks.Head.Prev = NULL;
	OccupiedBlocks.Head.Next = &OccupiedBlocks.Tail;
	OccupiedBlocks.Tail.Prev = &OccupiedBlocks.Head;
	OccupiedBlocks.Tail.Next = NULL;
	OccupiedBlocks.Head.MapElem = NULL;

	printf("Free Head = %x\n", &FreeBlocks.Head);
	printf("Free Tail = %x\n", &FreeBlocks.Tail);

	printf("Occ Head = %x\n", &OccupiedBlocks.Head);
	printf("Occ Tail = %x\n", &OccupiedBlocks.Tail);
}

int GetFirstFreePageNumber()
{
	int i, j;
	for (i = 0; i < BitMap.bit_cnt / BITS_8; i++)
	{
		for (j = 0; j < BITS_8; j++)
		{
			if (!(BitMap.bits[i] & (MASK_BIT_7 >> j)))
			{
				return i * BITS_8 + j;
			}
		}
	}
	return -1;

}

int GetFreePagesNumberStart(int numberOfPages)
{
	int i, j, bitmapIndex = 0, bitIndex = 0, pagesCounter = 0, err = -1;
	for (i = 0; i < BitMap.bit_cnt / BITS_8; i++)
	{
		for (j = 0; j < BITS_8; j++)
		{
			pagesCounter = 0;
			bitmapIndex = i;
			bitIndex = j;
			while (!(BitMap.bits[i] & MASK_BIT_7 >> j) && (pagesCounter < numberOfPages))
			{
				if (j < 7)
				{
					j++;
				}
				else
				{
					if (i < BitMap.bit_cnt)
					{
						i++;
						j = 0;
					}
					else
					{
						return err;		// not enought pages
					}
				}
				pagesCounter++;
			}
			if (pagesCounter == numberOfPages)
			{
				return bitmapIndex * BITS_8 + bitIndex;
			}
		}
	}
	return err;		// not enought pages
}

QWORD* GetFirstFreePage()
{
	QWORD pageNumber = GetFirstFreePageNumber();
	if (pageNumber >= 0)
	{
		return MEMORY_MAP_ADDRESS + (QWORD)(pageNumber * FRAME_SIZE);
	}
	return NULL;
}

QWORD* PageAlloc(int numberOfPages)
{
	int i;
	BYTE mask;
	if (numberOfPages == 1)
	{
		int pageNumber = GetFirstFreePageNumber();
		int bitmapIndex = pageNumber / BITS_8;
		/*QWORD mask = MASK_BIT_7 >> ((pageNumber % sizeof(BYTE)));*/
		mask = (BYTE)1U << (BITS_8 - 1 - (pageNumber % BITS_8));
		if (pageNumber >= 0)
		{
			BitMap.bits[bitmapIndex] |= mask;
			return MEMORY_MAP_ADDRESS + (QWORD)(pageNumber * FRAME_SIZE);
		}
	}
	else
	{
		int pageNumber = GetFreePagesNumberStart(numberOfPages);
		int bitmapIndex = pageNumber / BITS_8;
		int bitIndex = pageNumber % BITS_8;
		int pagesCounter = 0;
		if (pageNumber >= 0)
		{
			while ((pagesCounter < numberOfPages))
			{
				mask = (BYTE)1U << (BITS_8 - 1 - (bitIndex % BITS_8));
				BitMap.bits[bitmapIndex] |= mask;
				if (bitIndex < 7)
				{
					bitIndex++;
				}
				else
				{
					bitmapIndex++;
					bitIndex = 0;
				}
				pagesCounter++;
			}
			if (pagesCounter == numberOfPages)
			{
				return MEMORY_MAP_ADDRESS + (QWORD)(pageNumber * FRAME_SIZE);
			}
		}
	}
	return NULL;
}

//void PageFree(QWORD* page)
void PageFree(QWORD page)
{
	QWORD bitmapIndex = (QWORD)page / BITS_8;
	BitMap.bits[bitmapIndex] &= ~(1U << 7 - (((QWORD)page % BITS_8)));
}

MEM_LIST_ELEM* ListHead(MEM_LIST *list)
{
	return &list->Head;
}

MEM_LIST_ELEM* ListTail(MEM_LIST* list)
{
	return &list->Tail;
}

MEM_LIST_ELEM* ListBegin(MEM_LIST* list)
{
	return list->Head.Next;
}

MEM_LIST_ELEM* ListEnd(MEM_LIST* list)
{
	return &list->Tail;
}

MEM_LIST_ELEM* GetFirstAvailable()
{
	int i;
	for (i = 0; i < MAX_BLOCKS; i++)
	{
		if (U_BOOL == AvailableBlocks[i].MapElem->IsFree)
		{
			return &AvailableBlocks[i].MapElem;
		}
	}
	return NULL;
}

void PrintMemMap(MEMORY_MAP map)
{
	printf("(a:%x,%d)", map.Address, map.Size, map.IsFree);
}

void PrintList(MEM_LIST *list)
{
	MEM_LIST_ELEM* e = ListBegin(list);
	while (e != ListEnd(list))
	{
		PrintMemMap(*e->MapElem);
		printf("->");
		e = e->Next;
	}
}

void ListInsert(MEM_LIST_ELEM* before, MEM_LIST_ELEM* elem)
{
	elem->Prev = before->Prev;
	elem->Next = before;
	before->Prev->Next = elem;
	before->Prev = elem;
}

void ListInsertOrdered(MEM_LIST* list, MEM_LIST_ELEM* elem)
{
	MEM_LIST_ELEM *e;

	for (e = ListBegin(list); e != ListEnd(list); e = e->Next)
	{
		if (elem->MapElem->Address < e->MapElem->Address)
		{
			break;
		}
	}
	ListInsert(e, elem);
}

void ListRemove(MEM_LIST_ELEM *elem)
{
	elem->Prev->Next = elem->Next;
	elem->Next->Prev = elem->Prev;
}

BOOLEAN ListEmpty(MEM_LIST *list)
{
	return ListBegin(list) == ListEnd(list);
}

BOOLEAN AddressInsideBlock(QWORD address, MEM_LIST_ELEM* block)
{
	if (address >= block->MapElem->Address&&
		address < (block->MapElem->Address + block->MapElem->Size))
	{
		return TRUE;
	}
	return FALSE;
}

BOOLEAN IsAlligned(int number)
{
	if (number % 0x1000 == 0)
	{
		return TRUE;
	}
	return FALSE;
}

void CompactFreeBlocks()
{
	MEM_LIST_ELEM *CurrentBlock;
	MEM_LIST_ELEM* FreeTail;
	MEM_LIST_ELEM *DisposableBlock;
	QWORD page;
	QWORD* addr;
	
	FreeTail = ListEnd(&FreeBlocks);
	CurrentBlock = ListBegin(&FreeBlocks);
	while (CurrentBlock != FreeTail)
	{
		addr = CurrentBlock->MapElem->Address + CurrentBlock->MapElem->Size;
		while ((CurrentBlock->Next != ListEnd(&FreeBlocks)) &&
				(addr == CurrentBlock->Next->MapElem->Address) &&
				(!IsAlligned(CurrentBlock->Next->MapElem->Address)))
		{
			CurrentBlock->MapElem->Size += CurrentBlock->Next->MapElem->Size;
			DisposableBlock = CurrentBlock->Next;
			ListRemove(DisposableBlock);
			DisposableBlock->MapElem->Address	= NULL;
			DisposableBlock->MapElem->IsFree = U_BOOL;
			DisposableBlock->MapElem->Size = 0;
			addr = CurrentBlock->MapElem->Address + CurrentBlock->MapElem->Size;
			if (CurrentBlock->Next == ListEnd(&FreeBlocks))
			{
				break;
			}
		}
		CurrentBlock = CurrentBlock->Next;
	}

	CurrentBlock = ListBegin(&FreeBlocks);
	while (CurrentBlock != FreeTail)
	{
		while (IsAlligned(CurrentBlock->MapElem->Address))
		{
			if ((QWORD)PAGE_SIZE == (QWORD)CurrentBlock->MapElem->Size)
			{
				page = ((QWORD)CurrentBlock->MapElem->Address - (QWORD)MEMORY_MAP_ADDRESS) >> 12;
				PageFree(page);
				DisposableBlock = CurrentBlock;
				CurrentBlock = CurrentBlock->Next;
				ListRemove(DisposableBlock);
				DisposableBlock->MapElem->Address = NULL;
				DisposableBlock->MapElem->IsFree = U_BOOL;
				DisposableBlock->MapElem->Size = 0;
			}
			else if ((QWORD)PAGE_SIZE < (QWORD)CurrentBlock->MapElem->Size)
			{
				page = ((QWORD)CurrentBlock->MapElem->Address - (QWORD)MEMORY_MAP_ADDRESS) >> 12;
				PageFree(page);
				CurrentBlock->MapElem->Address += PAGE_SIZE;
				CurrentBlock->MapElem->Size -= PAGE_SIZE;
			}
			else
			{
				CurrentBlock = CurrentBlock->Next;
			}
			if (CurrentBlock == FreeTail)
			{
				return;
			}
		}
		CurrentBlock = CurrentBlock->Next;
	}
}

QWORD* MemBlockAlloc(int size)
{
	MEM_LIST_ELEM* CurrentMemMap;
	MEM_LIST_ELEM* AvailableBlock;
	MEM_LIST_ELEM* PrevMemMap;
	QWORD *addr = NULL;
	{
		/* Search for first free block. */
		if (ListEmpty(&FreeBlocks))
		{
			CurrentMemMap = GetFirstAvailable();
			CurrentMemMap->MapElem->IsFree = FALSE;
			CurrentMemMap->MapElem->Address = PageAlloc(roundUp((double)size / (double)PAGE_SIZE));
			CurrentMemMap->MapElem->Size = size;
			occupiedBlocks++;
			unusedBlockIndex++;
			ListInsertOrdered(&OccupiedBlocks, CurrentMemMap);
			addr = CurrentMemMap->MapElem->Address;
			if (roundUp((double)size / (double)PAGE_SIZE) * PAGE_SIZE - size > 0)
			{
				AvailableBlock = GetFirstAvailable();
				AvailableBlock->MapElem->IsFree = TRUE;
				AvailableBlock->MapElem->Address = CurrentMemMap->MapElem->Address + size;
				AvailableBlock->MapElem->Size = roundUp((double)size / (double)PAGE_SIZE) * PAGE_SIZE - size;
				CurrentMemMap = AvailableBlock;
				ListInsertOrdered(&FreeBlocks, CurrentMemMap);
				freeBlocks++;
			}
		}
		else
		{
			CurrentMemMap = ListBegin(&FreeBlocks);
			while (CurrentMemMap != ListEnd(&FreeBlocks))
			{
				if (CurrentMemMap->MapElem->Size >= size)
				{
					QWORD* newAddr = CurrentMemMap->MapElem->Address;
					AvailableBlock = GetFirstAvailable();
					AvailableBlock->MapElem->IsFree = FALSE;
					AvailableBlock->MapElem->Address = newAddr;
					AvailableBlock->MapElem->Size = size;
					CurrentMemMap->MapElem->Address += size;
					CurrentMemMap->MapElem->Size -= size;
					CurrentMemMap = AvailableBlock;
					unusedBlockIndex++;
					ListInsertOrdered(&OccupiedBlocks, CurrentMemMap);
					addr = CurrentMemMap->MapElem->Address;
					occupiedBlocks++;
					return addr;
				}
				CurrentMemMap = CurrentMemMap->Next;
			}
			{
				CurrentMemMap = GetFirstAvailable();
				CurrentMemMap->MapElem->IsFree = FALSE;
				CurrentMemMap->MapElem->Address = PageAlloc(roundUp((double)size / (double)PAGE_SIZE));
				CurrentMemMap->MapElem->Size = size;
				unusedBlockIndex++;
				ListInsertOrdered(&OccupiedBlocks, CurrentMemMap);
				addr = CurrentMemMap->MapElem->Address;
				occupiedBlocks++;

				if (roundUp((double)size / (double)PAGE_SIZE) * PAGE_SIZE - size > 0)
				{
					AvailableBlock = GetFirstAvailable();
					AvailableBlock->MapElem->IsFree = TRUE;
					AvailableBlock->MapElem->Address = CurrentMemMap->MapElem->Address + size;
					AvailableBlock->MapElem->Size = roundUp((double)size / (double)PAGE_SIZE) * PAGE_SIZE - size;
					CurrentMemMap = AvailableBlock;
					unusedBlockIndex++;
					ListInsertOrdered(&FreeBlocks, CurrentMemMap);
					freeBlocks++;
				}
				return addr;
			}
		}
		
	}
	return addr;
}

void MemBlockFree(QWORD* address)
{
	MEM_LIST_ELEM* CurrentMemMap;
	MEM_LIST_ELEM* DisposableBlock;
	MEM_LIST_ELEM* OccTail;

	MEM_LIST_ELEM* e;

	OccTail = ListEnd(&OccupiedBlocks);

	if (ListEmpty(&OccupiedBlocks))
	{
		printf("ERROR: there are no occupied blocks.\n");
		return;
	}

	CurrentMemMap = ListBegin(&OccupiedBlocks);
	while (CurrentMemMap != OccTail)
	{
		if (AddressInsideBlock(address, CurrentMemMap))
		{
			DisposableBlock = CurrentMemMap;
			CurrentMemMap = CurrentMemMap->Next;
			ListRemove(DisposableBlock);
			DisposableBlock->MapElem->IsFree = TRUE;
			ListInsertOrdered(&FreeBlocks, DisposableBlock);
			CompactFreeBlocks();
			continue;
		}
		CurrentMemMap = CurrentMemMap->Next;
	}
}