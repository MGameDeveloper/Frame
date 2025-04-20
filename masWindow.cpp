#include "masWindow.h"
#include <assert.h>

#define WND_CLASSNAME TEXT("FrameWndCls")

static WindowData GWindowData;

const WindowData* Window_GetData() {
	assert(GWindowData.handle != NULL && "Window has not been created");
	return &GWindowData;
}


// [ Miscellanies ]
static void _LogMsg(const char* Caption, const char* Msg) {
	MessageBoxA(NULL, Msg, Caption, MB_OK);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool Window_Create(LPCWSTR Title, UINT width, UINT height) {
	GWindowData.instance = GetModuleHandle(NULL);

	bool ret_val = true;
	WNDCLASSEX WCEX = { 0 };
	WCEX.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	WCEX.cbSize = sizeof(WNDCLASSEX);
	WCEX.hInstance = GWindowData.instance;
	WCEX.lpszClassName = WND_CLASSNAME;
	WCEX.lpfnWndProc = WndProc;
	WCEX.cbClsExtra = 0;
	WCEX.cbWndExtra = DLGWINDOWEXTRA;
	WCEX.hIcon = LoadIcon(GWindowData.instance, MAKEINTRESOURCE(IDI_APPLICATION));
	WCEX.hIconSm = LoadIcon(GWindowData.instance, MAKEINTRESOURCE(IDI_APPLICATION));
	WCEX.hCursor = LoadCursor(GWindowData.instance, IDC_ARROW);
	WCEX.lpszMenuName = NULL;

	if (RegisterClassEx(&WCEX) > 0) {
		RECT WinRect = { 0 };
		SetRect(&WinRect, 0, 0, width, height);
		if (!AdjustWindowRect(&WinRect, 0, FALSE)) {
			_LogMsg("WND", "Warning: Failed adjusting window size");
		}

		GWindowData.display_width = WinRect.right - WinRect.left;
		GWindowData.display_height = WinRect.bottom - WinRect.top;
		GWindowData.handle = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, WND_CLASSNAME, (GWindowData.title = Title), WS_OVERLAPPEDWINDOW,
			GWindowData.pos_x, GWindowData.pos_y, GWindowData.display_width, GWindowData.display_height, NULL, NULL, GWindowData.instance, NULL);
		if (GWindowData.handle == INVALID_HANDLE_VALUE) {
			UnregisterClass(WCEX.lpszClassName, WCEX.hInstance);
			_LogMsg("WND", "Failed creating window");
			ret_val = false;
		}
	}
	else {
		_LogMsg("WND", "Failed registering window");
	}

	//wchar_t buf[256] = {};
	//::GetWindowTextW(GWindowData.handle, buf, 256);

	return ret_val;
}


void Window_Destroy() {
	DestroyWindow(GWindowData.handle);
	UnregisterClassW(WND_CLASSNAME, GWindowData.instance);
}

void Window_Display() {
	UpdateWindow(GWindowData.handle);
	ShowWindow(GWindowData.handle, SW_SHOW);
}

void Window_HandleMessages() {
	MSG WinMsg{ };
	while (PeekMessage(&WinMsg, NULL, 0, 0, PM_REMOVE)) {
		if (WinMsg.message == WM_QUIT)
			GWindowData.is_close = true;
		TranslateMessage(&WinMsg);
		DispatchMessage(&WinMsg);
	}
}


/*************************************************************************************
*
**************************************************************************************/
extern LRESULT masInput_Win32Proc(HWND Hwnd, UINT Msg, WPARAM Wparam, LPARAM Lparam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	masInput_Win32Proc(hWnd, uMsg, wParam, lParam);
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
};