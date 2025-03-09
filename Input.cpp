#include "Input.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#include <stdlib.h>

#pragma comment(lib, "xinput.lib")

struct masKeyInfo
{
	masEKeyState    State;
	masKeyModifier  Modifier;
	float           Scaler;
	float           Time;
};

struct masInputUser
{
	masKeyInfo *Keys;
	bool        bActive;
};

static masInputUser GInputUsers[EInputUser_Count] = {};


static void masXInput_Init();
static void masXInput_Update();

bool masInput_Init()
{
	GInputUsers[EInputUser_0].Keys = (masKeyInfo*)malloc(sizeof(masKeyInfo) * EKey_Count);
	GInputUsers[EInputUser_1].Keys = (masKeyInfo*)malloc(sizeof(masKeyInfo) * EKey_GamepadCount);
	GInputUsers[EInputUser_2].Keys = (masKeyInfo*)malloc(sizeof(masKeyInfo) * EKey_GamepadCount);
	GInputUsers[EInputUser_3].Keys = (masKeyInfo*)malloc(sizeof(masKeyInfo) * EKey_GamepadCount);

	if (!GInputUsers[EInputUser_0].Keys ||
		!GInputUsers[EInputUser_1].Keys ||
		!GInputUsers[EInputUser_2].Keys ||
		!GInputUsers[EInputUser_3].Keys)
	{
		return false;
	}

	::memset(GInputUsers[EInputUser_0].Keys, 0, sizeof(masKeyInfo) * EKey_Count);
	::memset(GInputUsers[EInputUser_1].Keys, 0, sizeof(masKeyInfo) * EKey_GamepadCount);
	::memset(GInputUsers[EInputUser_2].Keys, 0, sizeof(masKeyInfo) * EKey_GamepadCount);
	::memset(GInputUsers[EInputUser_3].Keys, 0, sizeof(masKeyInfo) * EKey_GamepadCount);

	GInputUsers[EInputUser_0].bActive = true;
	GInputUsers[EInputUser_1].bActive = false;
	GInputUsers[EInputUser_2].bActive = false;
	GInputUsers[EInputUser_3].bActive = false;

	masXInput_Init();

	return true;
}

void masInput_DeInit()
{
	for (int32_t UserIdx = 0; UserIdx < EInputUser_Count; ++UserIdx)
	{
		::free(GInputUsers[UserIdx].Keys);
		GInputUsers[UserIdx].Keys = NULL;
	}
}

void masInput_OnKey(masEInputUser InputUser, masEKey Key, masEKeyState KeyState)
{
	if (InputUser < EInputUser_0 || InputUser > EInputUser_3)
		return;
	if (Key <= EKey_Unknown || Key >= EKey_Count)
		return;
	if (InputUser != EInputUser_0 && Key >= EKey_GamepadCount)
		return;

	masKeyInfo* KeyInfo = &GInputUsers[InputUser].Keys[Key];
	KeyInfo->State      = KeyState;
	KeyInfo->Modifier   = {};
	KeyInfo->Time       = 0.f;
}

void masInput_OnAxis(masEInputUser InputUser, masEKey Key, float Scaler)
{
	if (InputUser < EInputUser_0 || InputUser > EInputUser_3)
		return;
	if (Key <= EKey_Unknown || Key >= EKey_Count)
		return;
	if (InputUser != EInputUser_0 && Key >= EKey_GamepadCount)
		return;

	masKeyInfo* KeyInfo = &GInputUsers[InputUser].Keys[Key];
	KeyInfo->Scaler     = Scaler;
}

bool masInput_IsKey(masEInputUser InputUser, uint32_t Key, uint32_t KeyState)
{
	if (InputUser < EInputUser_0 || InputUser > EInputUser_3)
		return false;
	if (Key <= EKey_Unknown || Key >= EKey_Count)
		return false;
	if (InputUser != EInputUser_0 && Key >= EKey_GamepadCount)
		return false;

	return (GInputUsers[InputUser].Keys[Key].State & KeyState); // IMPLEMENTATION NOT COMPLETE
}

