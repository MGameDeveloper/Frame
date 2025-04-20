#include "masInputComp.h"
#include "masInputData.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define masAddrFrom(type, ptr, offset) (type*)(((uint8_t*)ptr) + offset)
#define MAS_POOL_TAG ".masPool"


/****************************************************************
*
*****************************************************************/
static uint32_t masArgCount(const char* ArgFmt)
{
    uint32_t KeyListFmtLen = (uint32_t)::strlen(ArgFmt);
    if (KeyListFmtLen == 0)
        return 0;

    if (KeyListFmtLen == 1 && ArgFmt[0] == '\n')
        return 0;

    uint32_t Count = 0;
    for (uint32_t i = 0; i < KeyListFmtLen; ++i)
        if (ArgFmt[i] == ',' || ArgFmt[i] == '\n')
            Count++;

    return Count;
}




/****************************************************************
*
*****************************************************************/
struct masPool
{
    char      Tag[9];
    char     *Name;
    uint8_t  *Items;
    uint32_t  ItemSize;
    uint32_t  Capacity;
    uint32_t  MaxUsedNum;
    uint32_t  UsedNum;
};

struct masItem
{
    masPool* Owner;
    int32_t  RefCount;
};




/****************************************************************
*
*****************************************************************/
masPool* masPool_Create(const char* Name, uint32_t ItemCount, uint32_t ItemSize)
{
    uint64_t NameLen = ::strlen(Name);
    if (NameLen > 0)
        NameLen++; // Null Terminator

    ItemSize += sizeof(masItem);
    uint64_t  MemSize = sizeof(masPool) + (ItemSize * ItemCount) + NameLen;
    masPool* Pool = (masPool*)::malloc(MemSize);
    if (!Pool)
        return NULL;
    ::memset(Pool, 0, MemSize);

    Pool->Name = masAddrFrom(char, Pool, sizeof(masPool));
    Pool->Items = masAddrFrom(uint8_t, Pool->Name, NameLen);
    Pool->ItemSize = ItemSize;
    Pool->Capacity = ItemCount;
    ::memcpy(Pool->Name, Name, NameLen - 1);
    ::memcpy(Pool->Tag, MAS_POOL_TAG, ::strlen(MAS_POOL_TAG));

    return Pool;
}
void masPool_Destroy(masPool** Pool)
{
    if (!Pool || !(*Pool))
        return;
    if (::strcmp((*Pool)->Tag, MAS_POOL_TAG) != 0)
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
    if (!Pool || !(*Pool))
        return NULL;

    masPool* P = *Pool;

    if (::strcmp(P->Tag, MAS_POOL_TAG) != 0)
        return NULL;

    if (P->UsedNum >= P->Capacity)
    {
        printf("POOL[ %s ]: ResizeRequired -> NewItem Faild\n", P->Name);
        return NULL;
    }

    for (uint32_t i = 0; i < P->Capacity; ++i)
    {
        masItem* Item = masAddrFrom(masItem, P->Items, P->ItemSize * i);
        if (Item->RefCount <= 0)
        {
            P->MaxUsedNum++;
            P->UsedNum++;
            Item->RefCount = 1;
            Item->Owner = P;
            return masAddrFrom(void, Item, sizeof(masItem));
        }
    }

    return NULL;
}
void masPool_FreeItem(void** Item)
{
    if (!Item || !(*Item))
        return;

    int64_t  HeaderOffset = (int64_t)sizeof(masItem);
    masItem *Header       = masAddrFrom(masItem, *Item, -HeaderOffset);
    if (!Header->Owner)
        return;
    if (::strcmp(Header->Owner->Tag, MAS_POOL_TAG) != 0)
        return;

    Header->RefCount--;
    if (Header->RefCount <= 0)
    {
        Header->Owner->UsedNum--;
        ::memset(*Item, 0, Header->Owner->ItemSize);
        printf("Pool[ %s ]: Item Freed\n", Header->Owner->Name);
    }

    *Item = NULL;
}




