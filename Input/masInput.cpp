#include "masInput.h"

//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
//#include <windowsx.h>
//#include <Xinput.h>
//#pragma comment(lib, "xinput.lib")
//
//#include <stdlib.h>
//#include <stdio.h>
//#include <assert.h>
#include <string.h>

#include "masInputData.h"


//#define MAS_ADDR_FROM(TYPE, PTR, OFFSET) (TYPE*)(((uint8_t*)PTR) + OFFSET)
//#define EKey_GamepadCount (EKey_RAnalogRight + 1)

/*
* Defined in masInput_XInput.cpp
*/
extern void masXInput_Init();
extern void masXInput_Update();

/*
* Defined in masInputComp.cpp
*/
extern void masInputComp_Update();


/*
* Defined in masInput_Win32.cpp
*/
extern void masInput_UpdateKeyMod();



/******************************************************************************************************
* 
*******************************************************************************************************/
bool masInput_Init()
{
	if (!masInputData_Alloc())
		return false;

	masXInput_Init();

	return true;
}

void masInput_DeInit()
{
	masInputData_DeAlloc();
}

void masInput_Reset()
{
	// RESET EVENT KEY BUF
	masInputUser* User = NULL;
	for (int32_t i = 0; i < EInputUser_Count; ++i)
	{
		masEInputUser InputUser = (masEInputUser)i;
		User                    = &GInputData.InputUsers[InputUser];

		if (!User->bActive)
			continue;
		::memset(User->EventKeyBuf->Uint32Buf, 0, sizeof(uint32_t) * User->EventKeyBuf->Uint32Count);
	}
}

void masInput_Process()
{
	masInput_UpdateKeyMod();
	masXInput_Update();
	masInputComp_Update();
}