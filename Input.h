#pragma once

#include <stdint.h>

enum masEKey
{
	EKey_Unknown = 0,

	/*
	* GAMEPAD BUTTONS & AXES & TRIGGERS
	*/
	EKey_Square,
	EKey_Cross,
	EKey_Circle,
	EKey_Triangle,
	EKey_Start,
	EKey_Select,
	EKey_DpadUp,
	EKey_DpadDown,
	EKey_DpadRight,
	EKey_DpadLeft,
	EKey_L1,
	EKey_L2,
	EKey_L3,
	EKey_R1,
	EKey_R2,
	EKey_R3,
	EKey_LAnalogUp,
	EKey_LAnalogDown,
	EKey_LAnalogLeft,
	EKey_LAnalogRight,
	EKey_RAnalogUp,
	EKey_RAnalogDown,
	EKey_RAnalogLeft,
	EKey_RAnalogRight,
	EKey_GamepadCount = EKey_RAnalogRight,

	EKey_Anykey,

	EKey_A,
	EKey_B,
	EKey_C,
	EKey_D,
	EKey_E,
	EKey_F,
	EKey_G,
	EKey_H,
	EKey_I,
	EKey_J,
	EKey_K,
	EKey_L,
	EKey_M,
	EKey_N,
	EKey_O,
	EKey_P,
	EKey_Q,
	EKey_R,
	EKey_S,
	EKey_T,
	EKey_U,
	EKey_V,
	EKey_W,
	EKey_X,
	EKey_Y,
	EKey_Z,

	EKey_F1,
	EKey_F2,
	EKey_F3,
	EKey_F4,
	EKey_F5,
	EKey_F6,
	EKey_F7,
	EKey_F8,
	EKey_F9,
	EKey_F10,
	EKey_F11,
	EKey_F12,

	EKey_NumLock,
	EKey_Numpad0,
	EKey_Numpad1,
	EKey_Numpad2,
	EKey_Numpad3,
	EKey_Numpad4,
	EKey_Numpad5,
	EKey_Numpad6,
	EKey_Numpad7,
	EKey_Numpad8,
	EKey_Numpad9,

	EKey_Num0,
	EKey_Num1,
	EKey_Num2,
	EKey_Num3,
	EKey_Num4,
	EKey_Num5,
	EKey_Num6,
	EKey_Num7,
	EKey_Num8,
	EKey_Num9,

	EKey_Decimal,
	EKey_PageUp,
	EKey_PageDown,
	EKey_Space,
	EKey_Enter,
	EKey_Backspace,
	EKey_Tab,
	EKey_PrintScreen,
	EKey_Insert,
	EKey_Delete,
	EKey_Divide,
	EKey_Multipy,
	EKey_Subtract,
	EKey_Addition,
	EKey_Home,
	EKey_End,
	EKey_Escape,
	EKey_CapsLock,

	EKey_ArrowUp,
	EKey_ArrowDown,
	EKey_ArrowLeft,
	EKey_ArrowRight,

	EKey_Ctrl,
	EKey_Shift,
	EKey_Alt,
	EKey_LCtrl,
	EKey_RCtrl,
	EKey_LShift,
	EKey_RShift,
	EKey_LAlt,
	EKey_RAlt,

	/*
	* MOUSE BUTTONS
	*/
	EKey_MouseWheelUp,
	EKey_MouseWheelDown,
	EKey_MouseLeft,
	EKey_MouseRight,
	EKey_MouseMiddle,
	EKey_MouseX1,
	EKey_MouseX2,

	EKey_Count
};

enum masEKeyState
{
	EKeyState_Unknown     = 0,
	EKeyState_Release     = (1 << 0),
	EKeyState_Press       = (1 << 1),
	EKeyState_Repeat      = (1 << 2),
	EKeyState_DoubleClick = (1 << 3),
};

enum masEInputUser
{
	EInputUser_0,
	EInputUser_1,
	EInputUser_2,
	EInputUser_3,

	EInputUser_Count
};


bool  masInput_Init();
void  masInput_DeInit();
void  masInput_Process();
void  masInput_Reset();

bool  masInput_CheckKey(masEInputUser InputUser, masEKey Key1);
bool  masInput_CheckKey(masEInputUser InputUser, masEKey Key1, masEKey Key2);
bool  masInput_CheckKey(masEInputUser InputUser, masEKey Key1, masEKey Key2, masEKey Key3);
bool  masInput_CheckKey(masEInputUser InputUser, masEKey Key1, masEKey Key2, masEKey Key3, masEKey Key4);

float masInput_AxisValue(masEInputUser InputUser, masEKey Key);


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