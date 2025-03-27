#include "Input.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "InputTempMemory.h"

/*
* TODO:
*     - VISUALIZE BITS FOR EVERY UINT32
*     - AXIS VALUE FOR[ R_ANALOG, L_ANALOG, TRIGGERS, MOUSE ]
*     - LIVE AXIS CHECK DATA
*     - LIVE EVENT CHECK DATA
*     - LIVE CHECK KEY MODIFIER
* 
* NOTE ABOUT AXIS HANDLING: 
*       Since axis only care about a bit being on and off when getting the state the press is 2 = 10 in binary and wont work
*       so quick fix just checking if 0 = EKeyState_None
*       *
*       Same for setting the bit for access EKeyState_Press wont work since 2 = 10 in binary and that's why we set it to 0 and 1 only 
*       if you treat axis like the event buffer 0 = EKeyState_None and 1 = EKeyState_Release.
*       *
*       Mouse handle requires having two snapshot of mouse position to determine its delta as axis value that gives us direction we need
*/



#define MAS_ADDR_FROM(TYPE, PTR, OFFSET) (TYPE*)(((uint8_t*)PTR) + OFFSET)

/*
* FORWARD DECLERATION
*/
static void masXInput_Init();
static void masXInput_Update();




/******************************************************************************************************
* API DATA
*******************************************************************************************************/
struct masKeyBuf
{
	uint32_t *Uint32Buf;
	uint32_t  Uint32Count;
	uint8_t   BitPerKey;
};

struct masInputUser
{
	masKeyBuf *EventKeyBuf;
	masKeyBuf *AxisKeyBuf;
	masVec3    Axes[6]; 
	bool       bActive;
};

static masInputUser GInputUsers[EInputUser_Count] = {};
static masVec3      GMousePos = {}; // To be used by EInputUser_0
static masVec3      GMouseDir = {};	// To be used by EInputUser_0
static uint16_t     GKeyMod   = {};	// To be used by EInputUser_0




