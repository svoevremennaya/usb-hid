#include <Windows.h>
#include <iostream>
#include "DanceMat.h"

#define ARROW_WIDTH 96
#define ARROW_HEIGHT 96
#define NUMBERS_KEY_TO_PRESS 8

BOOL keysToPress[NUMBERS_KEY_TO_PRESS][8] =
{
	// AL    AD      AU     AR     T     SQ     CR     CIR
	{ FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE },
	{ TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },
	{ FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },
	{ FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE},
	{ TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE },
	{ FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE },
	{ FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE },
	{ FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE }
};

HANDLE hThreadReceive, hThreadCheckState;

int iteration = 0;

int coordKeysToPress[2][8] = {
	{356, 452, 452, 548, 356, 548, 356, 548},
	{106, 202, 10, 106, 202, 202, 10, 10}
};

HDC hBack;
HDC hArrowUp, hArrowLeft, hArrowDown, hArrowRight, hSquare, hTriangle, hCircle, hCross;
HDC hArrowUpGreen, hArrowLeftGreen, hArrowDownGreen, hArrowRightGreen, hSquareGreen, hTriangleGreen, hCircleGreen, hCrossGreen;
HDC hArrowUpRed, hArrowLeftRed, hArrowDownRed, hArrowRightRed, hSquareRed, hTriangleRed, hCircleRed, hCrossRed;

HDC pictures[8];

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
	hBack = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"red_back.bmp");

	pSelf->hArrowLeft = hArrowLeft = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"arrow_left.bmp");
	pSelf->hArrowDown = hArrowDown = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"arrow_down.bmp");
	pSelf->hArrowUp = hArrowUp = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"arrow_up.bmp");
	pSelf->hArrowRight = hArrowRight = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"arrow_right.bmp");
	pSelf->hTriangle = hTriangle = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"triangle.bmp");
	pSelf->hSquare = hSquare = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"square.bmp");
	pSelf->hCross = hCross = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"cross.bmp");
	pSelf->hCircle = hCircle = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"circle.bmp");
	

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

void InitializeKeyStruct(PGameWindow pSelf)
{
	pictures[0] = hArrowLeft;
	pictures[1] = hArrowDown;
	pictures[2] = hArrowUp;
	pictures[3] = hArrowRight;
	pictures[4] = hTriangle;
	pictures[5] = hSquare;
	pictures[6] = hCross;
	pictures[7] = hCircle;
}

void GameWindow_Draw(PGameWindow pSelf)
{
	//FillRect(pSelf->hdcBack, &pSelf->rcClient, (HBRUSH)(CreateSolidBrush(RGB(40, 187, 253))));
	StretchBlt(pSelf->hdcBack, 0, 0, pSelf->rcClient.right, pSelf->rcClient.bottom, hBack, 0, 0, 1280, 720, SRCCOPY);
	GdiTransparentBlt(pSelf->hdcBack, 452, 316, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowUp, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 548, 412, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowRight, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 452, 508, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowDown, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 356, 412, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowLeft, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 548, 508, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hSquare, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 356, 508, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hTriangle, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 548, 316, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hCircle, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 356, 316, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hCross, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));

	for (int i = 0; i < 8; i++)
	{
		if (keysToPress[iteration][i])
		{
			GdiTransparentBlt(pSelf->hdcBack, coordKeysToPress[0][i], coordKeysToPress[1][i], ARROW_WIDTH, ARROW_HEIGHT, pictures[i], 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
		}
	}
}

void GameWindow_CheckState(PGameWindow pSelf)
{
	while (1)
	{
		if (pressedKeys[0]) { pSelf->hArrowLeft = hArrowLeftRed; }
		if (pressedKeys[1]) { pSelf->hArrowDown = hArrowDownRed; }
		if (pressedKeys[2]) { pSelf->hArrowUp = hArrowUpRed; }
		if (pressedKeys[3]) { pSelf->hArrowRight = hArrowRightRed; }
		if (pressedKeys[4]) { pSelf->hTriangle = hTriangleRed; }
		if (pressedKeys[5]) { pSelf->hSquare = hSquareRed; }
		if (pressedKeys[6]) { pSelf->hCross = hCrossRed; }
		if (pressedKeys[7]) { pSelf->hCircle = hCircleRed; }

		Sleep(100);
		InvalidateRect(pSelf->hWndSelf, NULL, TRUE);

		if (!pressedKeys[0]) { pSelf->hArrowLeft = hArrowLeft; }
		if (!pressedKeys[1]) { pSelf->hArrowDown = hArrowDown; }
		if (!pressedKeys[2]) { pSelf->hArrowUp = hArrowUp; }
		if (!pressedKeys[3]) { pSelf->hArrowRight = hArrowRight; }
		if (!pressedKeys[4]) { pSelf->hTriangle = hTriangle; }
		if (!pressedKeys[5]) { pSelf->hSquare = hSquare; }
		if (!pressedKeys[6]) { pSelf->hCross = hCross; }
		if (!pressedKeys[7]) { pSelf->hCircle = hCircle; }
	}
}