float masInput_AxisValue(masEInputUser InputUser, masEKey Key)
{
	if (InputUser < EInputUser_0 || InputUser > EInputUser_3)
		return 0.f;
	if (Key <= EKey_Unknown || Key >= EKey_Count)
		return 0.f;
	if (InputUser != EInputUser_0 && Key >= EKey_GamepadCount)
		return 0.f;

	if (GInputUsers[InputUser].Keys[Key].State > EKeyState_Release)
		return GInputUsers[InputUser].Keys[Key].Scaler;

	return 0.f;
}

void masInput_Process()
{
	masXInput_Update();
}

void masInput_Reset()
{
	for (int32_t UserIdx = 0; UserIdx < EInputUser_Count; ++UserIdx)
	{
		if (!GInputUsers[UserIdx].bActive)
			continue;

		if (UserIdx == 0)
		{
			for (int32_t ButtonIdx = 0; ButtonIdx < EKey_Count; ++ButtonIdx)
				GInputUsers[UserIdx].Keys[ButtonIdx].State = EKeyState_Unknown;
		}
		else
		{
			for (int32_t ButtonIdx = 0; ButtonIdx < EKey_GamepadCount; ++ButtonIdx)
				GInputUsers[UserIdx].Keys[ButtonIdx].State = EKeyState_Unknown;
		}
	}
}


/******************************************************************************************************
*
* XINPUT CODE
*
*******************************************************************************************************/
#define RepeatInitTime    0.2f
#define RepeatAdvanceTime 0.02f

struct masGamepad
{
	float LAnalogDeadZone;
	float RAnalogDeadZone;
	float LTriggerThreshold;
	float RTriggerThreshold;
	float RepeatTime[EKey_GamepadCount];
	bool  LastState [EKey_GamepadCount];
	bool  bAvailable;
};

static masGamepad GGamepads[EInputUser_Count] = {};

void masXInput_CheckControllersConnection()
{
	for (int32_t UserIdx = 0; UserIdx < EInputUser_Count; ++UserIdx)
	{
		XINPUT_STATE XInputState = {};
		if (::XInputGetState(UserIdx, &XInputState) == ERROR_DEVICE_NOT_CONNECTED)
		{
			GGamepads[UserIdx].bAvailable = false;
			if (UserIdx != EInputUser_0)
				GInputUsers[UserIdx].bActive = false;
		}
		else
			GGamepads[UserIdx].bAvailable = true;
	}
}

void masXInput_Init()
{
	masXInput_CheckControllersConnection();

	for (int32_t UserIdx = 0; UserIdx < EInputUser_Count; ++UserIdx)
	{
		GGamepads[UserIdx].LTriggerThreshold = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
		GGamepads[UserIdx].RTriggerThreshold = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
		GGamepads[UserIdx].LAnalogDeadZone   = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
		GGamepads[UserIdx].RAnalogDeadZone   = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
		for (int32_t ButtonIdx = 0; ButtonIdx < EKey_GamepadCount; ++ButtonIdx)
			GGamepads[UserIdx].RepeatTime[ButtonIdx] = RepeatInitTime;
	}
}