/***************************************************************************************************************************
*
****************************************************************************************************************************/
struct masInputKey
{
    masInputKey* Next;
    masInputKey* Prev;
    masEKey      Code;
};

struct masInputAction
{
    masInputAction* Next;
    masInputAction* Prev;

    char                Name[32];
    masInputActionFunc  Func;
    masInputKey        *KeyList;
    uint16_t            KeyMod;
    uint8_t             KeyState;
};

struct masInputAxis
{
    masInputAxis* Next;
    masInputAxis* Prev;

    char              Name[32];
    masInputAxisFunc  Func;
    masInputKey      *KeyList;
    float             AxisValue;
    uint16_t          KeyMod;
};

struct masInputComp
{
    masInputComp* Next;
    masInputComp* Prev;

    char            Name[32];
    masInputAction *Actions;
    masInputAxis   *Axes;
    bool            bConsumeEvent;
    bool            bBlockInput;
};

struct masInputCompStack
{
    masInputComp* Comps;
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
    if (!Comp)
        return NULL;
    Comp->bBlockInput = bBlockInput;
    Comp->bConsumeEvent = bConsumeEvent;

    uint32_t NameLen = ::strlen(Name);
    if (NameLen > 32)
        NameLen = 32;
    ::memcpy(Comp->Name, Name, NameLen);

    return Comp;
}

void masInputComp_Destroy(masInputComp** InputComp)
{
    // todo: need revision
    // validate InputComp

    masInputComp* Comp = *InputComp;

    for (; Comp->Actions; )
    {
        masInputAction* Temp = Comp->Actions;
        Comp->Actions = Temp->Next;
        for (; Temp->KeyList; )
        {
            masInputKey* TempKey = Temp->KeyList;
            Temp->KeyList = TempKey->Next;
            masPool_FreeItem((void**)&TempKey);
        }
        masPool_FreeItem((void**)&Temp);
    }

    for (; Comp->Axes; )
    {
        masInputAxis* Temp = Comp->Axes;
        Comp->Axes = Temp->Next;
        for (; Temp->KeyList; )
        {
            masInputKey* TempKey = Temp->KeyList;
            Temp->KeyList = TempKey->Next;
            masPool_FreeItem((void**)&TempKey);
        }
        masPool_FreeItem((void**)&Temp);
    }


    masPool_FreeItem((void**)&(*InputComp));
}


/***************************************************************************************************************************
*
****************************************************************************************************************************/
void masInputComp_AddAction(masInputComp* Comp, const char* Name, masInputActionFunc ActionFunc, EKeyMod_ KeyMod, EKeyState_ KeyState, const char* KeyListFmt, ...)
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

    uint32_t NameLen = ::strlen(Name);
    if (NameLen > 32)
        NameLen = 32;
    ::memcpy(Action->Name, Name, NameLen);

    Action->Func = ActionFunc;
    Action->KeyMod = KeyMod;
    Action->KeyState = KeyState;

    va_list Args;
    va_start(Args, KeyCount);
    for (uint32_t KeyIdx = 0; KeyIdx < KeyCount; ++KeyIdx)
    {
        masEKey KeyCode = va_arg(Args, masEKey);
        if (KeyCode == EKey_Unknown)
            continue;

        masInputKey* Key = (masInputKey*)masPool_NewItem(&InputKeyPool);
        if (!Key)
        {
            masPool_FreeItem((void**)&Action);
            return;
        }

        Key->Code = KeyCode;

        if (!Action->KeyList)
            Action->KeyList = Key;
        else
        {
            Key->Next = Action->KeyList;
            Action->KeyList->Prev = Key;
            Action->KeyList = Key;
        }
    }
    va_end(Args);

    if (!Comp->Actions)
        Comp->Actions = Action;
    else
    {
        Action->Next = Comp->Actions;
        Comp->Actions->Prev = Action;
        Comp->Actions = Action;
    }
}