/******************************************************************************************************
* 
*******************************************************************************************************/
static uint32_t masKeyBuf_CalculateUint32Count(uint32_t KeyCount, uint32_t BitPerKey)
{
	return (uint32_t)(::ceil((KeyCount * BitPerKey) / 32.f));
}
static bool masKeyBuf_Create()
{
	uint32_t UsersEventUint32[] =
	{
		masKeyBuf_CalculateUint32Count(256, 4),
		masKeyBuf_CalculateUint32Count(24,  4),
		masKeyBuf_CalculateUint32Count(24,  4),
		masKeyBuf_CalculateUint32Count(24,  4),
	};

	uint32_t UsersAxisUint32[] =
	{
		masKeyBuf_CalculateUint32Count(256, 1),
		masKeyBuf_CalculateUint32Count(24,  1),
		masKeyBuf_CalculateUint32Count(24,  1),
		masKeyBuf_CalculateUint32Count(24,  1),
	};

	uint32_t TotalUint32Count = 0;
	for (int32_t i = 0; i < EInputUser_Count; ++i)
	{
		printf("InputUser_%d: EventUint32Count = %u | AxisUint32Count = %u\n", i, UsersEventUint32[i], UsersAxisUint32[i]);
		TotalUint32Count += (UsersEventUint32[i] + UsersAxisUint32[i]);
	}
	printf("\n    ::TotalUint32Count: %u\n", TotalUint32Count);

	/*
	*/
	uint32_t KeyBufNumPerUser = 2;
	uint64_t MemSize = ((sizeof(masKeyBuf) * KeyBufNumPerUser) * EInputUser_Count) + (sizeof(uint32_t) * TotalUint32Count);
	printf("    ::TotalMemoryForKeyBuf: %llu\n\n", MemSize);

	/*
	*/
	uint8_t* Mem = (uint8_t*)::malloc(MemSize);
	if (!Mem)
		return false;
	else
		::memset(Mem, 0, MemSize);


	/*
	*/
	uint64_t UserInputDataOffset = 0;
	for (int32_t i = 0; i < EInputUser_Count; ++i)
	{
		masInputUser *User              = &GInputUsers[i];
		uint64_t      UserEventDataSize = (sizeof(masKeyBuf) + (sizeof(uint32_t) * UsersEventUint32[i]));
		uint64_t      UserAxisDataSize  = (sizeof(masKeyBuf) + (sizeof(uint32_t) * UsersAxisUint32[i]));

		// Setup User Input Event Key Buf
		User->EventKeyBuf = MAS_ADDR_FROM(masKeyBuf, Mem, UserInputDataOffset);
		User->EventKeyBuf->BitPerKey   = 4;
		User->EventKeyBuf->Uint32Count = UsersEventUint32[i];
		User->EventKeyBuf->Uint32Buf   = MAS_ADDR_FROM(uint32_t, User->EventKeyBuf, sizeof(masKeyBuf));

        // Setup User Input Axis Key Buf
		User->AxisKeyBuf = MAS_ADDR_FROM(masKeyBuf, User->EventKeyBuf, UserEventDataSize);
		User->AxisKeyBuf->BitPerKey   = 1;
		User->AxisKeyBuf->Uint32Count = UsersAxisUint32[i];
		User->AxisKeyBuf->Uint32Buf   = MAS_ADDR_FROM(uint32_t, User->AxisKeyBuf, sizeof(masKeyBuf));

		UserInputDataOffset += (UserEventDataSize + UserAxisDataSize);
	}


	return true;
}
static void masKeyBuf_Destroy()
{
	::free(GInputUsers[EInputUser_0].EventKeyBuf);
	for (int32_t i = 0; i < EInputUser_Count; ++i)
		GInputUsers[i].EventKeyBuf = GInputUsers[i].AxisKeyBuf = NULL;
}
static void masKeyBuf_SetState(masKeyBuf* KeyBuf, masEKey Key, uint32_t KeyState)
{
	uint32_t BytePos = Key / 32;
	uint32_t BitPos  = ((Key - 1) * KeyBuf->BitPerKey) % 32; // need to check the calculation

	// Build Mask to not affect other bits around our target bits
	uint32_t Mask = 0;
	for (int32_t i = 0; i < KeyBuf->BitPerKey; ++i)
		Mask |= (1 << i);
	Mask <<= BitPos;
	Mask = ~Mask;

	KeyBuf->Uint32Buf[BytePos] &= Mask; // Set Target Bits to Zero
	KeyBuf->Uint32Buf[BytePos] |= (KeyState << BitPos); // Set Target Bits to KeyState
	uint32_t Buf = KeyBuf->Uint32Buf[BytePos];

	int j = 0;
}
static masEKeyState masKeybuf_GetState(masKeyBuf* KeyBuf, masEKey Key)
{
	uint32_t BytePos = Key / 32;
	uint32_t BitPos  = ((Key - 1) * KeyBuf->BitPerKey) % 32; // need to check the calculation

	// Build Mask to get only the bits belong to our Key
	uint32_t Mask = 0;
	for (int32_t i = 0; i < KeyBuf->BitPerKey; ++i)
		Mask |= (1 << i);

	uint32_t Data     = KeyBuf->Uint32Buf[BytePos];
	Data >>= BitPos;
	uint32_t KeyState = (Data & Mask);

	return (masEKeyState)KeyState;
}
static void masInput_UpdateKeyMod()
{
	GKeyMod = 0;

	uint16_t CtrlMask  = EKeyMod_LCtrl  | EKeyMod_RCtrl;
	uint16_t ShiftMask = EKeyMod_LShift | EKeyMod_RShift;
	uint16_t AltMask   = EKeyMod_LAlt   | EKeyMod_RAlt;

	GKeyMod |= GetAsyncKeyState(VK_LCONTROL) ? EKeyMod_LCtrl  : 0;
	GKeyMod |= GetAsyncKeyState(VK_RCONTROL) ? EKeyMod_RCtrl  : 0;
	GKeyMod |= GetAsyncKeyState(VK_LSHIFT)   ? EKeyMod_LShift : 0;
	GKeyMod |= GetAsyncKeyState(VK_RSHIFT)   ? EKeyMod_RShift : 0;
	GKeyMod |= GetAsyncKeyState(VK_LMENU)    ? EKeyMod_LAlt   : 0;
	GKeyMod |= GetAsyncKeyState(VK_RMENU)    ? EKeyMod_RAlt   : 0;

	if (GKeyMod & CtrlMask)
		GKeyMod |= EKeyMod_Ctrl;
	if (GKeyMod & ShiftMask)
		GKeyMod |= EKeyMod_Shift;
	if (GKeyMod & AltMask)
		GKeyMod |= EKeyMod_Alt;
}
static bool masInput_IsKeyModActive(uint16_t UserKeyMod)
{
	uint16_t ConvUserKeyMod = UserKeyMod; // to insert alt, ctrl, shift if UserKeyMod only have the extended
	uint16_t CtrlMask  = EKeyMod_LCtrl  | EKeyMod_RCtrl;
	uint16_t ShiftMask = EKeyMod_LShift | EKeyMod_RShift;
	uint16_t AltMask   = EKeyMod_LAlt   | EKeyMod_RAlt;

	if (ConvUserKeyMod & CtrlMask)
		ConvUserKeyMod |= EKeyMod_Ctrl;
	else if (ConvUserKeyMod == EKeyMod_Ctrl)
		ConvUserKeyMod |= (EKeyMod_LCtrl | EKeyMod_RCtrl); // need to know if lctrl or rctrl was pressed

	if (ConvUserKeyMod & ShiftMask)
		ConvUserKeyMod |= EKeyMod_Shift;
	else if(ConvUserKeyMod == EKeyMod_Shift)
		ConvUserKeyMod |= (EKeyMod_LShift | EKeyMod_RShift);

	if (ConvUserKeyMod & AltMask)
		ConvUserKeyMod |= EKeyMod_Alt;
	else if (ConvUserKeyMod == EKeyMod_Alt)
		ConvUserKeyMod |= (EKeyMod_LAlt | EKeyMod_RAlt);

	// for making sure that bits are active event if GKeyMod has other keys active
	// here we only check for the bits we need
	if ((GKeyMod & UserKeyMod) == UserKeyMod) 
	{
		// since our bits above are active we active the non-extended modkey in ConvUserKeyMod since that what we do in GKeyMod
		// and here where we check for exact equality
		if(GKeyMod == ConvUserKeyMod) 
		    return true;
	}

	return false;
}




/******************************************************************************************************
* API Init & DeInit & Update
*******************************************************************************************************/
bool masInput_Init()
{
	if (!masInputTempMem_Create())
		return false;

	if (!masKeyBuf_Create())
		return false;

	GInputUsers[EInputUser_0].bActive = true;
	GInputUsers[EInputUser_1].bActive = false;
	GInputUsers[EInputUser_2].bActive = false;
	GInputUsers[EInputUser_3].bActive = false;

	masXInput_Init();

	return true;
}

