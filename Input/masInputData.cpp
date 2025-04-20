#include "masInputData.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAS_ADDR_FROM(TYPE, PTR, OFFSET) (TYPE*)(((uint8_t*)PTR) + OFFSET)
#define EKey_GamepadCount (EKey_RAnalogRight + 1)

masInputData GInputData = {};

/*********************************************************************************
*
/*********************************************************************************/
bool masInputData_IsKeyValid(masEInputUser InputUser, masEKey Key)
{
	if (InputUser == EInputUser_0)
	{
		if (Key >= EKey_Count || Key <= EKey_Unknown)
			return false;
	}
	else if (InputUser >= EInputUser_1 && InputUser <= EInputUser_3)
	{
		if (Key > EKey_RAnalogRight || Key < EKey_Square)
			return false;
	}

	return true;
}


/*********************************************************************************
*
/*********************************************************************************/
static uint32_t masKeyBuf_CalculateUint32Count(uint32_t KeyCount, uint32_t BitPerKey)
{
	return (uint32_t)(::ceil((KeyCount * BitPerKey) / 32.f));
}

bool masInputData_Alloc()
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
		masInputUser* User = &GInputData.InputUsers[i];
		uint64_t      UserEventDataSize = (sizeof(masKeyBuf) + (sizeof(uint32_t) * UsersEventUint32[i]));
		uint64_t      UserAxisDataSize = (sizeof(masKeyBuf) + (sizeof(uint32_t) * UsersAxisUint32[i]));

		// Setup User Input Event Key Buf
		User->EventKeyBuf = MAS_ADDR_FROM(masKeyBuf, Mem, UserInputDataOffset);
		User->EventKeyBuf->BitPerKey = 4;
		User->EventKeyBuf->Uint32Count = UsersEventUint32[i];
		User->EventKeyBuf->Uint32Buf = MAS_ADDR_FROM(uint32_t, User->EventKeyBuf, sizeof(masKeyBuf));

		// Setup User Input Axis Key Buf
		User->AxisKeyBuf = MAS_ADDR_FROM(masKeyBuf, User->EventKeyBuf, UserEventDataSize);
		User->AxisKeyBuf->BitPerKey = 1;
		User->AxisKeyBuf->Uint32Count = UsersAxisUint32[i];
		User->AxisKeyBuf->Uint32Buf = MAS_ADDR_FROM(uint32_t, User->AxisKeyBuf, sizeof(masKeyBuf));

		UserInputDataOffset += (UserEventDataSize + UserAxisDataSize);
	}

	GInputData.InputUsers[EInputUser_0].bActive = true;
	GInputData.InputUsers[EInputUser_1].bActive = false;
	GInputData.InputUsers[EInputUser_2].bActive = false;
	GInputData.InputUsers[EInputUser_3].bActive = false;

	return true;
}

void masInputData_DeAlloc()
{
	::free(GInputData.InputUsers[EInputUser_0].EventKeyBuf);
	for (int32_t i = 0; i < EInputUser_Count; ++i)
		GInputData.InputUsers[i].EventKeyBuf = GInputData.InputUsers[i].AxisKeyBuf = NULL;
}


/*********************************************************************************
*
/*********************************************************************************/
static void masInputDataInternal_SetKeyState(masKeyBuf* KeyBuf, masEKey Key, uint32_t KeyState)
{
	uint32_t BytePos = Key / 32;
	uint32_t BitPos = ((Key - 1) * KeyBuf->BitPerKey) % 32; // need to check the calculation

	// Build Mask to not affect other bits around our target bits
	uint32_t Mask = 0;
	for (int32_t i = 0; i < KeyBuf->BitPerKey; ++i)
		Mask |= (1 << i);
	Mask <<= BitPos;
	Mask = ~Mask;

	KeyBuf->Uint32Buf[BytePos] &= Mask;                 // Set Target Bits to Zero
	KeyBuf->Uint32Buf[BytePos] |= (KeyState << BitPos); // Set Target Bits to KeyState
}

void masInputData_SetKeyState(masEInputUser InputUser, masEKey Key, uint32_t KeyState)
{
	if (!masInputData_IsKeyValid(InputUser, Key))
		return;

	masKeyBuf *EventKeyBuf = GInputData.InputUsers[InputUser].EventKeyBuf;
	masKeyBuf *AxisKeyBuf  = GInputData.InputUsers[InputUser].AxisKeyBuf;

	masInputDataInternal_SetKeyState(EventKeyBuf, Key, KeyState);
	masInputDataInternal_SetKeyState(AxisKeyBuf,  Key, (KeyState == EKeyState_Release) ? 0 : 1);
}

masEKeyState masInputData_GetKeyState(masEInputUser InputUser, masEKey Key)
{
	if (!masInputData_IsKeyValid(InputUser, Key))
		return EKeyState_None;

	masKeyBuf* KeyBuf = GInputData.InputUsers[InputUser].EventKeyBuf;

	uint32_t BytePos = Key / 32;
	uint32_t BitPos = ((Key - 1) * KeyBuf->BitPerKey) % 32; // need to check the calculation

	// Build Mask to get only the bits belong to our Key
	uint32_t Mask = 0;
	for (int32_t i = 0; i < KeyBuf->BitPerKey; ++i)
		Mask |= (1 << i);

	uint32_t Data = KeyBuf->Uint32Buf[BytePos];
	Data >>= BitPos;
	uint32_t KeyState = (Data & Mask);

	return (masEKeyState)KeyState;
}

