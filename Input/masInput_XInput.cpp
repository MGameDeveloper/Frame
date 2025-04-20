#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

#include "masInputData.h"

#define EKey_GamepadCount (EKey_RAnalogRight + 1)
#define RepeatInitTime    0.2f
#define RepeatAdvanceTime 0.02f

struct masGamepad
{
	float RepeatTime[EKey_GamepadCount];
	bool  LastState[EKey_GamepadCount];
	//bool  bAvailable;
};

static masGamepad GGamepads[EInputUser_Count] = {};


void masXInput_CheckControllersConnection()
{
	masInputUser* User = NULL;
	for (int32_t UserIdx = 0; UserIdx < EInputUser_Count; ++UserIdx)
	{
		masEInputUser InputUser = (masEInputUser)UserIdx;
		User = &GInputData.InputUsers[InputUser];

		XINPUT_STATE XInputState = {};
		if (::XInputGetState(UserIdx, &XInputState) == ERROR_DEVICE_NOT_CONNECTED)
			User->bActive = false;
		else
			User->bActive = true;
	}
}

void masXInput_Init()
{
	masXInput_CheckControllersConnection();

	masInputUser* User = NULL;
	for (int32_t UserIdx = 0; UserIdx < EInputUser_Count; ++UserIdx)
	{
		masEInputUser InputUser = (masEInputUser)UserIdx;
		User = &GInputData.InputUsers[InputUser];

		User->DeadZone.LTrigger = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
		User->DeadZone.RTrigger = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
		User->DeadZone.LAnalogX = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
		User->DeadZone.LAnalogY = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
		User->DeadZone.RAnalogX = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
		User->DeadZone.RAnalogY = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;

		for (int32_t ButtonIdx = 0; ButtonIdx < EKey_GamepadCount; ++ButtonIdx)
			GGamepads[UserIdx].RepeatTime[ButtonIdx] = RepeatInitTime;
	}
}