void masInput_DeInit()
{
	masInputTempMem_Destroy();
	masKeyBuf_Destroy();
}

void masInput_Update()
{
	// RESET EVENT KEY BUF
	for (int32_t i = 0; i < EInputUser_Count; ++i)
	{
		if (!GInputUsers[i].bActive)
			continue;
		::memset(GInputUsers[i].EventKeyBuf->Uint32Buf, 0, sizeof(uint32_t) * GInputUsers[i].EventKeyBuf->Uint32Count);
	}
	masInputTempMem_Reset();


	masInput_UpdateKeyMod();
	masXInput_Update();
}


/******************************************************************************************************
* Utils Functions
*******************************************************************************************************/
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

	if(WriteIdx == 0)
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
int32_t     masInput_MapEKeyToAxisIdx(masEKey Key)
{
	switch (Key)
	{
	case EKey_RAnalogUp:
	case EKey_RAnalogDown:
		return 0;

	case EKey_RAnalogLeft:
	case EKey_RAnalogRight:
		return 1;

	case EKey_LAnalogUp:
	case EKey_LAnalogDown:
		return 2;

	case EKey_LAnalogLeft:
	case EKey_LAnalogRight:
		return 3;

	case EKey_L2:
		return 4;

	case EKey_R2:
		return 5;

	case EKey_MouseLeft:
	case EKey_MouseRight:
	case EKey_MouseMiddle:
	case EKey_MouseX2:
	case EKey_MouseX1:
	case EKey_MouseWheelDown:
	case EKey_MouseWheelUp:
		return 6;
	}

	return -1;
}




/******************************************************************************************************
* 
*******************************************************************************************************/
static void masInput_OnKey(masEInputUser InputUser, masEKey Key, masEKeyState KeyState)
{
	if (InputUser < EInputUser_0)
		return;

	if (InputUser == EInputUser_0 && (Key >= EKey_Count || Key <= EKey_Unknown))
		return;

	if ((InputUser > EInputUser_0 && InputUser < EInputUser_Count) && (Key >= EKey_GamepadCount || Key <= EKey_Unknown))
		return;

	masKeyBuf_SetState(GInputUsers[InputUser].AxisKeyBuf,  Key, (KeyState == EKeyState_Release) ? 0 : 1);
	masKeyBuf_SetState(GInputUsers[InputUser].EventKeyBuf, Key, KeyState);
}
static void masInput_OnAxis(masEInputUser InputUser, masEKey Key, float AxisX, float AxisY, float AxisZ)
{
	if (InputUser < EInputUser_0)
		return;

	if (InputUser == EInputUser_0 && (Key >= EKey_Count || Key <= EKey_Unknown))
		return;

	if ((InputUser > EInputUser_0 && InputUser < EInputUser_Count) && (Key >= EKey_GamepadCount || Key <= EKey_Unknown))
		return;

	int32_t AxisIdx = masInput_MapEKeyToAxisIdx(Key);
	if (AxisIdx == -1)
		return;

	GInputUsers[InputUser].Axes[AxisIdx] = { AxisX, AxisY, AxisZ };
}




/******************************************************************************************************
*
*
*******************************************************************************************************/
struct masAxisKeyDesc
{
	masEKey  *Keys;
	uint16_t  KeyMod;
	masVec3   AxisValue;
	uint8_t   KeysCount;
	bool      bAllMustBeActive;
};

struct masAxisKey
{
	char            *Name;
	masAxisKeyDesc **Axes;
	uint32_t         Count;
};