void masXInput_Update()
{
	XINPUT_STATE    XInputState = {};
	XINPUT_GAMEPAD* XGamepad = NULL;
	for (int32_t UserIdx = 0; UserIdx < EInputUser_Count; ++UserIdx)
	{
		masGamepad* Gamepad = &GGamepads[UserIdx];
		if (!Gamepad->bAvailable)
			continue;

		::memset(&XInputState, 0, sizeof(XINPUT_STATE));
		DWORD Ret = ::XInputGetState(UserIdx, &XInputState);
		if (Ret == ERROR_DEVICE_NOT_CONNECTED)
		{
			Gamepad->bAvailable = false;
			continue;
		}
		XGamepad = &XInputState.Gamepad;

		static bool Buttons[EKey_GamepadCount];
		::memset(Buttons, 0, sizeof(bool) * EKey_GamepadCount);

		Buttons[EKey_Square]       = XGamepad->wButtons & XINPUT_GAMEPAD_X;
		Buttons[EKey_Cross]        = XGamepad->wButtons & XINPUT_GAMEPAD_A;
		Buttons[EKey_Circle]       = XGamepad->wButtons & XINPUT_GAMEPAD_B;
		Buttons[EKey_Triangle]     = XGamepad->wButtons & XINPUT_GAMEPAD_Y;
		Buttons[EKey_Start]        = XGamepad->wButtons & XINPUT_GAMEPAD_START;
		Buttons[EKey_Select]       = XGamepad->wButtons & XINPUT_GAMEPAD_BACK;
		Buttons[EKey_DpadUp]       = XGamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
		Buttons[EKey_DpadDown]     = XGamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
		Buttons[EKey_DpadRight]    = XGamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
		Buttons[EKey_DpadLeft]     = XGamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
		Buttons[EKey_L1]           = XGamepad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
		Buttons[EKey_R1]           = XGamepad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
		Buttons[EKey_L3]           = XGamepad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
		Buttons[EKey_R3]           = XGamepad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
		/////////////////////////////////////////////////////////////////////////////////////
		Buttons[EKey_L2]           = XGamepad->bLeftTrigger > Gamepad->LTriggerThreshold;
		Buttons[EKey_R2]           = XGamepad->bRightTrigger > Gamepad->RTriggerThreshold;
		Buttons[EKey_LAnalogUp]    = XGamepad->sThumbLY > Gamepad->LAnalogDeadZone;
		Buttons[EKey_LAnalogDown]  = XGamepad->sThumbLY < Gamepad->LAnalogDeadZone;
		Buttons[EKey_LAnalogRight] = XGamepad->sThumbLX > Gamepad->LAnalogDeadZone;
		Buttons[EKey_LAnalogLeft]  = XGamepad->sThumbLX < Gamepad->LAnalogDeadZone;
		Buttons[EKey_RAnalogUp]    = XGamepad->sThumbRY > Gamepad->RAnalogDeadZone;
		Buttons[EKey_RAnalogDown]  = XGamepad->sThumbRY < Gamepad->RAnalogDeadZone;
		Buttons[EKey_RAnalogRight] = XGamepad->sThumbRX > Gamepad->RAnalogDeadZone;
		Buttons[EKey_RAnalogLeft]  = XGamepad->sThumbRX < Gamepad->RAnalogDeadZone;

		masEInputUser InputUser = (masEInputUser)UserIdx;
		
		/*
		* Dispatch axis keys L2, R2, LAnalog, RAnalog
		*/
		if (Buttons[EKey_L2])           masInput_OnAxis(InputUser, EKey_L2,           XGamepad->bLeftTrigger / 255.f);
		if (Buttons[EKey_R2])           masInput_OnAxis(InputUser, EKey_R2,           XGamepad->bRightTrigger / 255.f);
		if (Buttons[EKey_LAnalogUp])    masInput_OnAxis(InputUser, EKey_LAnalogUp,    XGamepad->sThumbLY /  32767.f);
		if (Buttons[EKey_LAnalogDown])	masInput_OnAxis(InputUser, EKey_LAnalogDown,  XGamepad->sThumbLY / -32768.f);
		if (Buttons[EKey_LAnalogRight]) masInput_OnAxis(InputUser, EKey_LAnalogRight, XGamepad->sThumbLX /  32767.f);
		if (Buttons[EKey_LAnalogLeft])	masInput_OnAxis(InputUser, EKey_LAnalogLeft,  XGamepad->sThumbLX / -32768.f);
		if (Buttons[EKey_LAnalogUp])	masInput_OnAxis(InputUser, EKey_RAnalogUp,    XGamepad->sThumbRY /  32767.f);
		if (Buttons[EKey_LAnalogDown])	masInput_OnAxis(InputUser, EKey_RAnalogDown,  XGamepad->sThumbRY / -32768.f);
		if (Buttons[EKey_LAnalogRight]) masInput_OnAxis(InputUser, EKey_RAnalogRight, XGamepad->sThumbRX /  32767.f);
		if (Buttons[EKey_LAnalogLeft])	masInput_OnAxis(InputUser, EKey_RAnalogLeft,  XGamepad->sThumbRX / -32768.f);


		/*
		* Dispatch Buttons
		*/
		for (int32_t ButtonIdx = 0; ButtonIdx < EKey_GamepadCount; ++ButtonIdx)
		{
			masEKey Key     = (masEKey)ButtonIdx;
			bool CurrState  = Buttons[ButtonIdx];
			bool LastState  = Gamepad->LastState[ButtonIdx];
			bool IsPressed  = !LastState && CurrState;
			bool IsReleased = LastState && !CurrState;
			bool IsRepeated = LastState && CurrState;

			if (IsRepeated)
			{
				if (Gamepad->RepeatTime[ButtonIdx] < Gamepad->RepeatTime[ButtonIdx] + RepeatInitTime)
					Gamepad->RepeatTime[ButtonIdx] += RepeatAdvanceTime;
				else
				{
					masInput_OnKey(InputUser, Key, EKeyState_Repeat);
					Gamepad->RepeatTime[ButtonIdx] += RepeatInitTime;
				}
			}
			else if (IsReleased)
				masInput_OnKey(InputUser, Key, EKeyState_Release);
			else if (IsPressed)
				masInput_OnKey(InputUser, Key, EKeyState_Press);
		}

		::memcpy(Gamepad->LastState, Buttons, sizeof(bool) * EKey_GamepadCount);
	}
}




