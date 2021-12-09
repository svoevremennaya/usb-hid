#include <Windows.h>

#define ARROW_WIDTH 96
#define ARROW_HEIGHT 96

typedef struct _GameWindow
{
	HWND hWndSelf;
	RECT rcClient;

	HDC hdcBack;
	HBITMAP hbmBack;
	HDC hArrowUp;
} GameWindow, *PGameWindow;

void GameWnd_InitializeBackBuffer(PGameWindow pSelf)
{
	HDC hdcWindow;
	int width, height;
	width = pSelf->rcClient.right - pSelf->rcClient.left;
	height = pSelf->rcClient.bottom - pSelf->rcClient.top;

	hdcWindow = GetDC(pSelf->hWndSelf);
	pSelf->hdcBack = CreateCompatibleDC(hdcWindow);
	pSelf->hbmBack = CreateCompatibleBitmap(hdcWindow, width, height);
	ReleaseDC(pSelf->hWndSelf, hdcWindow);

	SaveDC(pSelf->hdcBack);
	SelectObject(pSelf->hdcBack, pSelf->hbmBack);
}

void GameWindow_FinalizeBackBuffer(PGameWindow pSelf)
{
	if (pSelf->hdcBack)
	{
		RestoreDC(pSelf->hdcBack, -1);
		DeleteObject(pSelf->hbmBack);
		DeleteDC(pSelf->hdcBack);
		pSelf->hdcBack = 0;
	}
}

HDC GameWindow_LoadBitmapDC(HWND hWnd, const wchar_t* fileName)
{
	HANDLE hBitmap = LoadImage(0, (LPCWSTR)fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	HDC hdc = GetDC(hWnd);
	HDC resultDC = CreateCompatibleDC(hdc);
	SelectObject(resultDC, hBitmap);
	ReleaseDC(0, hdc);
	return resultDC;
}

void GameWnd_Draw(PGameWindow pSelf)
{
	FillRect(pSelf->hdcBack, &pSelf->rcClient, (HBRUSH)(CreateSolidBrush(RGB(40, 187, 253))));
	GdiTransparentBlt(pSelf->hdcBack, 10, 10, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowUp, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
}

LRESULT CALLBACK GameWindow_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PGameWindow pSelf;

	PAINTSTRUCT ps;
	HDC hdc;

	if (WM_CREATE == uMsg)
	{
		pSelf = (PGameWindow)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(GameWindow));
		SetWindowLong(hWnd, 0, (LONG)pSelf);
		pSelf->hWndSelf = hWnd;
	}
	else
	{
		pSelf = (PGameWindow)GetWindowLong(hWnd, 0);
		if (!pSelf)
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	switch (uMsg)
	{
	case WM_CREATE:
		pSelf->hArrowUp = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"arrow_up.bmp");
		break;
	case WM_DESTROY:
		GameWindow_FinalizeBackBuffer(pSelf);
		HeapFree(GetProcessHeap(), 0, pSelf);
		break;
	case WM_SIZE:
		GetClientRect(pSelf->hWndSelf, &pSelf->rcClient);
		GameWindow_FinalizeBackBuffer(pSelf);
		GameWnd_InitializeBackBuffer(pSelf);
		InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
		break;
	case WM_ERASEBKGND:
		return TRUE;
	case WM_PAINT:
		GameWnd_Draw(pSelf);
		hdc = BeginPaint(pSelf->hWndSelf, &ps);
		BitBlt(hdc, 0, 0, pSelf->rcClient.right - pSelf->rcClient.left, pSelf->rcClient.bottom - pSelf->rcClient.top, pSelf->hdcBack, 0, 0, SRCCOPY);
		EndPaint(pSelf->hWndSelf, &ps);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

ATOM GameWindow_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = GameWindow_WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 4;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(0, IDC_HAND);
	wcex.hbrBackground = 0;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"GameWindow";
	wcex.hIconSm = 0;
	return RegisterClassEx(&wcex);
}