#include <Windows.h>
#include <iostream>
#include "DanceMat.h"

#define ARROW_WIDTH 96
#define ARROW_HEIGHT 96
#define NUMBERS_KEY_TO_PRESS 7

BOOL keysToPress[NUMBERS_KEY_TO_PRESS][8] =
{
	// AL    AD      AU     AR     T     SQ     CR     CIR
	{ FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE },
	{ TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },
	{ FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },
	{ FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE},
	{ TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE },
	{ FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE },
	{ FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE }
};

HDC buf;
HANDLE hThreadReceive, hThreadCheckState;

int iteration = 0;

HDC hArrowUp, hArrowLeft, hArrowDown, hArrowRight, hSquare, hTriangle, hCircle, hCross;
HDC hArrowUpGreen, hArrowLeftGreen, hArrowDownGreen, hArrowRightGreen, hSquareGreen, hTriangleGreen, hCircleGreen, hCrossGreen;
HDC hArrowUpRed, hArrowLeftRed, hArrowDownRed, hArrowRightRed, hSquareRed, hTriangleRed, hCircleRed, hCrossRed;
HDC prev;
HDC* changed;

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
	hArrowUp = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"arrow_up.bmp");
	hArrowRight = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"arrow_right.bmp");
	hArrowDown = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"arrow_down.bmp");
	hArrowLeft = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"arrow_left.bmp");
	hSquare = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"square.bmp");
	hTriangle = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"triangle.bmp");
	hCircle = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"circle.bmp");
	hCross = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"cross.bmp");

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
void InitializeKeyStruct(PGameWindow pSelf)
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
	centre.keyId = CENTRE;

	pSelf->hArrowUp = hArrowUp;
	pSelf->hArrowDown = hArrowDown;
	pSelf->hArrowLeft = hArrowLeft;
	pSelf->hArrowRight = hArrowRight;
	pSelf->hTriangle = hTriangle;
	pSelf->hSquare = hSquare;
	pSelf->hCross = hCross;
	pSelf->hCircle = hCircle;

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
	FillRect(pSelf->hdcBack, &pSelf->rcClient, (HBRUSH)(CreateSolidBrush(RGB(40, 187, 253))));
	GdiTransparentBlt(pSelf->hdcBack, 452, 296, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowUp, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 548, 392, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowRight, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 452, 488, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowDown, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 356, 392, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowLeft, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 548, 488, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hSquare, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 356, 488, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hTriangle, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 548, 296, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hCircle, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 356, 296, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hCross, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));

	for (int i = 0; i < 8; i++)
	{
		if (keysToPress[iteration][i])
		{
			GdiTransparentBlt(pSelf->hdcBack, 100, 100, ARROW_WIDTH, ARROW_HEIGHT, pictures[i], 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
		}
	}
}

void CheckState3(PGameWindow pSelf)
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

void CompareResults(PGameWindow pSelf)
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