masAxisKeyDesc* masInput_DescribeAxisKey(bool MustAllBeActive, float AxisX, float AxisY, float AxisZ, uint16_t KeyMod, const char* KeyListFmt, ...)
{
	uint32_t KeyListFmt_Len = (uint32_t)::strlen(KeyListFmt);
	if (KeyListFmt_Len == 0)
		return NULL;
	if (KeyListFmt_Len == 1 && KeyListFmt[0] == '\n')
		return NULL;

	uint32_t KeyCount = 0;
	for (uint32_t i = 0; i < KeyListFmt_Len; ++i)
		if (KeyListFmt[i] == ',' || KeyListFmt[i] == '\n')
			KeyCount++;

	uint32_t        MemSize = sizeof(masAxisKeyDesc) + (sizeof(masEKey) * KeyCount);
	masAxisKeyDesc *AxisKey = (masAxisKeyDesc*)masInputTempMem_Alloc(MemSize);
	if (!AxisKey)
		return NULL;

	AxisKey->Keys             = MAS_ADDR_FROM(masEKey, AxisKey, sizeof(masAxisKeyDesc));
	AxisKey->KeysCount        = KeyCount;
	AxisKey->bAllMustBeActive = MustAllBeActive;
	AxisKey->KeyMod           = KeyMod;
	AxisKey->AxisValue        = { AxisX, AxisY, AxisZ };

	va_list Args;
	va_start(Args, KeyCount);
	for (uint32_t KeyIdx = 0; KeyIdx < KeyCount; ++KeyIdx)
		AxisKey->Keys[KeyIdx] = va_arg(Args, masEKey);
	va_end(Args);

	return AxisKey;
}
masAxisKey*     masInput_CreateAxisKey(const char* Name, masAxisKeyDesc** AxisKeyDescList, uint32_t ListCount)
{
	if (!AxisKeyDescList)
		return NULL;

	uint32_t NameLen   = (uint32_t)::strlen(Name);
	if (NameLen > 0)
		NameLen++; // NULL Teriminator

	uint32_t    MemSize = NameLen + sizeof(masAxisKey);
	masAxisKey *AxisKey = (masAxisKey*)masInputTempMem_Alloc(MemSize);
	if (!AxisKey)
		return NULL;

	
	AxisKey->Axes  = AxisKeyDescList;
	AxisKey->Count = ListCount;

	if (NameLen > 0)
	{
		AxisKey->Name = MAS_ADDR_FROM(char, AxisKey, sizeof(masAxisKey));
		::memcpy(AxisKey->Name, Name, NameLen - 1);
	}
	
	return AxisKey;
}
masVec3*        masInput_OnAxisKey(masEInputUser InputUser, masAxisKey* AxisKey)
{
#if 0
	printf("AXIS[ %s ]:\n", AxisKey->Name);
	for (uint32_t AxisIdx = 0; AxisIdx < AxisKey->Count; ++AxisIdx)
	{
	    printf("    KEYS_DEFS[ %u ]: \n", AxisIdx);
		masAxisKeyDesc* AxisDesc = AxisKey->Axes[AxisIdx];

		printf("        KEY_COUNT:              %u\n", AxisDesc->KeysCount);
		printf("        KEYS:                   [ ");
		for (int32_t KeyIdx = 0; KeyIdx < AxisDesc->KeysCount; ++KeyIdx)
		{
			if (KeyIdx > 0 && ((KeyIdx + 1) < AxisDesc->KeysCount))
				printf(" | ");
			printf("%s", masInput_KeyName(AxisDesc->Keys[KeyIdx]));
		}
		printf(" ]\n");

		printf("        KEY_MOD:                %s\n", masInput_KeyModName(AxisDesc->KeyMod));
		printf("        AXIS_VALUE:             [ %.1f, %.1f, %.1f ]\n", AxisDesc->AxisValue.x, AxisDesc->AxisValue.y, AxisDesc->AxisValue.z);
		printf("        ALL_KEY_MUST_BE_ACTIVE: %s\n\n", (AxisDesc->bAllMustBeActive) ? "TRUE" : "FALSE");
	}
#endif

	masVec3* AxisValue = (masVec3*)masInputTempMem_Alloc(sizeof(masVec3));
	if (!AxisValue)
		return NULL;

	for (uint32_t AxisIdx = 0; AxisIdx < AxisKey->Count; ++AxisIdx)
	{
		masAxisKeyDesc* AxisDesc = AxisKey->Axes[AxisIdx];
		if (!AxisDesc)
			continue;

		if (!masInput_IsKeyModActive(AxisDesc->KeyMod))
			continue;

		masVec3 AccumAxisValue = {};
		for (int32_t KeyIdx = 0; KeyIdx < AxisDesc->KeysCount; ++KeyIdx)
		{
			masEKey Key = AxisDesc->Keys[KeyIdx];
			if (InputUser < EInputUser_0)
				continue;
			if (InputUser == EInputUser_0 && (Key >= EKey_Count || Key <= EKey_Unknown))
				continue;
			if ((InputUser > EInputUser_0 && InputUser < EInputUser_Count) && (Key >= EKey_GamepadCount || Key <= EKey_Unknown))
				continue;

			// Check that axis key is active
			masEKeyState KeyState = masKeybuf_GetState(GInputUsers[InputUser].AxisKeyBuf, Key);
			if (KeyState == EKeyState_None)
			{
				if (AxisDesc->bAllMustBeActive)
				{
					::memset(&AccumAxisValue, 0, sizeof(masVec3));
					break;
				}
				continue;
			}

			// Query Axis Value if it have native axis value such as triggers and analogs
			int32_t NativeAxisValueIdx = masInput_MapEKeyToAxisIdx(Key);
			if (NativeAxisValueIdx == -1)
			{
				AccumAxisValue.x += AxisDesc->AxisValue.x;
				AccumAxisValue.y += AxisDesc->AxisValue.y;
				AccumAxisValue.z += AxisDesc->AxisValue.z;
			}
			else
			{
				if (NativeAxisValueIdx == 6) // mouse axis cuz only user_0 will use so it's not in the buffer as Axes
				{
					AccumAxisValue.x += GMouseDir.x;
					AccumAxisValue.y += GMouseDir.y;
					AccumAxisValue.z += GMouseDir.z;
				}
				else
				{
					AccumAxisValue.x += GInputUsers[InputUser].Axes[NativeAxisValueIdx].x;
					AccumAxisValue.y += GInputUsers[InputUser].Axes[NativeAxisValueIdx].y;
					AccumAxisValue.z += GInputUsers[InputUser].Axes[NativeAxisValueIdx].z;
				}
				
			}
		}

		AxisValue->x += AccumAxisValue.x;
		AxisValue->y +=	AccumAxisValue.y;
		AxisValue->z +=	AccumAxisValue.z;
	}

	// Clamp Values between 1 .. -1
	if (AxisValue->x >  1.f) AxisValue->x =  1.f;
	if (AxisValue->x < -1.f) AxisValue->x = -1.f;
	if (AxisValue->y >  1.f) AxisValue->y =  1.f;
	if (AxisValue->y < -1.f) AxisValue->y = -1.f;
	if (AxisValue->z >  1.f) AxisValue->z =  1.f;
	if (AxisValue->z < -1.f) AxisValue->z = -1.f;

	return AxisValue;
}