/******************************************************************************************************
* 
* WIN32 CODE
* 
*******************************************************************************************************/
static masEKey masInput_ConvertWin32Key(WPARAM Wparam);

LRESULT masInput_Win32Proc(HWND Hwnd, UINT Msg, WPARAM Wparam, LPARAM Lparam)
{
	switch (Msg)
	{
	/*
	* KEYBOARD EVENT HANDLE
	*/
	case WM_KEYUP:
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	{
		WORD vkCode        = LOWORD(Wparam);                          // virtual-key code
		WORD keyFlags      = HIWORD(Lparam);
		WORD scanCode      = LOBYTE(keyFlags);                        // scan code
		BOOL isExtendedKey = (keyFlags & KF_EXTENDED) == KF_EXTENDED; // extended-key flag, 1 if scancode has 0xE0 prefix

		if (isExtendedKey)
			scanCode = MAKEWORD(scanCode, 0xE0);

		BOOL wasKeyDown    = (keyFlags & KF_REPEAT) == KF_REPEAT;     // previous key-state flag, 1 on autorepeat
		WORD repeatCount   = LOWORD(Lparam);                          // repeat count, > 0 if several keydown messages was combined into one message
		BOOL isKeyReleased = (keyFlags & KF_UP) == KF_UP;             // transition-state flag, 1 on keyup

		masEKey Key = masInput_ConvertWin32Key(vkCode);
		if (isKeyReleased)
			masInput_OnKey(EInputUser_0, Key, EKeyState_Release);
		else
		{
			if (wasKeyDown)
				masInput_OnKey(EInputUser_0, Key, EKeyState_Repeat);
			else
				masInput_OnKey(EInputUser_0, Key, EKeyState_Press);
		}
	}

		break;


    /*
	* TEXT EVENT HANDLE
	*/
	case WM_CHAR:
	case WM_SYSCHAR:
	case WM_UNICHAR:
		//masInput_OnText(Unicode);
		break;


	/*
	* MOUSE EVENT HANDLE
	*/
	case WM_MOUSEMOVE:
		//masInput_OnMouseMove(x, y);
		break;

	case WM_MOUSEWHEEL:
	{
		short Delta = GET_WHEEL_DELTA_WPARAM(Wparam);
		if (Delta > 0)
			masInput_OnKey(EInputUser_0, EKey_MouseWheelUp, EKeyState_Press);
		else if (Delta < 0)
			masInput_OnKey(EInputUser_0, EKey_MouseWheelDown, EKeyState_Press);
	}
		break;

	case WM_LBUTTONDOWN:   masInput_OnKey(EInputUser_0, EKey_MouseLeft, EKeyState_Press);       break;
	case WM_LBUTTONUP:	   masInput_OnKey(EInputUser_0, EKey_MouseLeft, EKeyState_Release);     break;
	case WM_LBUTTONDBLCLK: masInput_OnKey(EInputUser_0, EKey_MouseLeft, EKeyState_DoubleClick); break;

	case WM_RBUTTONDOWN:   masInput_OnKey(EInputUser_0, EKey_MouseRight, EKeyState_Press);       break;
	case WM_RBUTTONUP:	   masInput_OnKey(EInputUser_0, EKey_MouseRight, EKeyState_Release);     break;
	case WM_RBUTTONDBLCLK: masInput_OnKey(EInputUser_0, EKey_MouseRight, EKeyState_DoubleClick); break;

	case WM_MBUTTONDOWN:   masInput_OnKey(EInputUser_0, EKey_MouseMiddle, EKeyState_Press);       break;
	case WM_MBUTTONUP:	   masInput_OnKey(EInputUser_0, EKey_MouseMiddle, EKeyState_Release);     break;
	case WM_MBUTTONDBLCLK: masInput_OnKey(EInputUser_0, EKey_MouseMiddle, EKeyState_DoubleClick); break;

	case WM_XBUTTONDOWN:   break;
	case WM_XBUTTONUP:	   break;
	case WM_XBUTTONDBLCLK: break;


	/*
	* 
	*/
	case WM_DEVICECHANGE:
		masXInput_CheckControllersConnection();
		break;
	}
	return 0;
}

