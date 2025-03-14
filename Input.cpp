#include "Input.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#pragma comment(lib, "xinput.lib")

/******************************************************************************************************
*
* CODE FOR MAKING EVERY KEY INTO 4 BITS THAT REPRESENTS [ RELEASE, PRESS, REPEAT, DOUBLCLICK ] INSTEAD OF EVERY KEY BEING 2 BYTES [ CURRENT STATE, LAST STATE ]
*
*******************************************************************************************************/
typedef uint32_t masKeyBuf[32];
uint8_t masKey_GetByteIdx(uint32_t Key)
{
	if (Key != EKey_Unknown && Key < EKey_Count)
		Key--;
	else
		return 0;

	return (Key / 8);
}
uint8_t masKey_GetBitIdx(uint32_t Key)
{
	if (Key != EKey_Unknown && Key < EKey_Count)
		Key--;
	else
		return 0;

	return ((Key % 8) * 4);
}
void masKey_ClearBits(masKeyBuf KeyBuf, uint32_t Key)
{
	KeyBuf[masKey_GetByteIdx(Key)] &= ~(0x0000000F << masKey_GetBitIdx(Key));
}
void masKey_SetBits(masKeyBuf KeyBuf, uint32_t Key, uint32_t State)
{
	KeyBuf[masKey_GetByteIdx(Key)] |= ( (State & 0x0000000F) << masKey_GetBitIdx(Key));
}
void PrintKeyBits(masKeyBuf KeyBuf, uint32_t Key)
{
	uint32_t Count = 0;
	for (int32_t i = 31; i >= 0; --i)
	{
		if ((i != 31) && (Count % 4) == 0)
			printf(" ");

		if (KeyBuf[masKey_GetByteIdx(Key)] & (1 << i))
			printf("1");
		else
			printf("0");

		Count++;
	}
	printf("\n");
}
void PrintStateBits(uint8_t State)
{
	uint32_t Count = 0;
	for (int32_t i = 7; i >= 0; --i)
	{
		if ((i != 7) && (Count % 4) == 0)
			printf(" ");

		if (State & (1 << i))
			printf("1");
		else
			printf("0");

		Count++;
	}
}
uint8_t masKey_GetState(masKeyBuf KeyBuf, uint32_t Key)
{
	uint8_t ByteIdx = masKey_GetByteIdx(Key);
	uint8_t BitIdx  = masKey_GetBitIdx(Key);
	uint8_t State   = (KeyBuf[ByteIdx] >> BitIdx) & 0x0000000F;
	return State;
}
void masKey_SetState(masKeyBuf KeyBuf, uint32_t Key, uint32_t State)
{
	masKey_ClearBits(KeyBuf, Key);
	masKey_SetBits(KeyBuf, Key, State);
	//PrintKeyBits(KeyBuf, Key);
}



/******************************************************************************************************
*
* MAS INPUT INTERFACE IMPLEMENTATION
*
*******************************************************************************************************/
struct masInputUser
{
	masKeyBuf AxisKeyBuf;
	masKeyBuf EventKeyBuf;
	bool      bActive;
};

static masInputUser GInputUsers[EInputUser_Count] = {};

/*
* FORWARD DECLERATION 
*/
static void masXInput_Init();
static void masXInput_Update();

bool masInput_Init()
{
	GInputUsers[EInputUser_0].bActive = true;
	GInputUsers[EInputUser_1].bActive = false;
	GInputUsers[EInputUser_2].bActive = false;
	GInputUsers[EInputUser_3].bActive = false;

#if 0 // check key bits overlapping
	masKeyBuf TempBuf = {};
	for (int32_t i = 1; i < EKey_Count; ++i)
	{
		masEKey Key = (masEKey)i;

		uint8_t Byte = GetByteIdx(Key);
		uint8_t Bit  = GetBitIdx(Key);

		if (TempBuf[Byte] & (1 << Bit))
		{
			printf("Key Dublication happend\n");
			assert(false && "Key Dublication happend");
			return false;
		}
		else
			TempBuf[Byte] |= (1 << Bit);
	}
#endif

	masXInput_Init();

	return true;
}

