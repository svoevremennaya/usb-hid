#include <Windows.h>
#include <iostream>
#include "DanceMat.h"

#define ARROW_WIDTH 96
#define ARROW_HEIGHT 96
#define NUMBERS_KEY_TO_PRESS 10

BYTE keysToPress[NUMBERS_KEY_TO_PRESS] = { ARROW_LEFT, SQUARE, ARROW_DOWN, ARROW_UP, TRIANGLE, ARROW_LEFT, ARROW_RIGHT, CIRCLE, CROSS, ARROW_DOWN };

HDC buf;
HANDLE hThreadReceive, hThreadCheckState;

HDC hArrowUpGreen, hArrowLeftGreen, hArrowDownGreen, hArrowRightGreen, hSquareGreen, hTriangleGreen, hCircleGreen, hCrossGreen;
HDC hArrowUpRed, hArrowLeftRed, hArrowDownRed, hArrowRightRed, hSquareRed, hTriangleRed, hCircleRed, hCrossRed;

typedef struct _GameWindow
{
	HWND hWndSelf;
	RECT rcClient;

	HDC hdcBack;
	HBITMAP hbmBack;
	HDC hArrowUp, hArrowLeft, hArrowDown, hArrowRight, hSquare, hTriangle, hCircle, hCross;
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

void LoadPictures(PGameWindow pSelf)
{
	pSelf->hArrowUp = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"arrow_up.bmp");
	pSelf->hArrowRight = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"arrow_right.bmp");
	pSelf->hArrowDown = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"arrow_down.bmp");
	pSelf->hArrowLeft = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"arrow_left.bmp");
	pSelf->hSquare = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"square.bmp");
	pSelf->hTriangle = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"triangle.bmp");
	pSelf->hCircle = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"circle.bmp");
	pSelf->hCross = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"cross.bmp");

	hArrowUpGreen = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"green\\arrow_up_green.bmp");
	hArrowRightGreen = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"green\\arrow_right_green.bmp");
	hArrowDownGreen = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"green\\arrow_down_green.bmp");
	hArrowLeftGreen = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"green\\arrow_left_green.bmp");
	hSquareGreen = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"green\\square.bmp_green");
	hTriangleGreen = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"green\\triangle_green.bmp");
	hCircleGreen = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"green\\circle_green.bmp");
	hCrossGreen = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"green\\cross_green.bmp");

	hArrowUpRed = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"red\\arrow_up_red.bmp");
	hArrowRightRed = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"red\\arrow_right_red.bmp");
	hArrowDownRed = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"red\\arrow_down_red.bmp");
	hArrowLeftRed = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"red\\arrow_left_red.bmp");
	hSquareRed = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"red\\square_red.bmp");
	hTriangleRed = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"red\\triangle_red.bmp");
	hCircleRed = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"red\\circle_red.bmp");
	hCrossRed = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"red\\cross_red.bmp");
}

// можно сразу загружать битмапки в поля структур, а не в отдельные переменные
void InitializeKeyStruct()
{
	arrowLeft.keyId = ARROW_LEFT;
	arrowLeft.hGreen = hArrowLeftGreen;
	arrowLeft.hRed = hArrowLeftRed;

	arrowRight.keyId = ARROW_RIGHT;
	arrowRight.hGreen = hArrowRightGreen;
	arrowRight.hRed = hArrowRightRed;

	arrowDown.keyId = ARROW_DOWN;
	arrowDown.hGreen = hArrowDownGreen;
	arrowDown.hRed = hArrowDownRed;

	arrowUp.keyId = ARROW_UP;
	arrowUp.hGreen = hArrowUpGreen;
	arrowUp.hRed = hArrowUpRed;

	circle.keyId = CIRCLE;
	circle.hGreen = hCircleGreen;
	circle.hRed = hCircleRed;

	triangle.keyId = TRIANGLE;
	triangle.hGreen = hTriangleGreen;
	triangle.hRed = hTriangleRed;

	square.keyId = SQUARE;
	square.hGreen = hSquareGreen;
	square.hRed = hSquareRed;

	cross.keyId = CROSS;
	cross.hGreen = hCrossGreen;
	cross.hRed = hCrossRed;

	selectKey.keyId = SELECT;
	startKey.keyId = START;
	empty.keyId = EMPTY;
}

