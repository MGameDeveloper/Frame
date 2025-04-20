#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>

#include "masInputData.h"

/*
* Defined in masInput_XInput.cpp
*/
extern void masXInput_CheckControllersConnection();


/*****************************************************************************
*
******************************************************************************/
void masInput_UpdateKeyMod()
{
	uint16_t CtrlMask  = EKeyMod_LCtrl  | EKeyMod_RCtrl;
	uint16_t ShiftMask = EKeyMod_LShift | EKeyMod_RShift;
	uint16_t AltMask   = EKeyMod_LAlt   | EKeyMod_RAlt;

	uint16_t KeyMod = 0;
	KeyMod |= GetAsyncKeyState(VK_LCONTROL) ? EKeyMod_LCtrl : 0;
	KeyMod |= GetAsyncKeyState(VK_RCONTROL) ? EKeyMod_RCtrl : 0;
	KeyMod |= GetAsyncKeyState(VK_LSHIFT) ? EKeyMod_LShift : 0;
	KeyMod |= GetAsyncKeyState(VK_RSHIFT) ? EKeyMod_RShift : 0;
	KeyMod |= GetAsyncKeyState(VK_LMENU) ? EKeyMod_LAlt : 0;
	KeyMod |= GetAsyncKeyState(VK_RMENU) ? EKeyMod_RAlt : 0;

	if (KeyMod & CtrlMask)
		KeyMod |= EKeyMod_Ctrl;
	if (KeyMod & ShiftMask)
		KeyMod |= EKeyMod_Shift;
	if (KeyMod & AltMask)
		KeyMod |= EKeyMod_Alt;

	GInputData.KeyMod = KeyMod;
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
	}

	return EKey_Unknown;
}


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
		WORD vkCode = LOWORD(Wparam);                          // virtual-key code
		WORD keyFlags = HIWORD(Lparam);
		WORD scanCode = LOBYTE(keyFlags);                        // scan code
		BOOL isExtendedKey = (keyFlags & KF_EXTENDED) == KF_EXTENDED; // extended-key flag, 1 if scancode has 0xE0 prefix

		if (isExtendedKey)
			scanCode = MAKEWORD(scanCode, 0xE0);

		BOOL wasKeyDown = (keyFlags & KF_REPEAT) == KF_REPEAT;     // previous key-state flag, 1 on autorepeat
		WORD repeatCount = LOWORD(Lparam);                          // repeat count, > 0 if several keydown messages was combined into one message
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
			masInputData_SetKeyState(EInputUser_0, Key, EKeyState_Release);
		else
		{
			if (wasKeyDown)
				masInputData_SetKeyState(EInputUser_0, Key, EKeyState_Repeat);
			else
				masInputData_SetKeyState(EInputUser_0, Key, EKeyState_Press);
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
	{
		float MouseDirX = GET_X_LPARAM(Lparam) - GInputData.MouseDirX;
		float MouseDirY = GET_Y_LPARAM(Lparam) - GInputData.MouseDirY;

		if (MouseDirX > 10.f)  MouseDirX = 1.f;
		if (MouseDirX < -10.f) MouseDirX = -1.f;
		if (MouseDirY > 10.f)  MouseDirY = 1.f;
		if (MouseDirY < -10.f) MouseDirY = -1.f;

		GInputData.MouseDirX = MouseDirX;
		GInputData.MouseDirY = MouseDirY;
		GInputData.MousePosX = GET_X_LPARAM(Lparam);
		GInputData.MousePosY = GET_Y_LPARAM(Lparam);
	}
	break;

	case WM_MOUSEWHEEL:
	{
		short Delta = GET_WHEEL_DELTA_WPARAM(Wparam);
		if (Delta > 0)
			masInputData_SetKeyState(EInputUser_0, EKey_MouseWheelUp, EKeyState_Press);
		else if (Delta < 0)
			masInputData_SetKeyState(EInputUser_0, EKey_MouseWheelDown, EKeyState_Press);
	}
	break;

	case WM_LBUTTONDOWN:   masInputData_SetKeyState(EInputUser_0, EKey_MouseLeft, EKeyState_Press);       break;
	case WM_LBUTTONUP:	   masInputData_SetKeyState(EInputUser_0, EKey_MouseLeft, EKeyState_Release);     break;
	case WM_LBUTTONDBLCLK: masInputData_SetKeyState(EInputUser_0, EKey_MouseLeft, EKeyState_DoubleClick); break;

	case WM_RBUTTONDOWN:   masInputData_SetKeyState(EInputUser_0, EKey_MouseRight, EKeyState_Press);       break;
	case WM_RBUTTONUP:	   masInputData_SetKeyState(EInputUser_0, EKey_MouseRight, EKeyState_Release);     break;
	case WM_RBUTTONDBLCLK: masInputData_SetKeyState(EInputUser_0, EKey_MouseRight, EKeyState_DoubleClick); break;

	case WM_MBUTTONDOWN:   masInputData_SetKeyState(EInputUser_0, EKey_MouseMiddle, EKeyState_Press);       break;
	case WM_MBUTTONUP:	   masInputData_SetKeyState(EInputUser_0, EKey_MouseMiddle, EKeyState_Release);     break;
	case WM_MBUTTONDBLCLK: masInputData_SetKeyState(EInputUser_0, EKey_MouseMiddle, EKeyState_DoubleClick); break;

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