void masXInput_Update()
{
	XINPUT_STATE    XInputState = {};
	XINPUT_GAMEPAD* XGamepad = NULL;
	masInputUser* User = NULL;
	for (int32_t UserIdx = 0; UserIdx < EInputUser_Count; ++UserIdx)
	{
		masEInputUser InputUser = (masEInputUser)UserIdx;
		User = &GInputData.InputUsers[InputUser];
		if (!User->bActive)
			continue;

		masGamepad* Gamepad = &GGamepads[UserIdx];
		//if (!Gamepad->bAvailable)
		//	continue;

		::memset(&XInputState, 0, sizeof(XINPUT_STATE));
		DWORD Ret = ::XInputGetState(UserIdx, &XInputState);
		if (Ret == ERROR_DEVICE_NOT_CONNECTED)
		{
			User->bActive = false;
			continue;
		}
		XGamepad = &XInputState.Gamepad;

		static bool Buttons[EKey_GamepadCount];
		::memset(Buttons, 0, sizeof(bool) * EKey_GamepadCount);

		Buttons[EKey_Square] = XGamepad->wButtons & XINPUT_GAMEPAD_X;
		Buttons[EKey_Cross] = XGamepad->wButtons & XINPUT_GAMEPAD_A;
		Buttons[EKey_Circle] = XGamepad->wButtons & XINPUT_GAMEPAD_B;
		Buttons[EKey_Triangle] = XGamepad->wButtons & XINPUT_GAMEPAD_Y;
		Buttons[EKey_Start] = XGamepad->wButtons & XINPUT_GAMEPAD_START;
		Buttons[EKey_Select] = XGamepad->wButtons & XINPUT_GAMEPAD_BACK;
		Buttons[EKey_DpadUp] = XGamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
		Buttons[EKey_DpadDown] = XGamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
		Buttons[EKey_DpadRight] = XGamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
		Buttons[EKey_DpadLeft] = XGamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
		Buttons[EKey_L1] = XGamepad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
		Buttons[EKey_R1] = XGamepad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
		Buttons[EKey_L3] = XGamepad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
		Buttons[EKey_R3] = XGamepad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
		/////////////////////////////////////////////////////////////////////////////////////
		Buttons[EKey_L2] = XGamepad->bLeftTrigger > User->DeadZone.LTrigger;
		Buttons[EKey_R2] = XGamepad->bRightTrigger > User->DeadZone.RTrigger;
		Buttons[EKey_LAnalogUp] = XGamepad->sThumbLY > User->DeadZone.LAnalogY;
		Buttons[EKey_LAnalogDown] = XGamepad->sThumbLY < -User->DeadZone.LAnalogY;
		Buttons[EKey_LAnalogRight] = XGamepad->sThumbLX > User->DeadZone.LAnalogX;
		Buttons[EKey_LAnalogLeft] = XGamepad->sThumbLX < -User->DeadZone.LAnalogX;
		Buttons[EKey_RAnalogUp] = XGamepad->sThumbRY > User->DeadZone.RAnalogY;
		Buttons[EKey_RAnalogDown] = XGamepad->sThumbRY < -User->DeadZone.RAnalogY;
		Buttons[EKey_RAnalogRight] = XGamepad->sThumbRX > User->DeadZone.RAnalogX;
		Buttons[EKey_RAnalogLeft] = XGamepad->sThumbRX < -User->DeadZone.RAnalogX;

		/*
		* Dispatch axis keys L2, R2, LAnalog, RAnalog
		*/
		if (Buttons[EKey_L2])           masInputData_SetUserAxisValue(InputUser, EKey_L2, XGamepad->bLeftTrigger / 255.f);
		if (Buttons[EKey_R2])           masInputData_SetUserAxisValue(InputUser, EKey_R2, XGamepad->bRightTrigger / 255.f);
		if (Buttons[EKey_LAnalogUp])    masInputData_SetUserAxisValue(InputUser, EKey_LAnalogUp, XGamepad->sThumbLY / 32767.f);
		if (Buttons[EKey_LAnalogDown])	masInputData_SetUserAxisValue(InputUser, EKey_LAnalogDown, XGamepad->sThumbLY / 32768.f);
		if (Buttons[EKey_LAnalogRight]) masInputData_SetUserAxisValue(InputUser, EKey_LAnalogRight, XGamepad->sThumbLX / 32767.f);
		if (Buttons[EKey_LAnalogLeft])	masInputData_SetUserAxisValue(InputUser, EKey_LAnalogLeft, XGamepad->sThumbLX / 32768.f);
		if (Buttons[EKey_RAnalogUp])	masInputData_SetUserAxisValue(InputUser, EKey_RAnalogUp, XGamepad->sThumbRY / 32767.f);
		if (Buttons[EKey_RAnalogDown])	masInputData_SetUserAxisValue(InputUser, EKey_RAnalogDown, XGamepad->sThumbRY / 32768.f);
		if (Buttons[EKey_RAnalogRight]) masInputData_SetUserAxisValue(InputUser, EKey_RAnalogRight, XGamepad->sThumbRX / 32767.f);
		if (Buttons[EKey_RAnalogLeft])	masInputData_SetUserAxisValue(InputUser, EKey_RAnalogLeft, XGamepad->sThumbRX / 32768.f);


		/*
		* Dispatch Buttons
		*/
		for (int32_t ButtonIdx = 0; ButtonIdx < EKey_GamepadCount; ++ButtonIdx)
		{
			masEKey Key = (masEKey)ButtonIdx;
			bool CurrState = Buttons[ButtonIdx];
			bool LastState = Gamepad->LastState[ButtonIdx];
			bool IsPressed = !LastState && CurrState;
			bool IsReleased = LastState && !CurrState;
			bool IsRepeated = LastState && CurrState;

			if (IsRepeated)
			{
				if (Gamepad->RepeatTime[ButtonIdx] < Gamepad->RepeatTime[ButtonIdx] + RepeatInitTime)
					Gamepad->RepeatTime[ButtonIdx] += RepeatAdvanceTime;
				else
				{
					masInputData_SetKeyState(InputUser, Key, EKeyState_Repeat);
					Gamepad->RepeatTime[ButtonIdx] += RepeatInitTime;
				}
			}
			else if (IsReleased)
				masInputData_SetKeyState(InputUser, Key, EKeyState_Release);
			else if (IsPressed)
				masInputData_SetKeyState(InputUser, Key, EKeyState_Press);
		}

		::memcpy(Gamepad->LastState, Buttons, sizeof(bool) * EKey_GamepadCount);
	}
}