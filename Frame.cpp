#include "Window.h"
#include "D3D11.h"
#include "Input.h"

#include <stdio.h>

int main(int argc, const char** argv)
{
	if (Window_Create(L"Frame A", 1200, 800))
	{
		const WindowData* WndData = Window_GetData();
		if (D3D11_Init(WndData->handle, WndData->display_width, WndData->display_height))
			Window_Display();
		else
		{
			Window_Destroy();
			return 1;
		}
	}
	
	while (!Window_GetData()->is_close)
	{
		Input_Reset();
		Window_HandleMessages();

		const InputMouse* Mouse = Input_GetMouse();
		if (Mouse->Left.DoubleClick)
			printf("LEFT MOUSE DOUBLECLICK[ %.2f, %.2f ]\n", Mouse->PosX, Mouse->PosY);
		if (Mouse->Right.DoubleClick)
			printf("RIGHT Mouse Button[ %.2f, %.2f ]\n", Mouse->PosX, Mouse->PosY);
		if (Mouse->Middle.DoubleClick)
			printf("MIDDLE Mouse Button[ %.2f, %.2f ]\n", Mouse->PosX, Mouse->PosY);
		if (Mouse->ScrollDown)
			printf("SCROLL DOWN[ %.2f, %.2f ]\n", Mouse->PosX, Mouse->PosY);
		if (Mouse->ScrollUp)
			printf("SCROLL UP[ %.2f, %.2f ]\n", Mouse->PosX, Mouse->PosY);
		if(Mouse->Left.Press)
			printf("LEFT MOUSE PRESS[ %.2f, %.2f ]\n", Mouse->PosX, Mouse->PosY);
		if(Mouse->Left.Release)
			printf("LEFT MOUSE RELEASE[ %.2f, %.2f ]\n", Mouse->PosX, Mouse->PosY);

		// const InputKeyboard* Keyboard = Input_GetKeyboard();
		// if( Keyboard->Keys[KEY_A].Press) ...
		// if( Keyboard->Unicode ) ...
		D3D11_Clear(150);
		D3D11_Present();
	}

	//     D3D11_Shutdown();
	Window_Destroy();
	return 0;
}


void deferred_context()
{
	const D3D11Data* pD3D = D3D11_GetData();

	ID3D11CommandList* CommandList = NULL;
	ID3D11DeviceContext* DeferredContext = NULL;
	
	HRESULT hr = pD3D->Device5->CreateDeferredContext(0, &DeferredContext);
	
	DeferredContext->FinishCommandList(false, &CommandList);

	// Register draw commands


	pD3D->ImmediateContext4->ExecuteCommandList(CommandList, false);
	// present
};