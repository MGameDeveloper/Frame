
#define masAddrFrom(type, ptr, offset) (type*)(((uint8_t*)ptr) + offset)
#define MAS_POOL_TAG ".masPool"

/****************************************************************
*
*****************************************************************/
struct masItem
{
    masPool *Owner;
    int32_t  RefCount;
};

struct masPool
{
    char        Tag[8];
    const char *Name;
    uint8_t    *Items;
	uint32_t    ItemSize;
	uint32_t    Capacity;
	uint32_t    MaxUsedNum;
    uint32_T    UsedNum;
};


/****************************************************************
*
*****************************************************************/
masPool* masPool_Create(const char *Name, uint32_t ItemCount, uint32_t ItemSize)
{
    uint64_t NameLen = ::strlen(Name);
	if(NameLen > 0)
	    NameLen++; // Null Terminator
		
    ItemSize          += sizeof(masItem);
	uint64_t  MemSize  = sizeof(masPool) + (ItemSize * ItemCount) + NameLen;
	masPool  *Pool     = (masPool*)::malloc(MemSize);
	if(!Pool)
	    return NULL;
	::memset(Pool, 0, MemSize);
	
	Pool->Name     = masAddrFrom(const char, Pool, sizeof(masPool));
	Pool->Items    = masAddrFrom(uint8_t, Pool->Name, NameLen);
	Pool->ItemSize = ItemSize;
	Pool->Capacity = ItemCount;
	::memcpy(Pool->Name, Name, NameLen - 1);
	::memcpy(Pool->Tag, MAS_POOL_TAG, ::strlen(MAS_POOL_TAG));

	return Pool;
}

void masPool_Destroy(masPool** Pool)
{
    if(!Pool || !(*Pool))
	    return;
	::free(*Pool);
	*Pool = NULL;
}


/****************************************************************
*
*****************************************************************/

void* masPool_NewItem(masPool** Pool)
{
    if(!Pool || !(*Pool))
	    return NULL;
	
    masPool* P = *Pool;

    if(::strcmp(P->Tag, MAS_POOL_TAG) != 0)
        return NULL;

	if(P->UsedNum >= P->Capacity)
	{
	    printf("POOL[ %s ]: ResizeRequired -> NewItem Faild\n", P->Name);
	    return NULL;
	}
	
	for(uint32_t i = 0; i < P->Capacity; ++i)
	{
	    masItem* Item = masAddrFrom(masItem, P->Items, P->ItemSize);
		if(Item->RefCount <= 0)
		{
		    P->MaxUsedNum++;
            P->UsedNum++;
		    Item->RefCount = 1;
			Item->Owner    = P;
		    return masAddrFrom(void, Item, sizeof(masItem));
		}
	}
	
	return NULL;
}
void masPool_FreeItem(void** Item)
{
    if(!Item || !(*Item))
	    return;
	
	masItem* Header = masAddrFrom(masItem, *Item, -sizeof(masItem));
    if(!Header->Owner)
        return;
    if(::strcmp(Header->Owner->Tag, MAS_POOL_TAG) != 0)
        return;

	Header->RefCount--;
	if(Header->RefCount <= 0)
	{
	    *Item = NULL;
        Header->Pool->UsedNum--;
	    ::memset(*Item, 0, Header->Pool->ItemSize);
        printf("Pool[ %s ]: Item Freed\n", Header->Owner->Name);
	}
}



void mas_test_main()
{
    masPool* InputCompPool = masPool_Create("InputComp", 16, 16);
    masInputComp* Comp = masPool_NewItem(NULL);
}