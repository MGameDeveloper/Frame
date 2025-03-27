#pragma once

#include "masInputCommon.h"


bool  masInput_Init();
void  masInput_DeInit();
void  masInput_Update();

const char* masInput_KeyName   (masEKey      Key);
const char* masInput_KeyModName(masEKeyMod   KeyMod);
const char* masInput_KeyState  (masEKeyState KeyState);

/***********************************************************************************************************************************************
* 
************************************************************************************************************************************************/
struct masAxisKeyDesc;
struct masAxisKey;
struct masVec3
{
	float x, y, z;
};

masAxisKeyDesc* masInput_DescribeAxisKey(bool MustAllBeActive, float AxisX, float AxisY, float AxisZ, uint16_t KeyMod, const char* KeyListFmt, ...);
masAxisKey*     masInput_CreateAxisKey  (const char* Name, masAxisKeyDesc** AxisKeyDescList, uint32_t ListCount);
masVec3*        masInput_OnAxisKey      (masEInputUser InputUser, masAxisKey* AxisKey);

#define MAS_DEF_AXIS_KEY(ALL_KEY_MUST_ACTIVE, AXIS_X, AXIS_Y, AXIS_Z, KEY_MOD, ...) masInput_DescribeAxisKey(ALL_KEY_MUST_ACTIVE, AXIS_X, AXIS_Y, AXIS_Z, KEY_MOD, #__VA_ARGS__##"\n", __VA_ARGS__)
#define MAS_DECLARE_AXIS_KEY(NAME, ...)\
    masAxisKeyDesc *AxisKeyDesc_##NAME[] = { __VA_ARGS__ };\
    masAxisKey     *NAME                 = masInput_CreateAxisKey(#NAME, AxisKeyDesc_##NAME, sizeof(AxisKeyDesc_##NAME)/sizeof(AxisKeyDesc_##NAME[0]))




/***********************************************************************************************************************************************
*
************************************************************************************************************************************************/
struct masEventKeyDesc;
struct masEventKey;

masEventKeyDesc* masInput_DescribeEventKey(bool MustAllBeActive, uint16_t KeyMod, uint8_t KeyState, const char* KeyListFmt, ...);
masEventKey*     masInput_CreateEventKey(const char* Name, masEventKeyDesc** EventKeyDescList, uint32_t ListCount);
bool             masInput_OnEventKey(masEInputUser InputUser, masEventKey* EventKey);

#define MAS_DEF_EVENT_KEY(ALL_KEY_MUST_ACTIVE, KEY_MOD, KEY_STATE, ...) masInput_DescribeEventKey(ALL_KEY_MUST_ACTIVE, KEY_MOD, KEY_STATE, #__VA_ARGS__##"\n", __VA_ARGS__)
#define MAS_DECLARE_EVENT_KEY(NAME, ...)\
    masEventKeyDesc* EventKeyDesc_##NAME[] = { __VA_ARGS__ };\
    masEventKey*     NAME                  = masInput_CreateEventKey(#NAME, EventKeyDesc_##NAME, sizeof(EventKeyDesc_##NAME)/sizeof(EventKeyDesc_##NAME[0]))




/***********************************************************************************************************************************************
*
************************************************************************************************************************************************/













































#if 0
typedef void(*masActionInputFunc)();
typedef void(*masAxisInputFunc)(float Scaler);


struct masInputComp;
masInputComp* masInput_CreateInputComp(const char* InputCompName, bool bConsumeInput = false);
void masInput_AddAction(masInputComp* InputComp, masEKey Key, masEKeyState KeyState, masKeyModifier   Modifier, masActionInputFunc Action);
void masInput_AddAxis  (masInputComp* InputComp, masEKey Key, float        Scaler,   masAxisInputFunc Axis);

void masInput_PushInputComp(masEInputUser InputUser, masInputComp* InputComp);
void masInput_PopInputComp(masEInputUser InputUser);
#endif

#if 0
void UsageCase()
{
	masInputComp* AComp = masInput_CreateInputComp("InputComp_A");
	{
		masInput_AddAction(AComp, masInput_ActionKey(EKey_Cross, EKeyState_Press), []() {"Jump";});
		masInput_AddAction(AComp, masInput_ActionKey(EKey_Square, EKeyState_Press), []() {"Reload";});
		masInput_AddAction(AComp, masInput_ActionKey(EKey_Circle, EKeyState_Press), []() {"Crouch";});
		masInput_AddAction(AComp, masInput_ActionKey(EKey_Triangle, EKeyState_Press), []() {"OpenMap";});
		masInput_AddAction(AComp, masInput_ActionKey(EKey_L1, EKeyState_Press), []() {"Aim";});
		masInput_AddAction(AComp, masInput_ActionKey(EKey_R1, EKeyState_Press), []() {"Shoot";});

		masInput_AddAxis(AComp, masInput_AxisKey(EKey_LAnalogUp, 1.f), [](float) {"MoveToFront";});
		masInput_AddAxis(AComp, masInput_AxisKey(EKey_LAnalogDown, -1.f), [](float) {"MoveToBack";});
		masInput_AddAxis(AComp, masInput_AxisKey(EKey_LAnalogRight, 1.f), [](float) {"MoveToRight";});
		masInput_AddAxis(AComp, masInput_AxisKey(EKey_LAnalogLeft, -1.f), [](float) {"MoveToLeft";});
		masInput_AddAxis(AComp, masInput_AxisKey(EKey_RAnalogUp, 1.f), [](float) {"RotateCameraUp";});
		masInput_AddAxis(AComp, masInput_AxisKey(EKey_RAnalogDown, -1.f), [](float) {"RotateCameraDown";});
		masInput_AddAxis(AComp, masInput_AxisKey(EKey_RAnalogRight, 1.f), [](float) {"RotateCameraRight";});
		masInput_AddAxis(AComp, masInput_AxisKey(EKey_RAnalogLeft, -1.f), [](float) {"RotateCameraLeft";});

		masInput_PushInputComp(EInputUser_0, AComp);
	}
}
#endif