/******************************************************************************************************
*
*
*******************************************************************************************************/
struct masEventKeyDesc
{
	masEKey  *Keys;
	uint16_t  KeyMod;
	uint8_t   KeyState;
	uint8_t   KeyCount;
	bool      bAllMustBeActive;
};

struct masEventKey
{
	char             *Name;
	masEventKeyDesc **KeyDescList;
	uint32_t          Count;
};

masEventKeyDesc* masInput_DescribeEventKey(bool MustAllBeActive, uint16_t KeyMod, uint8_t KeyState, const char* KeyListFmt, ...)
{
	uint32_t KeyListFmt_Len = (uint32_t)::strlen(KeyListFmt);
	if (KeyListFmt_Len == 0)
		return NULL;
	if (KeyListFmt_Len == 1 && KeyListFmt[0] == '\n')
		return NULL;

	uint32_t KeyCount = 0;
	for (uint32_t i = 0; i < KeyListFmt_Len; ++i)
		if (KeyListFmt[i] == ',' || KeyListFmt[i] == '\n')
			KeyCount++;

	uint32_t         MemSize = sizeof(masEventKeyDesc) + (sizeof(masEKey) * KeyCount);
	masEventKeyDesc *EventKey = (masEventKeyDesc*)masInputTempMem_Alloc(MemSize);
	if (!EventKey)
		return NULL;

	EventKey->Keys             = MAS_ADDR_FROM(masEKey, EventKey, sizeof(masEventKeyDesc));
	EventKey->KeyCount         = KeyCount;
	EventKey->bAllMustBeActive = MustAllBeActive;
	EventKey->KeyMod           = KeyMod;
	EventKey->KeyState         = KeyState;

	va_list Args;
	va_start(Args, KeyCount);
	for (uint32_t KeyIdx = 0; KeyIdx < KeyCount; ++KeyIdx)
		EventKey->Keys[KeyIdx] = va_arg(Args, masEKey);
	va_end(Args);

	return EventKey;
}
masEventKey*     masInput_CreateEventKey(const char* Name, masEventKeyDesc** EventKeyDescList, uint32_t ListCount)
{
	if (!EventKeyDescList)
		return NULL;

	uint32_t NameLen = (uint32_t)::strlen(Name);
	if (NameLen > 0)
		NameLen++; // NULL Teriminator

	uint32_t     MemSize  = NameLen + sizeof(masAxisKey);
	masEventKey *EventKey = (masEventKey*)masInputTempMem_Alloc(MemSize);
	if (!EventKey)
		return NULL;


	EventKey->KeyDescList = EventKeyDescList;
	EventKey->Count       = ListCount;

	if (NameLen > 0)
	{
		EventKey->Name = MAS_ADDR_FROM(char, EventKey, sizeof(masAxisKey));
		::memcpy(EventKey->Name, Name, NameLen - 1);
	}

	return EventKey;
}
bool             masInput_OnEventKey(masEInputUser InputUser, masEventKey* EventKey)
{
	for (uint32_t EventIdx = 0; EventIdx < EventKey->Count; ++EventIdx)
	{
		masEventKeyDesc* EventKeyDesc = EventKey->KeyDescList[EventIdx];
		if (!EventKeyDesc)
			continue;

		if (!masInput_IsKeyModActive(EventKeyDesc->KeyMod))
			continue;

		uint32_t ActiveKeyCount = 0;
		for (int32_t KeyIdx = 0; KeyIdx < EventKeyDesc->KeyCount; ++KeyIdx)
		{
			masEKey Key = EventKeyDesc->Keys[KeyIdx];
			if (InputUser < EInputUser_0)
				continue;
			if (InputUser == EInputUser_0 && (Key >= EKey_Count || Key <= EKey_Unknown))
				continue;
			if ((InputUser > EInputUser_0 && InputUser < EInputUser_Count) && (Key >= EKey_GamepadCount || Key <= EKey_Unknown))
				continue;

			// Check that axis key is active
			masEKeyState KeyState = masKeybuf_GetState(GInputUsers[InputUser].EventKeyBuf, Key);
			if (EventKeyDesc->KeyState == KeyState)
				ActiveKeyCount++;
		}

		if (EventKeyDesc->bAllMustBeActive)
		{
			if(ActiveKeyCount == EventKeyDesc->KeyCount)
			    return true;
		}
		else if(ActiveKeyCount > 0)
			return true;
	}

	return false;
}






/******************************************************************************************************
*
* XINPUT CODE
*
*******************************************************************************************************/
#define RepeatInitTime    0.2f
#define RepeatAdvanceTime 0.02f

struct masGamepad
{
	float LAnalogDeadZone;
	float RAnalogDeadZone;
	float LTriggerThreshold;
	float RTriggerThreshold;
	float RepeatTime[EKey_GamepadCount];
	bool  LastState [EKey_GamepadCount];
	bool  bAvailable;
};

static masGamepad GGamepads[EInputUser_Count] = {};

void masXInput_CheckControllersConnection()
{
	for (int32_t UserIdx = 0; UserIdx < EInputUser_Count; ++UserIdx)
	{
		XINPUT_STATE XInputState = {};
		if (::XInputGetState(UserIdx, &XInputState) == ERROR_DEVICE_NOT_CONNECTED)
		{
			GGamepads[UserIdx].bAvailable = false;
			if (UserIdx != EInputUser_0)
				GInputUsers[UserIdx].bActive = false;
		}
		else
			GGamepads[UserIdx].bAvailable = true;
	}
}