bool masInputData_IsKeyModActive(uint16_t UserKeyMod)
{
	uint16_t ConvUserKeyMod = UserKeyMod; // to insert alt, ctrl, shift if UserKeyMod only have the extended
	uint16_t CtrlMask = EKeyMod_LCtrl | EKeyMod_RCtrl;
	uint16_t ShiftMask = EKeyMod_LShift | EKeyMod_RShift;
	uint16_t AltMask = EKeyMod_LAlt | EKeyMod_RAlt;

	if (ConvUserKeyMod & CtrlMask)
		ConvUserKeyMod |= EKeyMod_Ctrl;
	else if (ConvUserKeyMod == EKeyMod_Ctrl)
		ConvUserKeyMod |= (EKeyMod_LCtrl | EKeyMod_RCtrl); // need to know if lctrl or rctrl was pressed

	if (ConvUserKeyMod & ShiftMask)
		ConvUserKeyMod |= EKeyMod_Shift;
	else if (ConvUserKeyMod == EKeyMod_Shift)
		ConvUserKeyMod |= (EKeyMod_LShift | EKeyMod_RShift);

	if (ConvUserKeyMod & AltMask)
		ConvUserKeyMod |= EKeyMod_Alt;
	else if (ConvUserKeyMod == EKeyMod_Alt)
		ConvUserKeyMod |= (EKeyMod_LAlt | EKeyMod_RAlt);

	// for making sure that bits are active event if GKeyMod has other keys active
	// here we only check for the bits we need
	if ((GInputData.KeyMod & UserKeyMod) == UserKeyMod)
	{
		// since our bits above are active we active the non-extended modkey in ConvUserKeyMod since that what we do in GKeyMod
		// and here where we check for exact equality
		if (GInputData.KeyMod == ConvUserKeyMod)
			return true;
	}

	return false;
}

bool masInputData_IsKeyActive(masEInputUser InputUser, masEKey Key)
{
	if (!masInputData_IsKeyValid(InputUser, Key))
		return EKeyState_None;

	masKeyBuf* KeyBuf = GInputData.InputUsers[InputUser].AxisKeyBuf;

	uint32_t BytePos = Key / 32;
	uint32_t BitPos = ((Key - 1) * KeyBuf->BitPerKey) % 32; // need to check the calculation

	// Build Mask to get only the bits belong to our Key
	uint32_t Mask = 0;
	for (int32_t i = 0; i < KeyBuf->BitPerKey; ++i)
		Mask |= (1 << i);

	uint32_t Data = KeyBuf->Uint32Buf[BytePos];
	Data >>= BitPos;
	bool IsActive = (Data & Mask);

	return IsActive;
}


/********************************************************************
*
*********************************************************************/
void masInputData_SetUserAxisValue(masEInputUser InputUser, masEKey Key, float AxisValue)
{
	if (!masInputData_IsKeyValid(InputUser, Key))
		return;

	masInputUser* User = &GInputData.InputUsers[InputUser];
	switch (Key)
	{
	case EKey_LAnalogUp:
	case EKey_LAnalogDown:
		User->Axes.LAnalogUpDown = AxisValue;
		break;

	case EKey_LAnalogLeft:
	case EKey_LAnalogRight:
		User->Axes.LAnalogRightLeft = AxisValue;
		break;

	case EKey_RAnalogUp:
	case EKey_RAnalogDown:
		User->Axes.RAnalogUpDown = AxisValue;
		break;

	case EKey_RAnalogLeft:
	case EKey_RAnalogRight:
		User->Axes.RAnalogRightLeft = AxisValue;
		break;

	case EKey_L2:
		User->Axes.LTrigger = AxisValue;
		break;

	case EKey_R2:
		User->Axes.RTrigger = AxisValue;
		break;
	}
}
float masInputData_GetUserAxisValue(masEInputUser InputUser, masEKey Key)
{
	if (!masInputData_IsKeyValid(InputUser, Key))
		return 0.0f;

	masInputUser* User = &GInputData.InputUsers[InputUser];
	switch (Key)
	{
	case EKey_LAnalogUp:    
	case EKey_LAnalogDown:
		return User->Axes.LAnalogUpDown; 

	case EKey_LAnalogLeft:	
	case EKey_LAnalogRight:	
		return User->Axes.LAnalogRightLeft;

	case EKey_RAnalogUp:	
	case EKey_RAnalogDown:	
		return User->Axes.RAnalogUpDown;

	case EKey_RAnalogLeft:	
	case EKey_RAnalogRight:	
		return User->Axes.RAnalogRightLeft;

	case EKey_L2:
		return User->Axes.LTrigger;

	case EKey_R2:
		return User->Axes.RTrigger;
	}

	return 0.0f;
}