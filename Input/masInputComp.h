#pragma once

#include "masInputCommon.h"

#define MAS_KEY_LIST(...) #__VA_ARGS__##"\n", __VA_ARGS__

struct masInputComp;
typedef void(*masInputActionFunc)();
typedef void(*masInputAxisFunc)(float);



/********************************************
*
*********************************************/
masInputComp* masInputComp_Create (const char* Name, bool bConsumeEvent, bool bBlockInput);
void          masInputComp_Destroy(masInputComp** InputComp);



/********************************************
*
*********************************************/
void masInputComp_AddAction(masInputComp* Comp, const char* Name, masInputActionFunc ActionFunc, EKeyMod_ KeyMod,    EKeyState_ KeyState, const char* KeyListFmt, ...);
void masInputComp_AddAxis  (masInputComp* Comp, const char* Name, masInputAxisFunc   AxisFunc,   float    AxisValue, EKeyMod_   KeyMod,   const char* KeyListFmt, ...);



/********************************************
*
*********************************************/
void masInputComp_Push(masEInputUser EInputUser, masInputComp* InputComp);
void masInputComp_Pop (masEInputUser EInputUser);



/********************************************
* 
*********************************************/
bool masInput_OnKeyEvent(masEInputUser InputUser, EKeyMod_ KeyMod, EKeyState_ KeyState, const char* KeyListFmt, ...);
bool masInput_OnKeyAxis (masEInputUser InputUser, EKeyMod_ KeyMod, const char* KeyListFmt, ...);



/********************************************
*
*********************************************/
const char* masInput_KeyName   (masEKey      Key);
const char* masInput_KeyModName(masEKeyMod   KeyMod);
const char* masInput_KeyState  (masEKeyState KeyState);