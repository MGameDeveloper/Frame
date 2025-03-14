#pragma once

#include "masInputCommon.h"
#include <stdio.h>

struct masInputC_KeyInfo
{
	float           Scaler;
	masEKey         Keys[8];
	masEKeyModifier KeyMods;
};

struct masInputC_Key
{
	masInputC_KeyInfo *Keys;
	uint8_t            KeysInfoCount;
	uint8_t            bContinues    : 1;
	uint8_t            bAnyKeyActive : 1;
};

#define MAS_ARRAY_SIZE(ARRAY)                 (sizeof(ARRAY) / sizeof(ARRAY[0]))
#define MAS_KEY_INFO(KeyMods, AxisValue, ...) { AxisValue, {__VA_ARGS__}, KeyMods }
#define MAS_DECLARE_AXIS_KEY(AxisName, AllKeyMustBeActive, ...)\
    masInputC_KeyInfo KeyInfoList_##AxisName[] = { __VA_ARGS__ };\
    masInputC_Key     AxisName                 = { KeyInfoList_##AxisName, MAS_ARRAY_SIZE(KeyInfoList_##AxisName), true, AllKeyMustBeActive }

struct masInptuC_ActiveKeyData
{
	float AxisValue;
};

struct masInputC_ActiveKeyList
{
	masInptuC_ActiveKeyData *Keys;
	uint8_t Count = 0.f;
};

float masInputC_AccumulateAxisValue(masInputC_ActiveKeyList* ActiveKeyList)
{
	// Accumulate AxisValue
	float AxisValue = 0.f;
	for (int32_t i = 0; i < ActiveKeyList->Count; ++i)
		AxisValue += ActiveKeyList->Keys[i].AxisValue;

	// Clamp Axis Value
	if (AxisValue > 1.f)
		AxisValue = 1.f;
	else if (AxisValue < -1.f)
		AxisValue = -1.f;
}

masInputC_ActiveKeyList* masInputC_OnKey(masEInputUser InputUser, masInputC_Key* InputKey);

int fu()
{
	MAS_DECLARE_AXIS_KEY(Axis_RotateCamera, true,
		MAS_KEY_INFO(EKeyMod_Alt,  EKey_MouseLeft),
		MAS_KEY_INFO(EKeyMod_None, EKey_ArrowLeft,  EKey_RAnalogLeft),
		MAS_KEY_INFO(EKeyMod_None, EKey_ArrowRight, EKey_RAnalogRight),
		MAS_KEY_INFO(EKeyMod_None, Ekey_ArrowUp,    EKey_RAnalogUp),
		MAS_KEY_INFO(EKeyMod_None, Ekey_ArrowDown,  EKey_RAnalogDown));
	if (masInputC_ActiveKeyList* List = masInputC_OnKey(EInputUser_0, &Axis_RotateCamera))
	{
		float AxisValue = masInputC_AccumulateAxisValue(List);
		printf("ROTATE_CAMERA[ %.2f ]\n", AxisValue);
	}
}