void masXInput_Init()
{
	masXInput_CheckControllersConnection();

	for (int32_t UserIdx = 0; UserIdx < EInputUser_Count; ++UserIdx)
	{
		GGamepads[UserIdx].LTriggerThreshold = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
		GGamepads[UserIdx].RTriggerThreshold = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
		GGamepads[UserIdx].LAnalogDeadZone   = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
		GGamepads[UserIdx].RAnalogDeadZone   = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
		for (int32_t ButtonIdx = 0; ButtonIdx < EKey_GamepadCount; ++ButtonIdx)
			GGamepads[UserIdx].RepeatTime[ButtonIdx] = RepeatInitTime;
	}
}

void masXInput_Update()
{
	XINPUT_STATE    XInputState = {};
	XINPUT_GAMEPAD* XGamepad = NULL;
	for (int32_t UserIdx = 0; UserIdx < EInputUser_Count; ++UserIdx)
	{
		masGamepad* Gamepad = &GGamepads[UserIdx];
		if (!Gamepad->bAvailable)
			continue;

		::memset(&XInputState, 0, sizeof(XINPUT_STATE));
		DWORD Ret = ::XInputGetState(UserIdx, &XInputState);
		if (Ret == ERROR_DEVICE_NOT_CONNECTED)
		{
			Gamepad->bAvailable = false;
			continue;
		}
		XGamepad = &XInputState.Gamepad;

		static bool Buttons[EKey_GamepadCount];
		::memset(Buttons, 0, sizeof(bool) * EKey_GamepadCount);

		Buttons[EKey_Square]       = XGamepad->wButtons & XINPUT_GAMEPAD_X;
		Buttons[EKey_Cross]        = XGamepad->wButtons & XINPUT_GAMEPAD_A;
		Buttons[EKey_Circle]       = XGamepad->wButtons & XINPUT_GAMEPAD_B;
		Buttons[EKey_Triangle]     = XGamepad->wButtons & XINPUT_GAMEPAD_Y;
		Buttons[EKey_Start]        = XGamepad->wButtons & XINPUT_GAMEPAD_START;
		Buttons[EKey_Select]       = XGamepad->wButtons & XINPUT_GAMEPAD_BACK;
		Buttons[EKey_DpadUp]       = XGamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
		Buttons[EKey_DpadDown]     = XGamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
		Buttons[EKey_DpadRight]    = XGamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
		Buttons[EKey_DpadLeft]     = XGamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
		Buttons[EKey_L1]           = XGamepad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
		Buttons[EKey_R1]           = XGamepad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
		Buttons[EKey_L3]           = XGamepad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
		Buttons[EKey_R3]           = XGamepad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
		/////////////////////////////////////////////////////////////////////////////////////
		Buttons[EKey_L2]           = XGamepad->bLeftTrigger  > Gamepad->LTriggerThreshold;
		Buttons[EKey_R2]           = XGamepad->bRightTrigger > Gamepad->RTriggerThreshold;
		Buttons[EKey_LAnalogUp]    = XGamepad->sThumbLY >  Gamepad->LAnalogDeadZone;
		Buttons[EKey_LAnalogDown]  = XGamepad->sThumbLY < -Gamepad->LAnalogDeadZone;
		Buttons[EKey_LAnalogRight] = XGamepad->sThumbLX >  Gamepad->LAnalogDeadZone;
		Buttons[EKey_LAnalogLeft]  = XGamepad->sThumbLX < -Gamepad->LAnalogDeadZone;
		Buttons[EKey_RAnalogUp]    = XGamepad->sThumbRY >  Gamepad->RAnalogDeadZone;
		Buttons[EKey_RAnalogDown]  = XGamepad->sThumbRY < -Gamepad->RAnalogDeadZone;
		Buttons[EKey_RAnalogRight] = XGamepad->sThumbRX >  Gamepad->RAnalogDeadZone;
		Buttons[EKey_RAnalogLeft]  = XGamepad->sThumbRX < -Gamepad->RAnalogDeadZone;

		masEInputUser InputUser = (masEInputUser)UserIdx;
		
		/*
		* Dispatch axis keys L2, R2, LAnalog, RAnalog
		*/
		if (Buttons[EKey_L2])           masInput_OnAxis(InputUser, EKey_L2,           XGamepad->bLeftTrigger  / 255.f ,  0.f, 0.f);
		if (Buttons[EKey_R2])           masInput_OnAxis(InputUser, EKey_R2,           XGamepad->bRightTrigger / 255.f,   0.f, 0.f);
		if (Buttons[EKey_LAnalogUp])    masInput_OnAxis(InputUser, EKey_LAnalogUp,    0.f, XGamepad->sThumbLY / 32767.f, 0.f);
		if (Buttons[EKey_LAnalogDown])	masInput_OnAxis(InputUser, EKey_LAnalogDown,  0.f, XGamepad->sThumbLY / 32768.f, 0.f);
		if (Buttons[EKey_LAnalogRight]) masInput_OnAxis(InputUser, EKey_LAnalogRight, XGamepad->sThumbLX      / 32767.f, 0.f, 0.f);
		if (Buttons[EKey_LAnalogLeft])	masInput_OnAxis(InputUser, EKey_LAnalogLeft,  XGamepad->sThumbLX      / 32768.f, 0.f, 0.f);
		if (Buttons[EKey_RAnalogUp])	masInput_OnAxis(InputUser, EKey_RAnalogUp,    0.f, XGamepad->sThumbRY / 32767.f, 0.f);
		if (Buttons[EKey_RAnalogDown])	masInput_OnAxis(InputUser, EKey_RAnalogDown,  0.f, XGamepad->sThumbRY / 32768.f, 0.f);
		if (Buttons[EKey_RAnalogRight]) masInput_OnAxis(InputUser, EKey_RAnalogRight, XGamepad->sThumbRX      / 32767.f, 0.f, 0.f);
		if (Buttons[EKey_RAnalogLeft])	masInput_OnAxis(InputUser, EKey_RAnalogLeft,  XGamepad->sThumbRX      / 32768.f, 0.f, 0.f);


		/*
		* Dispatch Buttons
		*/
		for (int32_t ButtonIdx = 0; ButtonIdx < EKey_GamepadCount; ++ButtonIdx)
		{
			masEKey Key     = (masEKey)ButtonIdx;
			bool CurrState  = Buttons[ButtonIdx];
			bool LastState  = Gamepad->LastState[ButtonIdx];
			bool IsPressed  = !LastState && CurrState;
			bool IsReleased = LastState && !CurrState;
			bool IsRepeated = LastState && CurrState;

			if (IsRepeated)
			{
				if (Gamepad->RepeatTime[ButtonIdx] < Gamepad->RepeatTime[ButtonIdx] + RepeatInitTime)
					Gamepad->RepeatTime[ButtonIdx] += RepeatAdvanceTime;
				else
				{
					masInput_OnKey(InputUser, Key, EKeyState_Repeat);
					Gamepad->RepeatTime[ButtonIdx] += RepeatInitTime;
				}
			}
			else if (IsReleased)
				masInput_OnKey(InputUser, Key, EKeyState_Release);
			else if (IsPressed)
				masInput_OnKey(InputUser, Key, EKeyState_Press);
		}

		::memcpy(Gamepad->LastState, Buttons, sizeof(bool) * EKey_GamepadCount);
	}
}




