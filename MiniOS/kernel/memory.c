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
		address <= (block->MapElem->Address + block->MapElem->Size))
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
	/*for (CurrentBlock = ListBegin(&FreeBlocks); CurrentBlock->Next != ListEnd(&FreeBlocks); CurrentBlock = CurrentBlock->Next)*/
	{
		//printf("next: %x\n", CurrentBlock->Next->MapElem->Address);
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
			//if (!((QWORD)CurrentBlock->MapElem->Address & 0xfff))
			if (IsAlligned(CurrentBlock->MapElem->Address))
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
			}
			addr = CurrentBlock->MapElem->Address + CurrentBlock->MapElem->Size;
			if (CurrentBlock->Next == ListEnd(&FreeBlocks))
			{
				break;
			}
		}
		if (IsAlligned(CurrentBlock->MapElem->Address))
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
		}
		CurrentBlock = CurrentBlock->Next;
	}
}

QWORD* MemBlockAlloc(int size)
{
	BOOLEAN allocated = FALSE;
	MEM_LIST_ELEM* CurrentMemMap;
	MEM_LIST_ELEM* AvailableBlock;
	MEM_LIST_ELEM* PrevMemMap;
	QWORD *addr = NULL;
	//CurrentMemMap = ListBegin(FreeBlocks);
	//if (CurrentMemMap->MapElem == NULL)
	//{
	//	MemMap[unusedBlockIndex].IsFree = FALSE;
	//	MemMap[unusedBlockIndex].Address = PageAlloc(roundUp(size / PAGE_SIZE));
	//	MemMap[unusedBlockIndex].Size = size;
	//	//CurrentMemMap->Prev = ListBegin(FreeBlocks)->Prev;
	//	//CurrentMemMap->Next = ListBegin(FreeBlocks);
	//	//ListBegin(FreeBlocks)->Prev->Next = CurrentMemMap;
	//	//ListBegin(FreeBlocks)->Prev = CurrentMemMap;
	//	CurrentMemMap->MapElem = MemMap + unusedBlockIndex;
	//	unusedBlockIndex++;
	//	ListInsert(ListBegin(OccupiedBlocks), CurrentMemMap);

	//	if (roundUp(size / PAGE_SIZE) * PAGE_SIZE - size > 0)
	//	{
	//		MemMap[unusedBlockIndex].IsFree = TRUE;
	//		MemMap[unusedBlockIndex].Address = CurrentMemMap->MapElem->Address + size;
	//		MemMap[unusedBlockIndex].Size = roundUp(size / PAGE_SIZE) * PAGE_SIZE - size;
	//		CurrentMemMap->MapElem = MemMap + unusedBlockIndex;
	//		unusedBlockIndex++;
	//		ListInsert(ListBegin(FreeBlocks), CurrentMemMap);
	//	}
	//}
	//while (!allocated)
	{
		/* Search for first free block. */
		if (ListEmpty(&FreeBlocks))
		{
			//printf("List is empty\n");
			//CurrentMemMap = ListBegin(&FreeBlocks);
			CurrentMemMap = GetFirstAvailable();
			CurrentMemMap->MapElem->IsFree = FALSE;
			CurrentMemMap->MapElem->Address = PageAlloc(roundUp(size / PAGE_SIZE));
			CurrentMemMap->MapElem->Size = size;
			/*AvailableBlocks[unusedBlockIndex].MapElem->IsFree = FALSE;
			AvailableBlocks[unusedBlockIndex].MapElem->Address = PageAlloc(roundUp(size / PAGE_SIZE));
			AvailableBlocks[unusedBlockIndex].MapElem->Size = size;*/
			occupiedBlocks++;
			//CurrentMemMap->Prev = ListBegin(FreeBlocks)->Prev;
			//CurrentMemMap->Next = ListBegin(FreeBlocks);
			//ListBegin(FreeBlocks)->Prev->Next = CurrentMemMap;
			//ListBegin(FreeBlocks)->Prev = CurrentMemMap;
			//printf("MemElem[%d]: %x\n", unusedBlockIndex, &AvailableBlocks[unusedBlockIndex]);
			//CurrentMemMap = &AvailableBlocks[unusedBlockIndex];
			//CurrentMemMap = AvailableBlock;
			unusedBlockIndex++;
			printf("Insert occupied %x\n", CurrentMemMap);
			//ListInsert(ListEnd(&OccupiedBlocks), CurrentMemMap);
			ListInsertOrdered(&OccupiedBlocks, CurrentMemMap);
			printf("Next: %x, Prev: %x\n", CurrentMemMap->Next, CurrentMemMap->Prev);
			addr = CurrentMemMap->MapElem->Address;
			//printf("Remain: %d\n", roundUp(size / PAGE_SIZE) * PAGE_SIZE - size);
			if (roundUp(size / PAGE_SIZE) * PAGE_SIZE - size > 0)
			{
				AvailableBlock = GetFirstAvailable();
				AvailableBlock->MapElem->IsFree = TRUE;
				AvailableBlock->MapElem->Address = CurrentMemMap->MapElem->Address + size;
				AvailableBlock->MapElem->Size = roundUp(size / PAGE_SIZE) * PAGE_SIZE - size;
				CurrentMemMap = AvailableBlock;
				//AvailableBlocks[unusedBlockIndex].MapElem->IsFree = TRUE;
				//AvailableBlocks[unusedBlockIndex].MapElem->Address = CurrentMemMap->MapElem->Address + size;
				//AvailableBlocks[unusedBlockIndex].MapElem->Size = roundUp(size / PAGE_SIZE) * PAGE_SIZE - size;
				////printf("MemElem[%d]: %x\n", unusedBlockIndex, &AvailableBlocks[unusedBlockIndex]);
				//CurrentMemMap = &AvailableBlocks[unusedBlockIndex];
				//unusedBlockIndex++;
				//printf("Unused Block index: %d\n", unusedBlockIndex);
				//printf("Head: %x, Tail: %x, Curent: %x\n", ListBegin(FreeBlocks), ListEnd(FreeBlocks), CurrentMemMap);
				printf("Insert free %x\n", CurrentMemMap);
				//ListInsert(ListEnd(&FreeBlocks), CurrentMemMap);
				ListInsertOrdered(&FreeBlocks, CurrentMemMap);
				printf("Next: %x, Prev: %x\n", CurrentMemMap->Next, CurrentMemMap->Prev);
				freeBlocks++;
			}
		}
		else
		{
			CurrentMemMap = ListBegin(&FreeBlocks);
			//printf("Begin %x\n", CurrentMemMap);
			while (CurrentMemMap != ListEnd(&FreeBlocks))
			{
				printf("Current %x\n", CurrentMemMap);
				//__magic();
				//printf("List is not empty\n");
				printf("Free: %d\n", freeBlocks);
				printf("Occupied: %d\n", occupiedBlocks);
				/*if (CurrentMemMap->MapElem == NULL)
				{
					//__magic();
					//printf("Current is NULL\n");
					AvailableBlocks[unusedBlockIndex].MapElem->IsFree = FALSE;
					AvailableBlocks[unusedBlockIndex].MapElem->Address = PageAlloc(roundUp(size / PAGE_SIZE));
					AvailableBlocks[unusedBlockIndex].MapElem->Size = size;
					//CurrentMemMap->Prev = ListBegin(FreeBlocks)->Prev;
					//CurrentMemMap->Next = ListBegin(FreeBlocks);
					//ListBegin(FreeBlocks)->Prev->Next = CurrentMemMap;
					//ListBegin(FreeBlocks)->Prev = CurrentMemMap;
					CurrentMemMap = &AvailableBlocks[unusedBlockIndex];
					unusedBlockIndex++;
					printf("Insert occupied %x\n", CurrentMemMap);
					ListInsert(ListEnd(&OccupiedBlocks), CurrentMemMap);
					printf("Next: %x, Prev: %x\n", CurrentMemMap->Next, CurrentMemMap->Prev);
					addr = CurrentMemMap->MapElem->Address;
					occupiedBlocks++;

					if (roundUp(size / PAGE_SIZE) * PAGE_SIZE - size > 0)
					{
						AvailableBlocks[unusedBlockIndex].MapElem->IsFree = TRUE;
						AvailableBlocks[unusedBlockIndex].MapElem->Address = CurrentMemMap->MapElem->Address + size;
						AvailableBlocks[unusedBlockIndex].MapElem->Size = roundUp(size / PAGE_SIZE) * PAGE_SIZE - size;
						CurrentMemMap = &AvailableBlocks[unusedBlockIndex];
						unusedBlockIndex++;
						printf("Insert free %x\n", CurrentMemMap);
						ListInsert(ListEnd(&FreeBlocks), CurrentMemMap);
						printf("Next: %x, Prev: %x\n", CurrentMemMap->Next, CurrentMemMap->Prev);
						freeBlocks++;
					}
					//break;
					return addr;
				}
				else*/ if (CurrentMemMap->MapElem->Size >= size)
				{
					//__magic();
					//printf("Size: %d >= %d\n", CurrentMemMap->MapElem->Size, size);
					QWORD* newAddr = CurrentMemMap->MapElem->Address;
					AvailableBlock = GetFirstAvailable();
					AvailableBlock->MapElem->IsFree = FALSE;
					AvailableBlock->MapElem->Address = newAddr;
					AvailableBlock->MapElem->Size = size;
					CurrentMemMap->MapElem->Address += size;
					CurrentMemMap->MapElem->Size -= size;
					CurrentMemMap = AvailableBlock;
					//AvailableBlocks[unusedBlockIndex].MapElem->IsFree = FALSE;
					//AvailableBlocks[unusedBlockIndex].MapElem->Address = newAddr;
					//AvailableBlocks[unusedBlockIndex].MapElem->Size = size;
					//CurrentMemMap->MapElem->Address += size;
					////printf("Size: %d\n", CurrentMemMap->MapElem->Size);
					//CurrentMemMap->MapElem->Size -= size;
					////printf("Size: %d\n", CurrentMemMap->MapElem->Size);
					//CurrentMemMap = &AvailableBlocks[unusedBlockIndex];
					unusedBlockIndex++;
					printf("Insert occupied %x\n", CurrentMemMap);
					//ListInsert(ListEnd(&OccupiedBlocks), CurrentMemMap);
					ListInsertOrdered(&OccupiedBlocks, CurrentMemMap);
					printf("Next: %x, Prev: %x\n", CurrentMemMap->Next, CurrentMemMap->Prev);
					addr = CurrentMemMap->MapElem->Address;
					occupiedBlocks++;
					//break;
					return addr;
				}
				//__magic();
				else
				{
					//TODO:
				}
				CurrentMemMap = CurrentMemMap->Next;
			}
			/*if (CurrentMemMap->MapElem == NULL)*/
			{
				//__magic();
				//printf("Current is NULL\n");
				CurrentMemMap = GetFirstAvailable();
				CurrentMemMap->MapElem->IsFree = FALSE;
				CurrentMemMap->MapElem->Address = PageAlloc(roundUp(size / PAGE_SIZE));
				CurrentMemMap->MapElem->Size = size;
				//AvailableBlocks[unusedBlockIndex].MapElem->IsFree = FALSE;
				//AvailableBlocks[unusedBlockIndex].MapElem->Address = PageAlloc(roundUp(size / PAGE_SIZE));
				//AvailableBlocks[unusedBlockIndex].MapElem->Size = size;
				////CurrentMemMap->Prev = ListBegin(FreeBlocks)->Prev;
				////CurrentMemMap->Next = ListBegin(FreeBlocks);
				////ListBegin(FreeBlocks)->Prev->Next = CurrentMemMap;
				////ListBegin(FreeBlocks)->Prev = CurrentMemMap;
				//CurrentMemMap = &AvailableBlocks[unusedBlockIndex];
				unusedBlockIndex++;
				printf("Insert occupied %x\n", CurrentMemMap);
				//ListInsert(ListEnd(&OccupiedBlocks), CurrentMemMap);
				ListInsertOrdered(&OccupiedBlocks, CurrentMemMap);
				printf("Next: %x, Prev: %x\n", CurrentMemMap->Next, CurrentMemMap->Prev);
				addr = CurrentMemMap->MapElem->Address;
				occupiedBlocks++;

				if (roundUp(size / PAGE_SIZE) * PAGE_SIZE - size > 0)
				{
					AvailableBlock = GetFirstAvailable();
					AvailableBlock->MapElem->IsFree = TRUE;
					AvailableBlock->MapElem->Address = CurrentMemMap->MapElem->Address + size;
					AvailableBlock->MapElem->Size = roundUp(size / PAGE_SIZE) * PAGE_SIZE - size;
					CurrentMemMap = AvailableBlock;
					//AvailableBlocks[unusedBlockIndex].MapElem->IsFree = TRUE;
					//AvailableBlocks[unusedBlockIndex].MapElem->Address = CurrentMemMap->MapElem->Address + size;
					//AvailableBlocks[unusedBlockIndex].MapElem->Size = roundUp(size / PAGE_SIZE) * PAGE_SIZE - size;
					//CurrentMemMap = &AvailableBlocks[unusedBlockIndex];
					unusedBlockIndex++;
					printf("Insert free %x\n", CurrentMemMap);
					//ListInsert(ListEnd(&FreeBlocks), CurrentMemMap);
					ListInsertOrdered(&FreeBlocks, CurrentMemMap);
					printf("Next: %x, Prev: %x\n", CurrentMemMap->Next, CurrentMemMap->Prev);
					freeBlocks++;
				}
				//break;
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
	//for (CurrentMemMap = ListBegin(&OccupiedBlocks); CurrentMemMap != ListEnd(&OccupiedBlocks); CurrentMemMap = CurrentMemMap->Next)
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