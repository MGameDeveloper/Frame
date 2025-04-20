#include "masFrame.h"

#include "masWindow.h"
#include "masD3D11.h"
#include "Input/masInput.h"

bool masFrame_Init()
{
	if (Window_Create(L"Frame A", 1200, 800))
	{
		const WindowData* WndData = Window_GetData();
		if (masD3D11_Init(WndData->handle, WndData->display_width, WndData->display_height))
			Window_Display();
		else
		{
			Window_Destroy();
			return false;
		}
	}

	if (!masInput_Init())
		return false;

	return true;
}

void masFrame_DeInit()
{
	masD3D11_Shutdown();
	Window_Destroy();
}

bool masFrame_IsRunning()
{
	return !Window_GetData()->is_close;
}

void masFrame_Input()
{
	masInput_Reset();
	Window_HandleMessages();
	masInput_Process();
}

void masFrame_Render()
{
	masD3D11_Clear(150);
	masD3D11_Present();
}