/******************************************************************************************************
* 
* WIN32 CODE
* 
*******************************************************************************************************/
static masEKey masInput_ConvertWin32Key(WPARAM Wparam);

LRESULT masInput_Win32Proc(HWND Hwnd, UINT Msg, WPARAM Wparam, LPARAM Lparam)
{
	switch (Msg)
	{
	/*
	* KEYBOARD EVENT HANDLE
	*/
	case WM_KEYUP:
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	{
		WORD vkCode        = LOWORD(Wparam);                          // virtual-key code
		WORD keyFlags      = HIWORD(Lparam);
		WORD scanCode      = LOBYTE(keyFlags);                        // scan code
		BOOL isExtendedKey = (keyFlags & KF_EXTENDED) == KF_EXTENDED; // extended-key flag, 1 if scancode has 0xE0 prefix

		if (isExtendedKey)
			scanCode = MAKEWORD(scanCode, 0xE0);

		BOOL wasKeyDown    = (keyFlags & KF_REPEAT) == KF_REPEAT;     // previous key-state flag, 1 on autorepeat
		WORD repeatCount   = LOWORD(Lparam);                          // repeat count, > 0 if several keydown messages was combined into one message
		BOOL isKeyReleased = (keyFlags & KF_UP) == KF_UP;             // transition-state flag, 1 on keyup

		//switch (vkCode)
		//{
		//case VK_SHIFT:   // converts to VK_LSHIFT or VK_RSHIFT
		//	vkCode = LOWORD(MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX));
		//case VK_CONTROL: // converts to VK_LCONTROL or VK_RCONTROL
		//	vkCode = LOWORD(MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX));
		//case VK_MENU:    // converts to VK_LMENU or VK_RMENU
		//	vkCode = LOWORD(MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX));
		//	break;
		//}

		masEKey Key = masInput_ConvertWin32Key(vkCode);
		if (isKeyReleased)
			masInput_OnKey(EInputUser_0, Key, EKeyState_Release);
		else
		{
			if (wasKeyDown)
				masInput_OnKey(EInputUser_0, Key, EKeyState_Repeat);
			else
				masInput_OnKey(EInputUser_0, Key, EKeyState_Press);
		}
	}

		break;


    /*
	* TEXT EVENT HANDLE
	*/
	case WM_CHAR:
	case WM_SYSCHAR:
	case WM_UNICHAR:
		//masInput_OnText(Unicode);
		break;


	/*
	* MOUSE EVENT HANDLE
	*/
	case WM_MOUSEMOVE:
		//masInput_OnMouseMove(x, y);

		GMouseDir.x = GET_X_LPARAM(Lparam) - GMousePos.x;
		GMouseDir.y = GET_Y_LPARAM(Lparam) - GMousePos.y;
		if (GMouseDir.x >  10.f) GMouseDir.x =  1.f;
		if (GMouseDir.x < -10.f) GMouseDir.x = -1.f;
		if (GMouseDir.y >  10.f) GMouseDir.y =  1.f;
		if (GMouseDir.y < -10.f) GMouseDir.y = -1.f;

		GMousePos.x = GET_X_LPARAM(Lparam);
		GMousePos.y = GET_Y_LPARAM(Lparam);
		break;

	case WM_MOUSEWHEEL:
	{
		short Delta = GET_WHEEL_DELTA_WPARAM(Wparam);
		if (Delta > 0)
			masInput_OnKey(EInputUser_0, EKey_MouseWheelUp, EKeyState_Press);
		else if (Delta < 0)
			masInput_OnKey(EInputUser_0, EKey_MouseWheelDown, EKeyState_Press);
	}
		break;

	case WM_LBUTTONDOWN:   masInput_OnKey(EInputUser_0, EKey_MouseLeft, EKeyState_Press);       break;
	case WM_LBUTTONUP:	   masInput_OnKey(EInputUser_0, EKey_MouseLeft, EKeyState_Release);     break;
	case WM_LBUTTONDBLCLK: masInput_OnKey(EInputUser_0, EKey_MouseLeft, EKeyState_DoubleClick); break;
						   
	case WM_RBUTTONDOWN:   masInput_OnKey(EInputUser_0, EKey_MouseRight, EKeyState_Press);       break;
	case WM_RBUTTONUP:	   masInput_OnKey(EInputUser_0, EKey_MouseRight, EKeyState_Release);     break;
	case WM_RBUTTONDBLCLK: masInput_OnKey(EInputUser_0, EKey_MouseRight, EKeyState_DoubleClick); break;
						   
	case WM_MBUTTONDOWN:   masInput_OnKey(EInputUser_0, EKey_MouseMiddle, EKeyState_Press);       break;
	case WM_MBUTTONUP:	   masInput_OnKey(EInputUser_0, EKey_MouseMiddle, EKeyState_Release);     break;
	case WM_MBUTTONDBLCLK: masInput_OnKey(EInputUser_0, EKey_MouseMiddle, EKeyState_DoubleClick); break;

	case WM_XBUTTONDOWN:   break;
	case WM_XBUTTONUP:	   break;
	case WM_XBUTTONDBLCLK: break;


	/*
	* 
	*/
	case WM_DEVICECHANGE:
		masXInput_CheckControllersConnection();
		break;
	}
	return 0;
}