void masInput_DeInit()
{

}

void masInput_Internal_OnKey(masEInputUser InputUser, uint32_t Key, uint8_t KeyState)
{
	masKey_SetState(GInputUsers[InputUser].AxisKeyBuf,  Key, KeyState);
	masKey_SetState(GInputUsers[InputUser].EventKeyBuf, Key, KeyState);
}

void masInput_Internal_OnAxis(masEInputUser InputUser, masEKey Key, float Scaler)
{
}

void masInput_Reset()
{
	for (uint8_t UserIdx = 0; UserIdx < EInputUser_Count; ++UserIdx)
	{
		// RESETN ENTIRE EVENT KEY BUF
		::memset(GInputUsers[UserIdx].EventKeyBuf, 0, sizeof(masKeyBuf));

		// RESET AXIS KEY BUF
		for (int32_t i = 0; i < EKey_Count; ++i)
		{
			uint8_t State = masKey_GetState(GInputUsers[UserIdx].AxisKeyBuf, i);
			
			// since os doesnt provide us with release events for thest we will set them manually
			if (i == EKey_MouseWheelUp || i == EKey_MouseWheelDown)
			{
				if (State == EKeyState_Press)
					masKey_SetState(GInputUsers[UserIdx].AxisKeyBuf, i, EKeyState_Unknown);
			}
			else
			{
				if (State != EKeyState_Unknown)
				{
					if (State == EKeyState_Release || State == EKeyState_DoubleClick)
						masKey_SetState(GInputUsers[UserIdx].AxisKeyBuf, i, EKeyState_Unknown);
				}
			}
		}
	}
}

void masInput_Update()
{
	masXInput_Update();
}