void GameWindow_CompareResults(PGameWindow pSelf)
{
	BOOL done = FALSE;
	for (iteration = 0; iteration < NUMBERS_KEY_TO_PRESS; iteration++)
	{
		//DrawKeysToPress(pSelf, i);
		done = FALSE;
		do
		{
			if (pressedKeys[0] == TRUE && keysToPress[iteration][0] == TRUE) { pSelf->hArrowLeft = hArrowLeftGreen; done = TRUE; }
			if (pressedKeys[1] == TRUE && keysToPress[iteration][1] == TRUE) { pSelf->hArrowDown = hArrowDownGreen; done = TRUE; }
			if (pressedKeys[2] == TRUE && keysToPress[iteration][2] == TRUE) { pSelf->hArrowUp = hArrowUpGreen; done = TRUE; }
			if (pressedKeys[3] == TRUE && keysToPress[iteration][3] == TRUE) { pSelf->hArrowRight = hArrowRightGreen; done = TRUE; }
			if (pressedKeys[4] == TRUE && keysToPress[iteration][4] == TRUE) { pSelf->hTriangle = hTriangleGreen; done = TRUE; }
			if (pressedKeys[5] == TRUE && keysToPress[iteration][5] == TRUE) { pSelf->hSquare = hSquareGreen; done = TRUE; }
			if (pressedKeys[6] == TRUE && keysToPress[iteration][6] == TRUE) { pSelf->hCross = hCrossGreen; done = TRUE; }
			if (pressedKeys[7] == TRUE && keysToPress[iteration][7] == TRUE) { pSelf->hCircle = hCircleGreen; done = TRUE; }

			if (pressedKeys[0] == FALSE && keysToPress[iteration][0] == FALSE) { pSelf->hArrowLeft = hArrowLeft; }
			if (pressedKeys[1] == FALSE && keysToPress[iteration][1] == FALSE) { pSelf->hArrowDown = hArrowDown; }
			if (pressedKeys[2] == FALSE && keysToPress[iteration][2] == FALSE) { pSelf->hArrowUp = hArrowUp; }
			if (pressedKeys[3] == FALSE && keysToPress[iteration][3] == FALSE) { pSelf->hArrowRight = hArrowRight; }
			if (pressedKeys[4] == FALSE && keysToPress[iteration][4] == FALSE) { pSelf->hTriangle = hTriangle; }
			if (pressedKeys[4] == FALSE && keysToPress[iteration][4] == FALSE) { pSelf->hTriangle = hTriangle; }
			if (pressedKeys[5] == FALSE && keysToPress[iteration][5] == FALSE) { pSelf->hSquare = hSquare; }
			if (pressedKeys[6] == FALSE && keysToPress[iteration][6] == FALSE) { pSelf->hCross = hCross; }
			if (pressedKeys[7] == FALSE && keysToPress[iteration][7] == FALSE) { pSelf->hCircle = hCircle; }

			if ((pressedKeys[0] == TRUE && keysToPress[iteration][0] == FALSE)) { pSelf->hArrowLeft = hArrowLeftRed; done = FALSE; }
			if ((pressedKeys[1] == TRUE && keysToPress[iteration][1] == FALSE)) { pSelf->hArrowDown = hArrowDownRed; done = FALSE; }
			if ((pressedKeys[2] == TRUE && keysToPress[iteration][2] == FALSE)) { pSelf->hArrowUp = hArrowUpRed; done = FALSE; }
			if ((pressedKeys[3] == TRUE && keysToPress[iteration][3] == FALSE)) { pSelf->hArrowRight = hArrowRightRed; done = FALSE; }
			if ((pressedKeys[4] == TRUE && keysToPress[iteration][4] == FALSE)) { pSelf->hTriangle = hTriangleRed; done = FALSE; }
			if ((pressedKeys[5] == TRUE && keysToPress[iteration][5] == FALSE)) { pSelf->hSquare = hSquareRed; done = FALSE; }
			if ((pressedKeys[6] == TRUE && keysToPress[iteration][6] == FALSE)) { pSelf->hCross = hCrossRed; done = FALSE; }
			if ((pressedKeys[7] == TRUE && keysToPress[iteration][7] == FALSE)) { pSelf->hCircle = hCircleRed; done = FALSE; }

			if (pressedKeys[0] == FALSE && keysToPress[iteration][0] == TRUE) { done = FALSE; }
			if (pressedKeys[1] == FALSE && keysToPress[iteration][1] == TRUE) { done - FALSE; }
			if (pressedKeys[2] == FALSE && keysToPress[iteration][2] == TRUE) { done = FALSE; }
			if (pressedKeys[3] == FALSE && keysToPress[iteration][3] == TRUE) { done = FALSE; }
			if (pressedKeys[4] == FALSE && keysToPress[iteration][4] == TRUE) { done = FALSE; }
			if (pressedKeys[5] == FALSE && keysToPress[iteration][5] == TRUE) { done = FALSE; }
			if (pressedKeys[6] == FALSE && keysToPress[iteration][6] == TRUE) { done = FALSE; }
			if (pressedKeys[7] == FALSE && keysToPress[iteration][7] == TRUE) { done = FALSE; }
			//Sleep(100);
			InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
			Sleep(100);
		} while (!done);
		//Sleep(300);
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
		InitializeKeyStruct(pSelf);
		hThreadReceive = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartReceiveData, NULL, 0, 0);
		hThreadCheckState = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GameWindow_CompareResults, pSelf, 0, 0);
		break;
	case WM_DESTROY:
		GameWindow_FinalizeBackBuffer(pSelf);
		CloseHandle(hThreadReceive);
		CloseHandle(hThreadCheckState);
		Hid_Close();
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