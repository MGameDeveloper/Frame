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

enum masEKeyMod
{
	EKeyMod_None,

	EKeyMod_Ctrl   = ( 1 << 0),
	EKeyMod_LCtrl  = ( 1 << 1),
	EKeyMod_RCtrl  = ( 1 << 2),
	EKeyMod_Shift  = ( 1 << 3),
	EKeyMod_LShift = ( 1 << 4),
	EKeyMod_RShift = ( 1 << 5),
	EKeyMod_Alt    = ( 1 << 6),
	EKeyMod_LAlt   = ( 1 << 7),
	EKeyMod_RAlt   = ( 1 << 8),

	EKeyMod_Count = 9
};


enum masEKeyState
{
	EKeyState_None        = 0,
	EKeyState_Release     = (1 << 0),
	EKeyState_Press       = (1 << 1),
	EKeyState_Repeat      = (1 << 2),
	EKeyState_DoubleClick = (1 << 3),

	EKeyState_Count = 4
};

enum masEInputUser
{
	EInputUser_0,
	EInputUser_1,
	EInputUser_2,
	EInputUser_3,

	EInputUser_Count
};


typedef uint8_t  EKeyState_;
typedef uint16_t EKeyMod_;