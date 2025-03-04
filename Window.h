#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct WindowData {
	HINSTANCE instance;
	HWND handle;
	UINT display_width;
	UINT display_height;
	UINT pos_x;
	UINT pos_y;
	LPCWSTR title;
	bool is_close;
};

bool Window_Create(LPCWSTR title, UINT width, UINT height);
void Window_Destroy();
void Window_HandleMessages();
void Window_Display();
const WindowData* Window_GetData();