bool masInput_Internal_CheckKey(masKeyBuf TargetKeyBuf, masEKeyState KeyState, bool bAllKeyTrue, masEKey Key1, masEKey Key2, masEKey Key3, masEKey Key4, masEKey Key5, masEKey Key6, masEKey Key7)
{
	uint32_t Keys[7] = { Key1, Key2, Key3, Key4, Key5, Key6, Key7 };
	uint8_t  KeyCount = 0;
	uint8_t  ActiveKeyCount = 0;

	for (int32_t i = 0; i < 7; ++i)
	{
		if (Keys[i] == EKey_Unknown)
			continue;

		uint8_t State = masKey_GetState(TargetKeyBuf, Keys[i]);
		if ((KeyState | State) == KeyState && State != EKey_Unknown)
			ActiveKeyCount++;

		KeyCount++;
	}

	if (bAllKeyTrue)
	{
		if (KeyCount > 0 && KeyCount == ActiveKeyCount)
			return true;
		else
			return false;
	}
	else
	{
		if (ActiveKeyCount > 0)
			return true;
		else
			return false;
	}

	return false;
}
bool masInput_OnSingleAxisKey(masEInputUser InputUser, masEKey Key1, masEKey Key2, masEKey Key3, masEKey Key4, masEKey Key5, masEKey Key6, masEKey Key7)
{
	return masInput_Internal_CheckKey(GInputUsers[InputUser].AxisKeyBuf, EKeyState_Press | EKeyState_Repeat, false, Key1, Key2, Key3, Key4, Key5, Key6, Key7);
}
bool masInput_OnMultiAxisKey(masEInputUser InputUser, masEKey Key1, masEKey Key2, masEKey Key3, masEKey Key4, masEKey Key5, masEKey Key6, masEKey Key7)
{
	return masInput_Internal_CheckKey(GInputUsers[InputUser].AxisKeyBuf, EKeyState_Press | EKeyState_Repeat, true, Key1, Key2, Key3, Key4, Key5, Key6, Key7);
}
bool masInput_OnSingleEventKey(masEInputUser InputUser, masEKeyState KeyState, masEKey Key1, masEKey Key2, masEKey Key3, masEKey Key4, masEKey Key5, masEKey Key6, masEKey Key7)
{
	return masInput_Internal_CheckKey(GInputUsers[InputUser].EventKeyBuf, KeyState, false, Key1, Key2, Key3, Key4, Key5, Key6, Key7);
}
bool masInput_OnMultiEventKey(masEInputUser InputUser, masEKeyState KeyState, masEKey Key1, masEKey Key2, masEKey Key3, masEKey Key4, masEKey Key5, masEKey Key6, masEKey Key7)
{
	return masInput_Internal_CheckKey(GInputUsers[InputUser].EventKeyBuf, KeyState, true, Key1, Key2, Key3, Key4, Key5, Key6, Key7);
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
		if (Buttons[EKey_L2])           masInput_Internal_OnAxis(InputUser, EKey_L2,           XGamepad->bLeftTrigger / 255.f);
		if (Buttons[EKey_R2])           masInput_Internal_OnAxis(InputUser, EKey_R2,           XGamepad->bRightTrigger / 255.f);
		if (Buttons[EKey_LAnalogUp])    masInput_Internal_OnAxis(InputUser, EKey_LAnalogUp,    XGamepad->sThumbLY /  32767.f);
		if (Buttons[EKey_LAnalogDown])	masInput_Internal_OnAxis(InputUser, EKey_LAnalogDown,  XGamepad->sThumbLY / -32768.f);
		if (Buttons[EKey_LAnalogRight]) masInput_Internal_OnAxis(InputUser, EKey_LAnalogRight, XGamepad->sThumbLX /  32767.f);
		if (Buttons[EKey_LAnalogLeft])	masInput_Internal_OnAxis(InputUser, EKey_LAnalogLeft,  XGamepad->sThumbLX / -32768.f);
		if (Buttons[EKey_LAnalogUp])	masInput_Internal_OnAxis(InputUser, EKey_RAnalogUp,    XGamepad->sThumbRY /  32767.f);
		if (Buttons[EKey_LAnalogDown])	masInput_Internal_OnAxis(InputUser, EKey_RAnalogDown,  XGamepad->sThumbRY / -32768.f);
		if (Buttons[EKey_LAnalogRight]) masInput_Internal_OnAxis(InputUser, EKey_RAnalogRight, XGamepad->sThumbRX /  32767.f);
		if (Buttons[EKey_LAnalogLeft])	masInput_Internal_OnAxis(InputUser, EKey_RAnalogLeft,  XGamepad->sThumbRX / -32768.f);


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
					masInput_Internal_OnKey(InputUser, Key, EKeyState_Repeat);
					Gamepad->RepeatTime[ButtonIdx] += RepeatInitTime;
				}
			}
			else if (IsReleased)
				masInput_Internal_OnKey(InputUser, Key, EKeyState_Release);
			else if (IsPressed)
				masInput_Internal_OnKey(InputUser, Key, EKeyState_Press);
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

		//switch (vkCode)
		//{
		//case VK_SHIFT:   // converts to VK_LSHIFT or VK_RSHIFT
		//	vkCode = LOWORD(MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX));
		//case VK_CONTROL: // converts to VK_LCONTROL or VK_RCONTROL
		//	vkCode = LOWORD(MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX));
		//case VK_MENU:    // converts to VK_LMENU or VK_RMENU
		//	vkCode = LOWORD(MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX));
		//	break;
		//}

		masEKey Key = masInput_ConvertWin32Key(vkCode);
		if (isKeyReleased)
			masInput_Internal_OnKey(EInputUser_0, Key, EKeyState_Release);
		else
		{
			if (wasKeyDown)
				masInput_Internal_OnKey(EInputUser_0, Key, EKeyState_Repeat);
			else
				masInput_Internal_OnKey(EInputUser_0, Key, EKeyState_Press);
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
			masInput_Internal_OnKey(EInputUser_0, EKey_MouseWheelUp, EKeyState_Press);
		else if (Delta < 0)
			masInput_Internal_OnKey(EInputUser_0, EKey_MouseWheelDown, EKeyState_Press);
	}
		break;

	case WM_LBUTTONDOWN:   masInput_Internal_OnKey(EInputUser_0, EKey_MouseLeft, EKeyState_Press);       break;
	case WM_LBUTTONUP:	   masInput_Internal_OnKey(EInputUser_0, EKey_MouseLeft, EKeyState_Release);     break;
	case WM_LBUTTONDBLCLK: masInput_Internal_OnKey(EInputUser_0, EKey_MouseLeft, EKeyState_DoubleClick); break;
						   
	case WM_RBUTTONDOWN:   masInput_Internal_OnKey(EInputUser_0, EKey_MouseRight, EKeyState_Press);       break;
	case WM_RBUTTONUP:	   masInput_Internal_OnKey(EInputUser_0, EKey_MouseRight, EKeyState_Release);     break;
	case WM_RBUTTONDBLCLK: masInput_Internal_OnKey(EInputUser_0, EKey_MouseRight, EKeyState_DoubleClick); break;
						   
	case WM_MBUTTONDOWN:   masInput_Internal_OnKey(EInputUser_0, EKey_MouseMiddle, EKeyState_Press);       break;
	case WM_MBUTTONUP:	   masInput_Internal_OnKey(EInputUser_0, EKey_MouseMiddle, EKeyState_Release);     break;
	case WM_MBUTTONDBLCLK: masInput_Internal_OnKey(EInputUser_0, EKey_MouseMiddle, EKeyState_DoubleClick); break;

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

	case 0x30: return EKey_Num0;
	case 0x31: return EKey_Num1;
	case 0x32: return EKey_Num2;
	case 0x33: return EKey_Num3;
	case 0x34: return EKey_Num4;
	case 0x35: return EKey_Num5;
	case 0x36: return EKey_Num6;
	case 0x37: return EKey_Num7;
	case 0x38: return EKey_Num8;
	case 0x39: return EKey_Num9;
	
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

	case VK_DECIMAL:  return EKey_Decimal;
	case VK_PRIOR:    return EKey_PageUp;
	case VK_NEXT:     return EKey_PageDown;
	case VK_SPACE:    return EKey_Space;
	case VK_RETURN:   return EKey_Enter;
	case VK_BACK:     return EKey_Backspace;
	case VK_TAB:      return EKey_Tab;
	case VK_SNAPSHOT: return EKey_PrintScreen;
	case VK_INSERT:   return EKey_Insert;
	case VK_DELETE:   return EKey_Delete;
	case VK_DIVIDE:   return EKey_Divide;
	case VK_MULTIPLY: return EKey_Multipy;
	case VK_SUBTRACT: return EKey_Subtract;
	case VK_ADD:      return EKey_Addition;
	case VK_HOME:     return EKey_Home;
	case VK_END:      return EKey_End;
	case VK_ESCAPE:   return EKey_Escape;
	case VK_CAPITAL:  return EKey_CapsLock;
	case VK_UP:       return EKey_ArrowUp;
	case VK_DOWN:     return EKey_ArrowDown;
	case VK_LEFT:     return EKey_ArrowLeft;
	case VK_RIGHT:    return EKey_ArrowRight;
	case VK_LCONTROL: return EKey_LCtrl;
	case VK_RCONTROL: return EKey_RCtrl;
	case VK_LSHIFT:   return EKey_LShift;
	case VK_RSHIFT:   return EKey_RShift;
	case VK_LMENU:    return EKey_LAlt;
	case VK_RMENU:    return EKey_RAlt;
	case VK_CONTROL:  return EKey_Ctrl;
	case VK_SHIFT:    return EKey_Shift;
	case VK_MENU:     return EKey_Alt;
	}

	return EKey_Unknown;
}