void masInputComp_AddAxis(masInputComp* Comp, const char* Name, masInputAxisFunc AxisFunc, float AxisValue, EKeyMod_ KeyMod, const char* KeyListFmt, ...)
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

    uint32_t NameLen = ::strlen(Name);
    if (NameLen > 32)
        NameLen = 32;
    ::memcpy(Axis->Name, Name, NameLen);

    Axis->Func = AxisFunc;
    Axis->KeyMod = KeyMod;
    Axis->AxisValue = AxisValue;

    va_list Args;
    va_start(Args, KeyCount);
    for (uint32_t KeyIdx = 0; KeyIdx < KeyCount; ++KeyIdx)
    {
        masEKey KeyCode = va_arg(Args, masEKey);
        if (KeyCode == EKey_Unknown)
            continue;

        masInputKey* Key = (masInputKey*)masPool_NewItem(&InputKeyPool);
        if (!Key)
        {
            masPool_FreeItem((void**)&Axis);
            return;
        }

        Key->Code = KeyCode;

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
        Axis->Next = Comp->Axes;
        Comp->Axes->Prev = Axis;
        Comp->Axes = Axis;
    }
}


/***************************************************************************************************************************
*
****************************************************************************************************************************/
void masInputComp_Push(masEInputUser EInputUser, masInputComp* InputComp)
{
    // validate EInputUser & InputComp

    masInputCompStack* UserStack = &InputCompStack[EInputUser];

    if (!UserStack->Comps)
        UserStack->Comps = InputComp;
    else
    {
        InputComp->Next = UserStack->Comps;
        UserStack->Comps->Prev = InputComp;
        UserStack->Comps = InputComp;
    }
}

void masInputComp_Pop(masEInputUser EInputUser)
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




/***************************************************************************************************************************
*
****************************************************************************************************************************/
static void masInputCompInternal_ProcessActions(masEInputUser InputUser, masInputComp* InputComp)
{
    masInputAction* Action = InputComp->Actions;
    for (; Action; Action = Action->Next)
    {
        if (masInputData_IsKeyModActive(Action->KeyMod))
        {
            masInputKey* Key = Action->KeyList;
            for (; Key; Key = Key->Next)
            {
                masEKeyState KeyState = masInputData_GetKeyState(InputUser, Key->Code);
                if (KeyState == Action->KeyState)
                {
                    if (Action->Func)
                        Action->Func(); // we could build an array of all actions and execute them at the end

                    if (InputComp->bConsumeEvent)
                        masInputData_SetKeyState(InputUser, Key->Code, EKeyState_None);
                }
            }
        }
    }
}
static void masInputCompInternal_ProcessAxes(masEInputUser InputUser, masInputComp* InputComp)
{
    masInputAxis* Axis = InputComp->Axes;
    for (; Axis; Axis = Axis->Next)
    {
        masInputKey* Key = Axis->KeyList;
        for (; Key; Key = Key->Next)
        {
            if (masInputData_IsKeyModActive(Axis->KeyMod))
            {
                if (masInputData_IsKeyActive(InputUser, Key->Code))
                {
                    float AxisValue = masInputData_GetUserAxisValue(InputUser, Key->Code);
                    if (AxisValue == 0.0f)
                        AxisValue = Axis->AxisValue;

                    if (Axis->Func)
                        Axis->Func(AxisValue); // axis value could be L2, R2, LAnalog, RAnalog or constent scaler specified by user
                }
            }
        }
    }
}


/***************************************************************************************************************************
*
****************************************************************************************************************************/
void masInputComp_Update()
{
    for (int32_t UserIdx = 0; UserIdx < 4; ++UserIdx)
    {
        masEInputUser      InputUser = (masEInputUser)UserIdx;
        masInputCompStack *Stack     = &InputCompStack[UserIdx];


        masInputComp *Comp = Stack->Comps;
        for (; Comp; Comp = Comp->Next)
        {
            masInputCompInternal_ProcessActions(InputUser, Comp);
            masInputCompInternal_ProcessAxes(InputUser, Comp);

            if (Comp->bBlockInput)
                break;
        }
    }
}




