#include "masWindow.h"
#include "masD3D11.h"
#include "Input/masInput.h"

#include <stdio.h>
#include "masFrame.h"

bool masGame_Init();
void masGame_DeInit();
void masGame_Update();

int main(int argc, const char** argv)
{
	if (!masFrame_Init())
		return -1;

	if (!masGame_Init())
		return -1;

	while (masFrame_IsRunning())
	{
		masFrame_Input();

		masGame_Update();

		masFrame_Render();
	}

	masGame_DeInit();
	masFrame_DeInit();
	return 0;
}




/****************************************************************************************
* EXAMPLE: Game Code
*****************************************************************************************/
#include "Input/masInputComp.h"

bool masGame_Init()   
{ 
	masInputComp *Comp = masInputComp_Create("MainInputComp", false, false);
	masInputComp_AddAction(Comp, "Reload",  []()             { printf("Reloading...\n");            }, EKeyMod_None, EKeyState_Press, MAS_KEY_LIST(EKey_R, EKey_Square));
	masInputComp_AddAxis  (Comp, "Forward", [](float Scaler) { printf("Forward[ %.2f ]\n", Scaler); }, 1.f, EKeyMod_None, MAS_KEY_LIST(EKey_LAnalogUp, EKey_W));

	masInputComp* SubComp = masInputComp_Create("SubInputComp", false, true);
	masInputComp_AddAction(SubComp, "ChargeJetWeapon", []() {printf("ChargingJetWeapon\n"); }, EKeyMod_None, EKeyState_Press, MAS_KEY_LIST(EKey_R, EKey_Square));

	masInputComp_Push(EInputUser_0, Comp);
	masInputComp_Push(EInputUser_0, SubComp);

	return true;
}

void masGame_DeInit() 
{ 
}

void masGame_Update()
{
	if (masInput_OnKeyEvent(EInputUser_0, EKeyMod_None, EKeyState_Press, MAS_KEY_LIST(EKey_R, EKey_Square)))
	{
		printf("OnKeyEvent\n");
	}

	if (masInput_OnKeyAxis(EInputUser_0, EKeyMod_None, MAS_KEY_LIST(EKey_S, EKey_LAnalogUp)))
	{
		printf("OnKeyAxis\n");
	}
}