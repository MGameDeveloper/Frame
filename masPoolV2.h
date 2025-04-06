#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
    char      Tag[8];
    char     *Name;
    uint8_t  *Items;
	uint32_t  ItemSize;
	uint32_t  Capacity;
	uint32_t  MaxUsedNum;
    uint32_t  UsedNum;
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
	
	Pool->Name     = masAddrFrom(char, Pool, sizeof(masPool));
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
    if(::strcmp((*Pool)->Tag, MAS_POOL_TAG) != 0)
        return;
    printf("Pool[ %s ]: Destroyed\n", (*Pool)->Name);
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
        Header->Owner->UsedNum--;
	    ::memset(*Item, 0, Header->Owner->ItemSize);
        printf("Pool[ %s ]: Item Freed\n", Header->Owner->Name);
	}

    *Item = NULL;
}


/****************************************************************
*
*****************************************************************/
void* masPool_CopyItem(void* Item)
{
    if(!Item)
        return NULL;

    masItem* Header = masAddrFrom(masItem, Item, sizeof(masItem));
    if(::strcmp(Header->Owner->Tag, MAS_POOL_TAG) != 0)
        return NULL;
    Header->RefCount++;
    return Item;
}





/***************************************************************************************************************************
*
****************************************************************************************************************************/

struct masInputAction
{
    char      Name[32];
    void     *Func;
    uint32_t *Key;
    uint32_t  KeyCount;
    uint32_t  KeyMod;
    uint32_t  KeyState;
    bool      bAllKeyMustBeActive;
};

struct masInputAxis
{
    char      Name[32];
    void     *Func;
    uint32_t *Key;
    uint32_t  KeyCount;
    float     AxisValue;
    uint32_t  KeyMod;
    bool      bAllKeyMustBeActive;
};

struct masInputComp
{
    char            Name[32];
    masInputAction *Actions;
    masInputAxis   *Axes;
    uint32_t        ActionCount;
    uint32_t        AxisCount;
    bool            bConsumeEvent;
    bool            bBlockInput;
};

struct masInputCompStack
{
    masInputComp *Comps;
    uint32_t      PushIdx;
};

/***************************************************************************************************************************
*
****************************************************************************************************************************/

masPool *InputActionPool = masPool_Create("masInputAction", 32, sizeof(masInputAction));
masPool *InputAxisPool   = masPool_Create("masInputAxis",   32, sizeof(masInputAxis));
masPool *InputCompPool   = masPool_Create("masInputComp",   32, sizeof(masInputComp));
masInputCompStack InputCompStack[4] = { 0 }; // 4 Users Max

masInputComp* masInputComp_Create(const char* Name, bool bConsumeEvent, bool bBlockInput)
{
    masInputComp* Comp = (masInputComp*)masPool_NewItem(&InputCompPool);
    if(!Comp)
        return NULL;
    Comp->bBlockInput   = bBlockInput;
    Comp->bConsumeEvent = bConsumeEvent;
    return Comp;
}
void masInputComp_Destroy(masInputComp** InputComp)
{
    masPool_FreeItem((void**)&(*InputComp));
}

void masInputComp_AddAction(masInputComp* Comp, const char* Name, uint32_t KeyMod, uint32_t KeyState, const char* KeyListFmt, ...)
{
    
}