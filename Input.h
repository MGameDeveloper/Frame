#pragma once

#include <stdint.h>

enum InputKeyID
{
	KEY_UNKNOWN,
	KEY_ANYKEY,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,

	KEY_COUNT
};

struct InputMouse
{
	float PosX;
	float PosY;

	uint8_t ScrollUp : 1;
	uint8_t ScrollDown : 1;
	struct
	{
		uint8_t Press : 1;
		uint8_t Release : 1;
		uint8_t DoubleClick : 1;
	} Left, Right, Middle;
};

struct InputKeyboard
{
	wchar_t Unicode;

	struct
	{
		uint8_t Press : 1;
		uint8_t Release : 1;
		uint8_t Repeat : 1;
	} Keys[KEY_COUNT];
};

//struct InputData
//{
//	union
//	{
//		uint8_t Flags;
//		struct
//		{
//			uint8_t LeftCtrl : 1;
//			uint8_t LeftShift : 1;
//			uint8_t LeftAlt : 1;
//			uint8_t RightCtrl : 1;
//			uint8_t RightShift : 1;
//			uint8_t RightAlt : 1;
//		};
//	} Modifier;
//};

void  Input_Reset();
const InputMouse* Input_GetMouse();


