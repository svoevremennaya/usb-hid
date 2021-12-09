#include <Windows.h>
#include "GameWindow.h"

#define ARROW_WIDTH 96
#define ARROW_HEIGHT 96

HWND hWndGame;

HWND btnStart;
//HWND hWndGame;
//HWND hWndMain;

HINSTANCE hInst;
ATOM atomGame;

HDC hdcBack;
HBITMAP hbmBack;
RECT clientRect;
RECT rcGame;

HDC hArrowUp;

LRESULT CALLBACK WndGameProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void InitializeBackBuffer(HWND hWnd, int width, int height)
{
	HDC hdcWindow;
	hdcWindow = GetDC(hWnd);
	hdcBack = CreateCompatibleDC(hdcWindow);
	hbmBack = CreateCompatibleBitmap(hdcWindow, width, height);
	ReleaseDC(hWnd, hdcWindow);

	SaveDC(hdcBack);
	SelectObject(hdcBack, hbmBack);
}

void FinalizeBackBuffer()
{
	if (hdcBack)
	{
		RestoreDC(hdcBack, -1);
		DeleteObject(hbmBack);
		DeleteDC(hdcBack);
		hdcBack = 0;
	}
}

HDC LoadBitmapDC(HWND hWnd, const wchar_t* fileName)
{
	HANDLE hBitmap = LoadImage(0, (LPCWSTR)fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	HDC hdc = GetDC(hWnd);
	HDC resultDC = CreateCompatibleDC(hdc);
	SelectObject(resultDC, hBitmap);
	ReleaseDC(0, hdc);
	return resultDC;
}

void Draw()
{
	FillRect(hdcBack, &clientRect, (HBRUSH)(CreateSolidBrush(RGB(40, 187, 253))));
	GdiTransparentBlt(hdcBack, 10, 10, ARROW_WIDTH, ARROW_HEIGHT, hArrowUp, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (uMsg)
	{
	case WM_CREATE:
		//hArrowUp = LoadBitmapDC(hWnd, L"arrow_up.bmp");
		break;
	case WM_DESTROY:
		FinalizeBackBuffer();
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		GetClientRect(hWnd, &clientRect);
		FinalizeBackBuffer();
		InitializeBackBuffer(hWnd, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_COMMAND:
		if (lParam == (LPARAM)btnStart)
		{
			//atomGame = GameWindow_RegisterClass(hInst);
			hWndGame = CreateWindowEx(0, (LPCWSTR)atomGame, L"Game", WS_DISABLED | WS_OVERLAPPEDWINDOW, 100, 100, 1000, 700, 0, 0, hInst, NULL);
			EnableWindow(hWndGame, TRUE);
			ShowWindow(hWndGame, SW_NORMAL);
		}
		break;
	/*case WM_PAINT:
		Draw();
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, hdcBack, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;*/
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	WNDCLASSEX wcex;
	HWND hWnd;
	MSG msg;
	//ATOM atomGame;
	
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(0, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)COLOR_WINDOW; //(HBRUSH)(CreateSolidBrush(RGB(40, 187, 253)));
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = L"WindowClass";
	wcex.hIconSm = 0;

	RegisterClassEx(&wcex);

	atomGame = GameWindow_RegisterClass(hInstance);

	hWnd = CreateWindowEx(0, L"WindowClass", L"MyWindow", (WS_OVERLAPPEDWINDOW | WS_VISIBLE), 200, 100, 1000, 600, 0, 0, hInstance, NULL);
	btnStart = CreateWindowEx(0, L"BUTTON", L"START", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 400, 215, 200, 70, hWnd, NULL, hInstance, NULL);

	//hWndGame = CreateWindowEx(0, (LPCWSTR)atomGame, L"Game", WS_DISABLED | WS_OVERLAPPEDWINDOW, 100, 100, 1000, 700, 0, 0, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, 0, 0, 0))
	{
		DispatchMessage(&msg);
	}

	return msg.wParam;
}