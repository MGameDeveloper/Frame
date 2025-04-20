#pragma once

#include "masInputCommon.h"

struct masKeyBuf
{
	uint32_t* Uint32Buf;
	uint32_t  Uint32Count;
	uint8_t   BitPerKey;
};

struct masInputUser
{
	masKeyBuf* EventKeyBuf;
	masKeyBuf* AxisKeyBuf;

	struct
	{
		float LAnalogUpDown;
		float LAnalogRightLeft;
		float RAnalogUpDown;
		float RAnalogRightLeft;
		float LTrigger;
		float RTrigger;
	}Axes;

	struct
	{
		float LAnalogY;
		float LAnalogX;
		float RAnalogY;
		float RAnalogX;
		float LTrigger;
		float RTrigger;
	}DeadZone;

	bool       bActive;
};

struct masInputData
{
	masInputUser InputUsers[EInputUser_Count];
	uint16_t     KeyMod = {};
	float        MouseDirX = {};
	float        MouseDirY = {};
	float        MousePosX = {};
	float        MousePosY = {};
};



/********************************************
*
*********************************************/
bool masInputData_Alloc();
void masInputData_DeAlloc();



/********************************************
*
*********************************************/
void         masInputData_SetKeyState     (masEInputUser InputUser, masEKey Key, uint32_t KeyState);
masEKeyState masInputData_GetKeyState     (masEInputUser InputUser, masEKey Key);
bool         masInputData_IsKeyModActive  (uint16_t UserKeyMod);
bool         masInputData_IsKeyActive     (masEInputUser InputUser, masEKey Key);



/********************************************
*
*********************************************/
bool         masInputData_IsKeyValid      (masEInputUser InputUser, masEKey Key);
void         masInputData_SetUserAxisValue(masEInputUser InputUser, masEKey Key, float AxisValue);
float        masInputData_GetUserAxisValue(masEInputUser InputUser, masEKey Key);



/********************************************
*
*********************************************/
extern masInputData GInputData;