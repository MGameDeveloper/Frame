#include "Input.h"
#include <string.h>
#include <stdio.h>

enum InputKeyState
{
	KEY_STATE_UNKNOWN,
	KEY_STATE_PRESS,
	KEY_STATE_RELEASE,
	KEY_STATE_DOUBLE_CLICK,
	KEY_STATE_REPEAT,
};

struct InputData
{
	float MousePosX;
	float MousePosY;

	struct
	{
		InputMouse Current;
		InputMouse Last;
	} MouseState;
};

static InputData GInputData = {};


void Input_Reset()
{
	/*
	* CAPTURE FRAME INPUT STATE TO RESET IT FOR THE NEXT FRAME
	*/
	int MOUSE_DATA_SIZE = sizeof(InputMouse);
	memcpy_s(&GInputData.MouseState.Last, MOUSE_DATA_SIZE, &GInputData.MouseState.Current, MOUSE_DATA_SIZE);
	memset(&GInputData.MouseState.Current, 0, MOUSE_DATA_SIZE);
	GInputData.MouseState.Current.PosX = GInputData.MousePosX;
	GInputData.MouseState.Current.PosY = GInputData.MousePosY;
}

const InputMouse* Input_GetMouse()
{
	return &GInputData.MouseState.Current;
}

/*****************************************************************************
*
******************************************************************************/
#include <Windows.h>
#include <windowsx.h>
static InputKeyID Input_KeyFromWin32(WPARAM KeyCode)
{
	switch (KeyCode)
	{
	case 0x41: return KEY_A;
	case 0x42: return KEY_B;
	case 0x43: return KEY_C;
	case 0x44: return KEY_D;
	case 0x45: return KEY_E;
	case 0x46: return KEY_F;
	case 0x47: return KEY_G;
	case 0x48: return KEY_H;
	case 0x49: return KEY_I;
	case 0x4A: return KEY_J;
	case 0x4B: return KEY_K;
	case 0x4C: return KEY_L;
	case 0x4D: return KEY_M;
	case 0x4E: return KEY_N;
	case 0x4F: return KEY_O;
	case 0x50: return KEY_P;
	case 0x51: return KEY_Q;
	case 0x52: return KEY_R;
	case 0x53: return KEY_S;
	case 0x54: return KEY_T;
	case 0x55: return KEY_U;
	case 0x56: return KEY_V;
	case 0x57: return KEY_W;
	case 0x58: return KEY_X;
	case 0x59: return KEY_Y;
	case 0x5A: return KEY_Z;
	}

	return KEY_UNKNOWN;
}


// extern this function in your win32 cpp and call it inside main window proc
LRESULT Input_Win32Proc(HWND Hwnd, UINT Msg, WPARAM Wparam, LPARAM Lparam)
{
	switch (Msg)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	{
		WORD          vkCode    = LOWORD(Wparam);
		WORD          IsPressed = LOWORD(Lparam);
		InputKeyID    Key       = Input_KeyFromWin32(vkCode);
		//InputKeyState LastState = GInputData.LastKeyState[Key];
		//
		//switch (LastState)
		//{
		//case KEY_STATE_REPEAT:
		//case KEY_STATE_PRESS:
		//	if (IsPressed)
		//		GInputData.CurrentKeyState[Key] = KEY_STATE_REPEAT;
		//	else
		//		GInputData.CurrentKeyState[Key] = KEY_STATE_RELEASE;
		//	break;
		//
		//case KEY_STATE_RELEASE:
		//	if(IsPressed)
		//		GInputData.CurrentKeyState[Key] = KEY_STATE_PRESS;
		//	else
		//		GInputData.CurrentKeyState[Key] = KEY_STATE_RELEASE;
		//	break;
		//}
	}
	break;

	case WM_MOUSEMOVE:
		GInputData.MousePosX = (float)GET_X_LPARAM(Lparam);
		GInputData.MousePosY = (float)GET_Y_LPARAM(Lparam);
		break;

	case WM_MOUSEWHEEL:
	{
		short Delta = GET_WHEEL_DELTA_WPARAM(Wparam);
		if (Delta > 0)
			GInputData.MouseState.Current.ScrollUp = true;
		else if (Delta < 0)
			GInputData.MouseState.Current.ScrollDown = true;
	}
		break;

	/*
	*/
	case WM_LBUTTONDOWN: GInputData.MouseState.Current.Left  .Press = true; break;
	case WM_RBUTTONDOWN: GInputData.MouseState.Current.Right .Press = true; break;
	case WM_MBUTTONDOWN: GInputData.MouseState.Current.Middle.Press = true; break;

	case WM_LBUTTONUP: GInputData.MouseState.Current.Left  .Release = true; break;
	case WM_RBUTTONUP: GInputData.MouseState.Current.Right .Release = true; break;
	case WM_MBUTTONUP: GInputData.MouseState.Current.Middle.Release = true; break;

	case WM_LBUTTONDBLCLK: GInputData.MouseState.Current.Left  .DoubleClick = true; break;
	case WM_RBUTTONDBLCLK: GInputData.MouseState.Current.Right .DoubleClick = true; break;
	case WM_MBUTTONDBLCLK: GInputData.MouseState.Current.Middle.DoubleClick = true; break;
	}

	return 0;
}