void CheckState(PGameWindow pSelf)
{
	changed = &(pSelf->hArrowDown);
	buf = pSelf->hArrowDown;

	while (1)
	{
		if (strPrev != pressedKeyStr)
		{
			switch (pressedKey.keyId)
			{
			case ARROW_UP:
				buf = pSelf->hArrowUp;
				pSelf->hArrowUp = hArrowUpRed;
				changed = &(pSelf->hArrowUp);
				//InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				//Sleep(100);
				//pSelf->hArrowUp = buf;
				break;
			case ARROW_LEFT:
				buf = pSelf->hArrowLeft;
				pSelf->hArrowLeft = hArrowLeftRed;
				changed = &(pSelf->hArrowLeft);
				/*InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				Sleep(100);
				pSelf->hArrowLeft = buf;*/
				break;
			case ARROW_DOWN:
				buf = pSelf->hArrowDown;
				pSelf->hArrowDown = hArrowDownRed;
				changed = &(pSelf->hArrowDown);
				/*InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				Sleep(100);
				pSelf->hArrowDown = buf;*/
				break;
			case ARROW_RIGHT:
				buf = pSelf->hArrowRight;
				pSelf->hArrowRight = hArrowRightRed;
				changed = &(pSelf->hArrowRight);
				/*InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				Sleep(100);
				pSelf->hArrowRight = buf;*/
				break;
			case SQUARE:
				buf = pSelf->hSquare;
				pSelf->hSquare = hSquareRed;
				changed = &(pSelf->hSquare);
				/*InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				Sleep(100);
				pSelf->hSquare = buf;*/
				break;
			case TRIANGLE:
				buf = pSelf->hTriangle;
				pSelf->hTriangle = hTriangleRed;
				changed = &(pSelf->hTriangle);
				/*InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				Sleep(100);
				pSelf->hTriangle = buf;*/
				break;
			case CIRCLE:
				buf = pSelf->hCircle;
				pSelf->hCircle = hCircleRed;
				changed = &(pSelf->hCircle);
				/*InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				Sleep(100);
				pSelf->hCircle = buf;*/
				break;
			case CROSS:
				buf = pSelf->hCross;
				pSelf->hCross = hCrossRed;
				changed = &(pSelf->hCross);
				/*InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
				Sleep(100);
				pSelf->hCross = buf;*/
				break;
			case EMPTY:
			case CENTRE:
				*changed = buf;
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
//void CompareResults(PGameWindow pSelf)
//{
//	for (int i = 0; i < NUMBERS_KEY_TO_PRESS; i++)
//	{
//		do
//		{
//			if (pressedKey.keyId == keysToPress[i])
//			{
//				switch (pressedKey.keyId)
//				{
//				case ARROW_UP:
//					buf = pSelf->hArrowUp;
//					pSelf->hArrowUp = pressedKey.hGreen;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hArrowUp = buf;
//					break;
//				case ARROW_LEFT:
//					buf = pSelf->hArrowLeft;
//					pSelf->hArrowLeft = pressedKey.hGreen;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hArrowLeft = buf;
//					break;
//				case ARROW_DOWN:
//					buf = pSelf->hArrowDown;
//					pSelf->hArrowDown = pressedKey.hGreen;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hArrowDown = buf;
//					break;
//				case ARROW_RIGHT:
//					buf = pSelf->hArrowRight;
//					pSelf->hArrowRight = pressedKey.hGreen;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hArrowRight = buf;
//					break;
//				case SQUARE:
//					buf = pSelf->hSquare;
//					pSelf->hSquare = pressedKey.hGreen;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hSquare = buf;
//					break;
//				case TRIANGLE:
//					buf = pSelf->hTriangle;
//					pSelf->hTriangle = pressedKey.hGreen;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hTriangle = buf;
//					break;
//				case CIRCLE:
//					buf = pSelf->hCircle;
//					pSelf->hCircle = pressedKey.hGreen;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hCircle = buf;
//					break;
//				case CROSS:
//					buf = pSelf->hCross;
//					pSelf->hCross = pressedKey.hGreen;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hCross = buf;
//					break;
//				}
//			}
//			else
//			{
//				switch (pressedKey.keyId)
//				{
//				case ARROW_UP:
//					buf = pSelf->hArrowUp;
//					pSelf->hArrowUp = pressedKey.hRed;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hArrowUp = buf;
//					break;
//				case ARROW_LEFT:
//					buf = pSelf->hArrowLeft;
//					pSelf->hArrowLeft = pressedKey.hRed;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hArrowLeft = buf;
//					break;
//				case ARROW_DOWN:
//					buf = pSelf->hArrowDown;
//					pSelf->hArrowDown = pressedKey.hRed;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hArrowDown = buf;
//					break;
//				case ARROW_RIGHT:
//					buf = pSelf->hArrowRight;
//					pSelf->hArrowRight = pressedKey.hRed;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hArrowRight = buf;
//					break;
//				case SQUARE:
//					buf = pSelf->hSquare;
//					pSelf->hSquare = pressedKey.hRed;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hSquare = buf;
//					break;
//				case TRIANGLE:
//					buf = pSelf->hTriangle;
//					pSelf->hTriangle = pressedKey.hRed;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hTriangle = buf;
//					break;
//				case CIRCLE:
//					buf = pSelf->hCircle;
//					pSelf->hCircle = pressedKey.hRed;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hCircle = buf;
//					break;
//				case CROSS:
//					buf = pSelf->hCross;
//					pSelf->hCross = pressedKey.hRed;
//					InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//					Sleep(100);
//					pSelf->hCross = buf;
//					break;
//				}
//			}
//			InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
//			//Sleep(200);
//		} while (pressedKey.keyId != keysToPress[i]);
//	}
//}

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