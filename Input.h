#pragma once

#include "Input/masInputCommon.h"


bool  masInput_Init();
void  masInput_DeInit();
void  masInput_Reset();
void  masInput_Update();

bool  masInput_OnSingleAxisKey(masEInputUser InputUser, masEKey Key1, masEKey Key2 = EKey_Unknown, masEKey Key3 = EKey_Unknown, masEKey Key4 = EKey_Unknown, masEKey Keys5 = EKey_Unknown, masEKey Key6 = EKey_Unknown, masEKey Key7 = EKey_Unknown);
bool  masInput_OnMultiAxisKey (masEInputUser InputUser,masEKey Key1, masEKey Key2, masEKey Key3 = EKey_Unknown,masEKey Key4 = EKey_Unknown, masEKey Keys5 = EKey_Unknown, masEKey Key6 = EKey_Unknown, masEKey Key7 = EKey_Unknown);

bool  masInput_OnSingleEventKey(masEInputUser InputUser, masEKeyState KeyState, masEKey Key1, masEKey Key2 = EKey_Unknown, masEKey Key3 = EKey_Unknown, masEKey Key4 = EKey_Unknown, masEKey Keys5 = EKey_Unknown, masEKey Key6 = EKey_Unknown, masEKey Key7 = EKey_Unknown);
bool  masInput_OnMultiEventKey (masEInputUser InputUser, masEKeyState KeyState, masEKey Key1, masEKey Key2, masEKey Key3 = EKey_Unknown, masEKey Key4 = EKey_Unknown, masEKey Keys5 = EKey_Unknown, masEKey Key6 = EKey_Unknown, masEKey Key7 = EKey_Unknown);


/**************************************************************************************************************
* NEW API
***************************************************************************************************************/
struct masInputKey 
{
	masEInputUser InputUser;
	masEKey       Keys[7];
	uint32_t      Modifiers;
	uint8_t       KeyCount;
	bool          bAllKeysMustBeActive;
};

masInputKey masInput_CreateAxisKey  (masEInputUser InputUser, bool bAllKeysMustBeActive, masEKeyModifier Mods, masEKey Key1, masEKey Key2 = EKey_Unknown, masEKey Key3 = EKey_Unknown, masEKey Key4 = EKey_Unknown, masEKey Keys5 = EKey_Unknown, masEKey Key6 = EKey_Unknown, masEKey Key7 = EKey_Unknown);
masInputKey masInput_CreateActionKey(masEInputUser InputUser, bool bAllKeysMustBeActive, masEKeyModifier Mods, masEKeyState KeyState, masEKey Key1, masEKey Key2 = EKey_Unknown, masEKey Key3 = EKey_Unknown, masEKey Key4 = EKey_Unknown, masEKey Keys5 = EKey_Unknown, masEKey Key6 = EKey_Unknown, masEKey Key7 = EKey_Unknown);
bool masInput_OnKey(masInputKey InputKey);


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

