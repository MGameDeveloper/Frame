#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

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
struct masInputKey
{
    masInputKey *Next;
    masInputKey *Prev;
    int32_t      Key;
};

struct masInputAction
{
    masInputAction* Next;
    masInputAction* Prev;

    char         Name[32];
    void        *Func;
    masInputKey *KeyList;
    uint32_t     KeyMod;
    uint32_t     KeyState;
    bool         bAllKeyMustBeActive;
};

struct masInputAxis
{
    masInputAxis *Next;
    masInputAxis *Prev;

    char         Name[32];
    void        *Func;
    masInputKey *KeyList;
    float        AxisValue;
    uint32_t     KeyMod;
    bool         bAllKeyMustBeActive;
};

struct masInputComp
{
    masInputComp   *Next;
    masInputComp   *Prev;

    char            Name[32];
    masInputAction *Actions;
    masInputAxis   *Axes;
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
masPool *InputKeyPool    = masPool_Create("masInputKey",    5000, sizeof(masInputKey));
masPool *InputActionPool = masPool_Create("masInputAction", 32,   sizeof(masInputAction));
masPool *InputAxisPool   = masPool_Create("masInputAxis",   32,   sizeof(masInputAxis));
masPool *InputCompPool   = masPool_Create("masInputComp",   32,   sizeof(masInputComp));
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
    // todo: need revision
    // validate InputComp

    masInputComp* Comp = *InputComp;
    
    for ( ; Comp->Actions ; )
    {
        masInputAction* Temp = Comp->Actions;
        Comp->Actions = Temp->Next;
        for (; Temp->KeyList; )
        {
            masInputKey *TempKey = Temp->KeyList;
            Temp->KeyList        = TempKey->Next;
            masPool_FreeItem((void**)&TempKey);
        }
        masPool_FreeItem((void**)&Temp);
    }

    for ( ; Comp->Axes ; )
    {
        masInputAxis *Temp = Comp->Axes;
        Comp->Axes         = Temp->Next;
        for (; Temp->KeyList; )
        {
            masInputKey   *TempKey = Temp->KeyList;
            Temp->KeyList          = TempKey->Next;
            masPool_FreeItem((void**)&TempKey);
        }
        masPool_FreeItem((void**)&Temp);
    }


    masPool_FreeItem((void**)&(*InputComp));
}


#define MAS_KEY_LIST(...) #__VA_ARGS__##"\n", __VA_ARGS__

void masInputComp_AddAction(masInputComp* Comp, const char* Name, void* ActionFunc, uint32_t KeyMod, uint32_t KeyState, bool bAllMustBeActive, const char* KeyListFmt, ...)
{
    uint32_t KeyListFmtLen = (uint32_t)::strlen(KeyListFmt);
    if (KeyListFmtLen == 0)
        return;
    if (KeyListFmtLen == 1 && KeyListFmt[0] == '\n')
        return;
    uint32_t KeyCount = 0;
    for (uint32_t i = 0; i < KeyListFmtLen; ++i)
        if (KeyListFmt[i] == ',' || KeyListFmt[i] == '\n')
            KeyCount++;
    
    masInputAction* Action = (masInputAction*)masPool_NewItem(&InputActionPool);
    if (!Action)
        return;
    ::memcpy(Action->Name, Name, ::strlen(Name));
    Action->Func                = ActionFunc;
    Action->KeyMod              = KeyMod;
    Action->KeyState            = KeyState;
    Action->bAllKeyMustBeActive = bAllMustBeActive;  

    va_list Args;
    va_start(Args, KeyCount);
    for (uint32_t KeyIdx = 0; KeyIdx < KeyCount; ++KeyIdx)
    {
        // validate keys value
        masInputKey* Key = (masInputKey*)masPool_NewItem(&InputKeyPool);
        if (!Key)
        {
            masPool_FreeItem((void**)&Action);
            return;
        }

        if (!Action->KeyList)
            Action->KeyList = Key;
        else
        {
            Key->Next             = Action->KeyList;
            Action->KeyList->Prev = Key;
            Action->KeyList       = Key;
        }
    }
    va_end(Args);

    if (!Comp->Actions)
        Comp->Actions = Action;
    else
    {
        Action->Next        = Comp->Actions;
        Comp->Actions->Prev = Action;
        Comp->Actions       = Action;
    }
}

void masInputComp_AddAxis(masInputComp* Comp, const char* Name, void* AxisFunc, float AxisValue, uint32_t KeyMod, bool bAllMustBeActive, const char* KeyListFmt, ...)
{
    uint32_t KeyListFmtLen = (uint32_t)::strlen(KeyListFmt);
    if (KeyListFmtLen == 0)
        return;
    if (KeyListFmtLen == 1 && KeyListFmt[0] == '\n')
        return;
    uint32_t KeyCount = 0;
    for (uint32_t i = 0; i < KeyListFmtLen; ++i)
        if (KeyListFmt[i] == ',' || KeyListFmt[i] == '\n')
            KeyCount++;

    masInputAxis* Axis = (masInputAxis*)masPool_NewItem(&InputAxisPool);
    if (!Axis)
        return;
    ::memcpy(Axis->Name, Name, ::strlen(Name));
    Axis->Func                = AxisFunc;
    Axis->KeyMod              = KeyMod;
    Axis->AxisValue           = AxisValue;
    Axis->bAllKeyMustBeActive = bAllMustBeActive;

    va_list Args;
    va_start(Args, KeyCount);
    for (uint32_t KeyIdx = 0; KeyIdx < KeyCount; ++KeyIdx)
    {
        // validate keys value
        masInputKey* Key = (masInputKey*)masPool_NewItem(&InputKeyPool);
        if (!Key)
        {
            masPool_FreeItem((void**)&Axis);
            return;
        }

        if (!Axis->KeyList)
            Axis->KeyList = Key;
        else
        {
            Key->Next = Axis->KeyList;
            Axis->KeyList->Prev = Key;
            Axis->KeyList = Key;
        }
    }
    va_end(Args);

    if (!Comp->Axes)
        Comp->Axes = Axis;
    else
    {
        Axis->Next       = Comp->Axes;
        Comp->Axes->Prev = Axis;
        Comp->Axes       = Axis;
    }
}

void masInputCompStack_Push(uint8_t EInputUser, masInputComp* InputComp)
{
    // validate EInputUser & InputComp

    masInputCompStack* UserStack = &InputCompStack[EInputUser];
    
    if (!UserStack->Comps)
        UserStack->Comps = InputComp;
    else
    {
        InputComp->Next        = UserStack->Comps;
        UserStack->Comps->Prev = InputComp;
        UserStack->Comps       = InputComp;
    }
}

void masInputCompStack_Pop(uint8_t EInputUser)
{
    // validate EInptUser

    masInputCompStack* UserStack = &InputCompStack[EInputUser];

    masInputComp* Comp = UserStack->Comps;
    if (!Comp)
        return;

    if (Comp->Next)
        UserStack->Comps = Comp->Next;
    else
        UserStack->Comps = NULL;

    masInputComp_Destroy(&Comp);
}