/***************************************************************************************************************************
*
****************************************************************************************************************************/
bool masInput_OnKeyEvent(masEInputUser InputUser, EKeyMod_ KeyMod, EKeyState_ KeyState, const char* KeyListFmt, ...)
{
    if (!masInputData_IsKeyModActive(KeyMod))
        return false;

    uint32_t ArgCount = masArgCount(KeyListFmt);
    if (ArgCount == 0)
        return false;

    va_list Args;
    va_start(Args, ArgCount);
    for (uint32_t ArgIdx = 0; ArgIdx < ArgCount; ++ArgIdx)
    {
        masEKey KeyCode = va_arg(Args, masEKey);
        if (KeyCode == EKey_Unknown)
            continue;

        masEKeyState State = masInputData_GetKeyState(InputUser, KeyCode);
        if (State == KeyState)
        {
            va_end(Args);
            return true;
        }
    }
    va_end(Args);

    return false;
}

bool masInput_OnKeyAxis(masEInputUser InputUser, EKeyMod_ KeyMod, const char* KeyListFmt, ...)
{
    if (!masInputData_IsKeyModActive(KeyMod))
        return false;

    uint32_t ArgCount = masArgCount(KeyListFmt);
    if (ArgCount == 0)
        return false;

    va_list Args;
    va_start(Args, ArgCount);
    for (uint32_t ArgIdx = 0; ArgIdx < ArgCount; ++ArgIdx)
    {
        masEKey KeyCode = va_arg(Args, masEKey);
        if (KeyCode == EKey_Unknown)
            continue;

        if (masInputData_IsKeyActive(InputUser, KeyCode))
        {
            va_end(Args);
            return true;
        }
    }
    va_end(Args);

    return false;
}