masEKey masInput_ConvertWin32Key(WPARAM Wparam)
{
	switch (Wparam)
	{
	case 'A': return EKey_A;
	case 'B': return EKey_B;
	case 'C': return EKey_C;
	case 'D': return EKey_D;
	case 'E': return EKey_E;
	case 'F': return EKey_F;
	case 'G': return EKey_G;
	case 'H': return EKey_H;
	case 'I': return EKey_I;
	case 'J': return EKey_J;
	case 'K': return EKey_K;
	case 'L': return EKey_L;
	case 'M': return EKey_M;
	case 'N': return EKey_N;
	case 'O': return EKey_O;
	case 'P': return EKey_P;
	case 'Q': return EKey_Q;
	case 'R': return EKey_R;
	case 'S': return EKey_S;
	case 'T': return EKey_T;
	case 'U': return EKey_U;
	case 'V': return EKey_V;
	case 'W': return EKey_W;
	case 'X': return EKey_X;
	case 'Y': return EKey_Y;
	case 'Z': return EKey_Z;

	case 0x30: return EKey_Num0;
	case 0x31: return EKey_Num1;
	case 0x32: return EKey_Num2;
	case 0x33: return EKey_Num3;
	case 0x34: return EKey_Num4;
	case 0x35: return EKey_Num5;
	case 0x36: return EKey_Num6;
	case 0x37: return EKey_Num7;
	case 0x38: return EKey_Num8;
	case 0x39: return EKey_Num9;
	
	case VK_NUMLOCK: return EKey_NumLock;
	case VK_NUMPAD0: return EKey_Numpad0;
	case VK_NUMPAD1: return EKey_Numpad1;
	case VK_NUMPAD2: return EKey_Numpad2;
	case VK_NUMPAD3: return EKey_Numpad3;
	case VK_NUMPAD4: return EKey_Numpad4;
	case VK_NUMPAD5: return EKey_Numpad5;
	case VK_NUMPAD6: return EKey_Numpad6;
	case VK_NUMPAD7: return EKey_Numpad7;
	case VK_NUMPAD8: return EKey_Numpad8;
	case VK_NUMPAD9: return EKey_Numpad9;

	case VK_F1:  return EKey_F1;
	case VK_F2:  return EKey_F2;
	case VK_F3:  return EKey_F3;
	case VK_F4:  return EKey_F4;
	case VK_F5:  return EKey_F5;
	case VK_F6:  return EKey_F6;
	case VK_F7:  return EKey_F7;
	case VK_F8:  return EKey_F8;
	case VK_F9:  return EKey_F9;
	case VK_F10: return EKey_F10;
	case VK_F11: return EKey_F11;
	case VK_F12: return EKey_F12;

	case VK_DECIMAL:  return EKey_Decimal;
	case VK_PRIOR:    return EKey_PageUp;
	case VK_NEXT:     return EKey_PageDown;
	case VK_SPACE:    return EKey_Space;
	case VK_RETURN:   return EKey_Enter;
	case VK_BACK:     return EKey_Backspace;
	case VK_TAB:      return EKey_Tab;
	case VK_SNAPSHOT: return EKey_PrintScreen;
	case VK_INSERT:   return EKey_Insert;
	case VK_DELETE:   return EKey_Delete;
	case VK_DIVIDE:   return EKey_Divide;
	case VK_MULTIPLY: return EKey_Multipy;
	case VK_SUBTRACT: return EKey_Subtract;
	case VK_ADD:      return EKey_Addition;
	case VK_HOME:     return EKey_Home;
	case VK_END:      return EKey_End;
	case VK_ESCAPE:   return EKey_Escape;
	case VK_CAPITAL:  return EKey_CapsLock;
	case VK_UP:       return EKey_ArrowUp;
	case VK_DOWN:     return EKey_ArrowDown;
	case VK_LEFT:     return EKey_ArrowLeft;
	case VK_RIGHT:    return EKey_ArrowRight;
	}

	return EKey_Unknown;
}