masEKey masInput_ConvertWin32Key(WPARAM Wparam)
{
	switch (Wparam)
	{
	case 'A': return EKey_A;
	case 'B': return EKey_B;
	case 'C': return EKey_C;
	case 'D': return EKey_D;
	case 'E': return EKey_E;
	case 'F': return EKey_F;
	case 'G': return EKey_G;
	case 'H': return EKey_H;
	case 'I': return EKey_I;
	case 'J': return EKey_J;
	case 'K': return EKey_K;
	case 'L': return EKey_L;
	case 'M': return EKey_M;
	case 'N': return EKey_N;
	case 'O': return EKey_O;
	case 'P': return EKey_P;
	case 'Q': return EKey_Q;
	case 'R': return EKey_R;
	case 'S': return EKey_S;
	case 'T': return EKey_T;
	case 'U': return EKey_U;
	case 'V': return EKey_V;
	case 'W': return EKey_W;
	case 'X': return EKey_X;
	case 'Y': return EKey_Y;
	case 'Z': return EKey_Z;

	case VK_NUMLOCK: return EKey_NumLock;
	case VK_NUMPAD0: return EKey_Numpad0;
	case VK_NUMPAD1: return EKey_Numpad1;
	case VK_NUMPAD2: return EKey_Numpad2;
	case VK_NUMPAD3: return EKey_Numpad3;
	case VK_NUMPAD4: return EKey_Numpad4;
	case VK_NUMPAD5: return EKey_Numpad5;
	case VK_NUMPAD6: return EKey_Numpad6;
	case VK_NUMPAD7: return EKey_Numpad7;
	case VK_NUMPAD8: return EKey_Numpad8;
	case VK_NUMPAD9: return EKey_Numpad9;

	case VK_F1:  return EKey_F1;
	case VK_F2:  return EKey_F2;
	case VK_F3:  return EKey_F3;
	case VK_F4:  return EKey_F4;
	case VK_F5:  return EKey_F5;
	case VK_F6:  return EKey_F6;
	case VK_F7:  return EKey_F7;
	case VK_F8:  return EKey_F8;
	case VK_F9:  return EKey_F9;
	case VK_F10: return EKey_F10;
	case VK_F11: return EKey_F11;
	case VK_F12: return EKey_F12;
	}

	return EKey_Unknown;
}