/********************************************************************
*
*********************************************************************/
const char* masInput_KeyName(masEKey Key)
{

#define MAS_KEY_NAME(KEY) case KEY: return #KEY##"\0"
    switch (Key)
    {
        MAS_KEY_NAME(EKey_Square);
        MAS_KEY_NAME(EKey_Cross);
        MAS_KEY_NAME(EKey_Circle);
        MAS_KEY_NAME(EKey_Triangle);
        MAS_KEY_NAME(EKey_Start);
        MAS_KEY_NAME(EKey_Select);
        MAS_KEY_NAME(EKey_DpadUp);
        MAS_KEY_NAME(EKey_DpadDown);
        MAS_KEY_NAME(EKey_DpadRight);
        MAS_KEY_NAME(EKey_DpadLeft);
        MAS_KEY_NAME(EKey_L1);
        MAS_KEY_NAME(EKey_L2);
        MAS_KEY_NAME(EKey_L3);
        MAS_KEY_NAME(EKey_R1);
        MAS_KEY_NAME(EKey_R2);
        MAS_KEY_NAME(EKey_R3);
        MAS_KEY_NAME(EKey_LAnalogUp);
        MAS_KEY_NAME(EKey_LAnalogDown);
        MAS_KEY_NAME(EKey_LAnalogLeft);
        MAS_KEY_NAME(EKey_LAnalogRight);
        MAS_KEY_NAME(EKey_RAnalogUp);
        MAS_KEY_NAME(EKey_RAnalogDown);
        MAS_KEY_NAME(EKey_RAnalogLeft);
        MAS_KEY_NAME(EKey_RAnalogRight);
        MAS_KEY_NAME(EKey_Anykey);
        MAS_KEY_NAME(EKey_A);
        MAS_KEY_NAME(EKey_B);
        MAS_KEY_NAME(EKey_C);
        MAS_KEY_NAME(EKey_D);
        MAS_KEY_NAME(EKey_E);
        MAS_KEY_NAME(EKey_F);
        MAS_KEY_NAME(EKey_G);
        MAS_KEY_NAME(EKey_H);
        MAS_KEY_NAME(EKey_I);
        MAS_KEY_NAME(EKey_J);
        MAS_KEY_NAME(EKey_K);
        MAS_KEY_NAME(EKey_L);
        MAS_KEY_NAME(EKey_M);
        MAS_KEY_NAME(EKey_N);
        MAS_KEY_NAME(EKey_O);
        MAS_KEY_NAME(EKey_P);
        MAS_KEY_NAME(EKey_Q);
        MAS_KEY_NAME(EKey_R);
        MAS_KEY_NAME(EKey_S);
        MAS_KEY_NAME(EKey_T);
        MAS_KEY_NAME(EKey_U);
        MAS_KEY_NAME(EKey_V);
        MAS_KEY_NAME(EKey_W);
        MAS_KEY_NAME(EKey_X);
        MAS_KEY_NAME(EKey_Y);
        MAS_KEY_NAME(EKey_Z);
        MAS_KEY_NAME(EKey_F1);
        MAS_KEY_NAME(EKey_F2);
        MAS_KEY_NAME(EKey_F3);
        MAS_KEY_NAME(EKey_F4);
        MAS_KEY_NAME(EKey_F5);
        MAS_KEY_NAME(EKey_F6);
        MAS_KEY_NAME(EKey_F7);
        MAS_KEY_NAME(EKey_F8);
        MAS_KEY_NAME(EKey_F9);
        MAS_KEY_NAME(EKey_F10);
        MAS_KEY_NAME(EKey_F11);
        MAS_KEY_NAME(EKey_F12);
        MAS_KEY_NAME(EKey_NumLock);
        MAS_KEY_NAME(EKey_Numpad0);
        MAS_KEY_NAME(EKey_Numpad1);
        MAS_KEY_NAME(EKey_Numpad2);
        MAS_KEY_NAME(EKey_Numpad3);
        MAS_KEY_NAME(EKey_Numpad4);
        MAS_KEY_NAME(EKey_Numpad5);
        MAS_KEY_NAME(EKey_Numpad6);
        MAS_KEY_NAME(EKey_Numpad7);
        MAS_KEY_NAME(EKey_Numpad8);
        MAS_KEY_NAME(EKey_Numpad9);
        MAS_KEY_NAME(EKey_Num0);
        MAS_KEY_NAME(EKey_Num1);
        MAS_KEY_NAME(EKey_Num2);
        MAS_KEY_NAME(EKey_Num3);
        MAS_KEY_NAME(EKey_Num4);
        MAS_KEY_NAME(EKey_Num5);
        MAS_KEY_NAME(EKey_Num6);
        MAS_KEY_NAME(EKey_Num7);
        MAS_KEY_NAME(EKey_Num8);
        MAS_KEY_NAME(EKey_Num9);
        MAS_KEY_NAME(EKey_Decimal);
        MAS_KEY_NAME(EKey_PageUp);
        MAS_KEY_NAME(EKey_PageDown);
        MAS_KEY_NAME(EKey_Space);
        MAS_KEY_NAME(EKey_Enter);
        MAS_KEY_NAME(EKey_Backspace);
        MAS_KEY_NAME(EKey_Tab);
        MAS_KEY_NAME(EKey_PrintScreen);
        MAS_KEY_NAME(EKey_Insert);
        MAS_KEY_NAME(EKey_Delete);
        MAS_KEY_NAME(EKey_Divide);
        MAS_KEY_NAME(EKey_Multipy);
        MAS_KEY_NAME(EKey_Subtract);
        MAS_KEY_NAME(EKey_Addition);
        MAS_KEY_NAME(EKey_Home);
        MAS_KEY_NAME(EKey_End);
        MAS_KEY_NAME(EKey_Escape);
        MAS_KEY_NAME(EKey_CapsLock);
        MAS_KEY_NAME(EKey_ArrowUp);
        MAS_KEY_NAME(EKey_ArrowDown);
        MAS_KEY_NAME(EKey_ArrowLeft);
        MAS_KEY_NAME(EKey_ArrowRight);
        MAS_KEY_NAME(EKey_MouseWheelUp);
        MAS_KEY_NAME(EKey_MouseWheelDown);
        MAS_KEY_NAME(EKey_MouseLeft);
        MAS_KEY_NAME(EKey_MouseRight);
        MAS_KEY_NAME(EKey_MouseMiddle);
        MAS_KEY_NAME(EKey_MouseX1);
        MAS_KEY_NAME(EKey_MouseX2);
    }
#undef MAS_KEY_NAME

    return "EKey_Unknown\0";
}
const char* masInput_KeyModName(masEKeyMod KeyMod)
{
    uint32_t WriteIdx = 0;
    static char Buf[256];
    ::memset(Buf, 0, sizeof(char) * 256);

#define MAS_KEY_MOD_NAME(KEY_MOD) case KEY_MOD: WriteIdx += sprintf_s(Buf + WriteIdx, 256 - WriteIdx, #KEY_MOD); break
    for (int32_t i = 0; i < EKeyMod_Count; ++i)
    {
        uint16_t Mod = KeyMod & (1 << i);
        if (Mod == EKeyMod_None)
            continue;

        if (WriteIdx > 0 && ((1 << i) != EKeyMod_None))
            WriteIdx += sprintf_s(Buf + WriteIdx, 256 - WriteIdx, " | ");

        switch (Mod)
        {
            MAS_KEY_MOD_NAME(EKeyMod_Ctrl);
            MAS_KEY_MOD_NAME(EKeyMod_LCtrl);
            MAS_KEY_MOD_NAME(EKeyMod_RCtrl);
            MAS_KEY_MOD_NAME(EKeyMod_Shift);
            MAS_KEY_MOD_NAME(EKeyMod_LShift);
            MAS_KEY_MOD_NAME(EKeyMod_RShift);
            MAS_KEY_MOD_NAME(EKeyMod_Alt);
            MAS_KEY_MOD_NAME(EKeyMod_LAlt);
            MAS_KEY_MOD_NAME(EKeyMod_RAlt);
        }
    }
#undef MAS_KEY_MOD_NAME

    if (WriteIdx == 0)
        WriteIdx += sprintf_s(Buf + WriteIdx, 256 - WriteIdx, "masEKeyMod_None");

    return Buf;
}
const char* masInput_KeyState(masEKeyState KeyState)
{
    uint32_t WriteIdx = 0;
    static char Buf[256];
    ::memset(Buf, 0, sizeof(char) * 256);

#define MAS_KEY_STATE_NAME(KEY_STATE) case KEY_STATE: WriteIdx += sprintf_s(Buf + WriteIdx, 256 - WriteIdx, #KEY_STATE); break
    for (int32_t i = 0; i < EKeyState_Count; ++i)
    {
        uint8_t State = KeyState & (1 << i);
        if (State == EKeyState_None)
            continue;

        if (WriteIdx > 0 && ((i << 1) != EKeyState_None))
            WriteIdx += sprintf_s(Buf + WriteIdx, 256 - WriteIdx, " | ");

        switch (State)
        {
            MAS_KEY_STATE_NAME(EKeyState_Release);
            MAS_KEY_STATE_NAME(EKeyState_Press);
            MAS_KEY_STATE_NAME(EKeyState_Repeat);
            MAS_KEY_STATE_NAME(EKeyState_DoubleClick);
        }
    }
#undef MAS_KEY_STATE_NAME

    if (WriteIdx == 0)
        WriteIdx += sprintf_s(Buf + WriteIdx, 256 - WriteIdx, "masEKeyState_None");

    return Buf;
}