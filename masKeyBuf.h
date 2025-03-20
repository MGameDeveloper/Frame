#pragma once

/*********************************************************************************************************
*
*
**********************************************************************************************************/
#include <stdlib.h>
#include <math.h>

#define masAddrFrom(type, ptr, offset) (type*)(((uint8_t*)ptr) + offset)

struct masKeyBuf
{
	uint32_t* Uint32Buf;
	uint8_t   Uint32Count;
	uint8_t   BitPerKey;
};
masKeyBuf* masKeyBuf_Create(uint32_t KeyCount, uint8_t BitPerKey)
{
	uint32_t  Uint32Count = (uint32_t)::ceilf((KeyCount * BitPerKey) / 32.f);
	uint64_t  MemSize = sizeof(masKeyBuf) + (sizeof(uint32_t) * Uint32Count);
	masKeyBuf* KeyBuf = (masKeyBuf*)::malloc(MemSize);
	if (!KeyBuf)
		return NULL;
	else
		::memset(KeyBuf, 0, MemSize);

	KeyBuf->Uint32Buf = masAddrFrom(uint32_t, KeyBuf, sizeof(masKeyBuf));
	KeyBuf->Uint32Count = Uint32Count;
	KeyBuf->BitPerKey = BitPerKey;

	return KeyBuf;
}

void masKeyBuf_Destroy(masKeyBuf** KeyBuf)
{
	if (!KeyBuf || !(*KeyBuf))
		return;

	::free(*KeyBuf);
	*KeyBuf = NULL;
}

masEKeyState masKeyBuf_GetState(masKeyBuf* KeyBuf, uint32_t Key)
{
	uint8_t BytePos = Key / 8;
	uint8_t BitPos = ((Key - 1) % 8) * 4;

	return (masEKeyState)((KeyBuf->Uint32Buf[BytePos] >> BitPos) & 0x0F);
}

void masKeyBuf_SetState(masKeyBuf* KeyBuf, uint32_t Key, uint8_t State)
{
	uint8_t BytePos = Key / 8;
	uint8_t BitPos = ((Key - 1) % 8) * 4;
	uint32_t Mask = ~(State << BitPos);

	KeyBuf->Uint32Buf[BytePos] &= Mask;
	KeyBuf->Uint32Buf[BytePos] |= (State << BitPos);
}

void masKeyBuf_Reset(masKeyBuf* KeyBuf)
{
	for (uint32_t Key = 0; Key < 256; ++Key)
	{
		masEKeyState KeyState = masKeyBuf_GetState(KeyBuf, Key);
		if (KeyState == EKeyState_Release || KeyState == EKeyState_DoubleClick)
		{
			masKeyBuf_SetState(KeyBuf, Key, EKeyState_NoState);
		}
	}
}


masKeyBuf* EventBuf[4] = {};
masKeyBuf* AxisBuf[4] = {};
void masKeyBuf_Main(int argc, const char** argv)
{
	EventBuf[0] = masKeyBuf_Create(256, 2);
	EventBuf[1] = masKeyBuf_Create(24, 2);
	EventBuf[2] = masKeyBuf_Create(24, 2);
	EventBuf[3] = masKeyBuf_Create(24, 2);

	AxisBuf[0] = masKeyBuf_Create(256, 1);
	AxisBuf[1] = masKeyBuf_Create(24, 1);
	AxisBuf[2] = masKeyBuf_Create(24, 1);
	AxisBuf[3] = masKeyBuf_Create(24, 1);

	uint32_t Uint32Count = 0;
	for (int32_t i = 0; i < 4; ++i)
		Uint32Count += (EventBuf[i]->Uint32Count + AxisBuf[i]->Uint32Count);

	printf("INPUT_UINT32_COUNT: %u\n", Uint32Count);
}

void masInput_OnKey(masEInputUser InputUser, masEKey Key, masEKeyState KeyState)
{
	if (InputUser > EInputUser_0 && (Key > EKey_GamepadCount || Key <= EKey_Unknown))
		return;

	if (InputUser == EInputUser_0 && (Key > EKey_Count || Key <= EKey_Unknown))
		return;

	masKeyBuf_SetState(AxisBuf[InputUser], Key, (KeyState > EKeyState_Release) ? 1 : 0);
	masKeyBuf_SetState(EventBuf[InputUser], Key, KeyState);
}


int main(int argc, const char** argv)
{
	masKeyBuf_Main(0, NULL);
	return 0;
}