void GameWindow_Draw(PGameWindow pSelf)
{
	FillRect(pSelf->hdcBack, &pSelf->rcClient, (HBRUSH)(CreateSolidBrush(RGB(40, 187, 253))));
	GdiTransparentBlt(pSelf->hdcBack, 452, 296, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowUp, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 548, 392, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowRight, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 452, 488, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowDown, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 356, 392, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowLeft, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 548, 488, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hSquare, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 356, 488, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hTriangle, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 548, 296, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hCircle, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 356, 296, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hCross, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
}

void CheckState(PGameWindow pSelf)
{
	while (1)
	{
		if (strPrev != pressedKeyStr)
		{
			switch (pressedKey.keyId)
			{
			case ARROW_UP:
				buf = pSelf->hArrowUp;
				pSelf->hArrowUp = hArrowUpRed;
				InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				Sleep(100);
				pSelf->hArrowUp = buf;
				break;
			case ARROW_LEFT:
				buf = pSelf->hArrowLeft;
				pSelf->hArrowLeft = hArrowLeftRed;
				InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				Sleep(100);
				pSelf->hArrowLeft = buf;
				break;
			case ARROW_DOWN:
				buf = pSelf->hArrowDown;
				pSelf->hArrowDown = hArrowDownRed;
				InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				Sleep(100);
				pSelf->hArrowDown = buf;
				break;
			case ARROW_RIGHT:
				buf = pSelf->hArrowRight;
				pSelf->hArrowRight = hArrowRightRed;
				InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				Sleep(100);
				pSelf->hArrowRight = buf;
				break;
			case SQUARE:
				buf = pSelf->hSquare;
				pSelf->hSquare = hSquareRed;
				InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				Sleep(100);
				pSelf->hSquare = buf;
				break;
			case TRIANGLE:
				buf = pSelf->hTriangle;
				pSelf->hTriangle = hTriangleRed;
				InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				Sleep(100);
				pSelf->hTriangle = buf;
				break;
			case CIRCLE:
				buf = pSelf->hCircle;
				pSelf->hCircle = hCircleRed;
				InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				Sleep(100);
				pSelf->hCircle = buf;
				break;
			case CROSS:
				buf = pSelf->hCross;
				pSelf->hCross = hCrossRed;
				InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				Sleep(100);
				pSelf->hCross = buf;
				break;
			}
			InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
		}
	}
}

void CheckState2(PGameWindow pSelf)
{
		
}

// Compare the expexted and pressed key
void CompareResults(PGameWindow pSelf)
{
	for (int i = 0; i < NUMBERS_KEY_TO_PRESS; i++)
	{
		do
		{
			if (pressedKey.keyId == keysToPress[i])
			{
				switch (pressedKey.keyId)
				{
				case ARROW_UP:
					buf = pSelf->hArrowUp;
					pSelf->hArrowUp = pressedKey.hGreen;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hArrowUp = buf;
					break;
				case ARROW_LEFT:
					buf = pSelf->hArrowLeft;
					pSelf->hArrowLeft = pressedKey.hGreen;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hArrowLeft = buf;
					break;
				case ARROW_DOWN:
					buf = pSelf->hArrowDown;
					pSelf->hArrowDown = pressedKey.hGreen;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hArrowDown = buf;
					break;
				case ARROW_RIGHT:
					buf = pSelf->hArrowRight;
					pSelf->hArrowRight = pressedKey.hGreen;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hArrowRight = buf;
					break;
				case SQUARE:
					buf = pSelf->hSquare;
					pSelf->hSquare = pressedKey.hGreen;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hSquare = buf;
					break;
				case TRIANGLE:
					buf = pSelf->hTriangle;
					pSelf->hTriangle = pressedKey.hGreen;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hTriangle = buf;
					break;
				case CIRCLE:
					buf = pSelf->hCircle;
					pSelf->hCircle = pressedKey.hGreen;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hCircle = buf;
					break;
				case CROSS:
					buf = pSelf->hCross;
					pSelf->hCross = pressedKey.hGreen;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hCross = buf;
					break;
				}
			}
			else
			{
				switch (pressedKey.keyId)
				{
				case ARROW_UP:
					buf = pSelf->hArrowUp;
					pSelf->hArrowUp = pressedKey.hRed;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hArrowUp = buf;
					break;
				case ARROW_LEFT:
					buf = pSelf->hArrowLeft;
					pSelf->hArrowLeft = pressedKey.hRed;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hArrowLeft = buf;
					break;
				case ARROW_DOWN:
					buf = pSelf->hArrowDown;
					pSelf->hArrowDown = pressedKey.hRed;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hArrowDown = buf;
					break;
				case ARROW_RIGHT:
					buf = pSelf->hArrowRight;
					pSelf->hArrowRight = pressedKey.hRed;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hArrowRight = buf;
					break;
				case SQUARE:
					buf = pSelf->hSquare;
					pSelf->hSquare = pressedKey.hRed;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hSquare = buf;
					break;
				case TRIANGLE:
					buf = pSelf->hTriangle;
					pSelf->hTriangle = pressedKey.hRed;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hTriangle = buf;
					break;
				case CIRCLE:
					buf = pSelf->hCircle;
					pSelf->hCircle = pressedKey.hRed;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hCircle = buf;
					break;
				case CROSS:
					buf = pSelf->hCross;
					pSelf->hCross = pressedKey.hRed;
					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
					Sleep(100);
					pSelf->hCross = buf;
					break;
				}
			}
			InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
			//Sleep(200);
		} while (pressedKey.keyId != keysToPress[i]);
	}
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
		LoadPictures(pSelf);
		InitializeKeyStruct();
		hThreadReceive = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartReceiveData, NULL, 0, 0);
		hThreadCheckState = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CompareResults, pSelf, 0, 0);
		break;
	case WM_DESTROY:
		GameWindow_FinalizeBackBuffer(pSelf);
		CloseHandle(hThreadReceive);
		CloseHandle(hThreadCheckState);
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
	case WM_LBUTTONDOWN:
		//Promah(pSelf);
		buf = pSelf->hArrowLeft;
		pSelf->hArrowLeft = hArrowLeftRed;
		InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
		break;
	case WM_LBUTTONUP:
		pSelf->hArrowLeft = buf;
		InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
		break;
	case WM_PAINT:
		GameWindow_Draw(pSelf);
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