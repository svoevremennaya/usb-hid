#include "GameWindow.h"

// Keys that should be pressed by user
BOOL ranKeysToPress[8];

HANDLE hThreadReceive, hThreadCheckState;

// Coordinates of the up pictures
int coordKeysToPress[2][8] = {
	{ 446, 542, 542, 638, 446, 638, 446, 638 },
	{ 106, 202, 10, 106, 202, 202, 10, 10 }
};

HDC hBack;
HDC hArrowUp, hArrowLeft, hArrowDown, hArrowRight, hSquare, hTriangle, hCircle, hCross;
HDC hArrowUpGreen, hArrowLeftGreen, hArrowDownGreen, hArrowRightGreen, hSquareGreen, hTriangleGreen, hCircleGreen, hCrossGreen;
HDC hArrowUpRed, hArrowLeftRed, hArrowDownRed, hArrowRightRed, hSquareRed, hTriangleRed, hCircleRed, hCrossRed;

HDC pictures[8];

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
	hBack = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"back_3.bmp");

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
	hSquareGreen = GameWindow_LoadBitmapDC(pSelf->hWndSelf, L"green\\square_green.bmp");
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
	StretchBlt(pSelf->hdcBack, 0, 0, pSelf->rcClient.right, pSelf->rcClient.bottom, hBack, 0, 0, 1000, 667, SRCCOPY);
	GdiTransparentBlt(pSelf->hdcBack, 542, 316, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowUp, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 638, 412, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowRight, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 542, 508, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowDown, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 446, 412, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hArrowLeft, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 638, 508, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hSquare, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 446, 508, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hTriangle, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 638, 316, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hCircle, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));
	GdiTransparentBlt(pSelf->hdcBack, 446, 316, ARROW_WIDTH, ARROW_HEIGHT, pSelf->hCross, 0, 0, ARROW_WIDTH, ARROW_HEIGHT, RGB(34, 177, 76));

	for (int i = 0; i < 8; i++)
	{
		if (ranKeysToPress[i])
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

void GameWindow_GenerateKeysToPress()
{
	int numKeysToPress, indKeyToPress;

	srand(time(NULL));
	numKeysToPress = 1 + rand() % 3;

	for (int i = 0; i < 8; i++)
	{
		ranKeysToPress[i] = FALSE;
	}

	for (int i = 0; i < numKeysToPress; i++)
	{
		srand(time(NULL));
		indKeyToPress = rand() % 8;
		ranKeysToPress[indKeyToPress] = TRUE;
	}
}

void GameWindow_ProcessData(PGameWindow pSelf)
{
	while (1)
	{
		GameWindow_GenerateKeysToPress();
		BOOL done = FALSE;
		do
		{
			if (pressedKeys[0] == TRUE && ranKeysToPress[0] == TRUE) { pSelf->hArrowLeft = hArrowLeftGreen; done = TRUE; }
			if (pressedKeys[1] == TRUE && ranKeysToPress[1] == TRUE) { pSelf->hArrowDown = hArrowDownGreen; done = TRUE; }
			if (pressedKeys[2] == TRUE && ranKeysToPress[2] == TRUE) { pSelf->hArrowUp = hArrowUpGreen; done = TRUE; }
			if (pressedKeys[3] == TRUE && ranKeysToPress[3] == TRUE) { pSelf->hArrowRight = hArrowRightGreen; done = TRUE; }
			if (pressedKeys[4] == TRUE && ranKeysToPress[4] == TRUE) { pSelf->hTriangle = hTriangleGreen; done = TRUE; }
			if (pressedKeys[5] == TRUE && ranKeysToPress[5] == TRUE) { pSelf->hSquare = hSquareGreen; done = TRUE; }
			if (pressedKeys[6] == TRUE && ranKeysToPress[6] == TRUE) { pSelf->hCross = hCrossGreen; done = TRUE; }
			if (pressedKeys[7] == TRUE && ranKeysToPress[7] == TRUE) { pSelf->hCircle = hCircleGreen; done = TRUE; }

			if (pressedKeys[0] == FALSE && ranKeysToPress[0] == FALSE) { pSelf->hArrowLeft = hArrowLeft; }
			if (pressedKeys[1] == FALSE && ranKeysToPress[1] == FALSE) { pSelf->hArrowDown = hArrowDown; }
			if (pressedKeys[2] == FALSE && ranKeysToPress[2] == FALSE) { pSelf->hArrowUp = hArrowUp; }
			if (pressedKeys[3] == FALSE && ranKeysToPress[3] == FALSE) { pSelf->hArrowRight = hArrowRight; }
			if (pressedKeys[4] == FALSE && ranKeysToPress[4] == FALSE) { pSelf->hTriangle = hTriangle; }
			if (pressedKeys[4] == FALSE && ranKeysToPress[4] == FALSE) { pSelf->hTriangle = hTriangle; }
			if (pressedKeys[5] == FALSE && ranKeysToPress[5] == FALSE) { pSelf->hSquare = hSquare; }
			if (pressedKeys[6] == FALSE && ranKeysToPress[6] == FALSE) { pSelf->hCross = hCross; }
			if (pressedKeys[7] == FALSE && ranKeysToPress[7] == FALSE) { pSelf->hCircle = hCircle; }

			if ((pressedKeys[0] == TRUE && ranKeysToPress[0] == FALSE)) { pSelf->hArrowLeft = hArrowLeftRed; done = FALSE; }
			if ((pressedKeys[1] == TRUE && ranKeysToPress[1] == FALSE)) { pSelf->hArrowDown = hArrowDownRed; done = FALSE; }
			if ((pressedKeys[2] == TRUE && ranKeysToPress[2] == FALSE)) { pSelf->hArrowUp = hArrowUpRed; done = FALSE; }
			if ((pressedKeys[3] == TRUE && ranKeysToPress[3] == FALSE)) { pSelf->hArrowRight = hArrowRightRed; done = FALSE; }
			if ((pressedKeys[4] == TRUE && ranKeysToPress[4] == FALSE)) { pSelf->hTriangle = hTriangleRed; done = FALSE; }
			if ((pressedKeys[5] == TRUE && ranKeysToPress[5] == FALSE)) { pSelf->hSquare = hSquareRed; done = FALSE; }
			if ((pressedKeys[6] == TRUE && ranKeysToPress[6] == FALSE)) { pSelf->hCross = hCrossRed; done = FALSE; }
			if ((pressedKeys[7] == TRUE && ranKeysToPress[7] == FALSE)) { pSelf->hCircle = hCircleRed; done = FALSE; }

			for (int j = 0; j < 8; j++)
			{
				if (pressedKeys[j] == FALSE && ranKeysToPress[j] == TRUE)
				{
					done = FALSE;
				}
			}
			InvalidateRect(pSelf->hWndSelf, NULL, TRUE);
			Sleep(200);
		} while (!done);

	}

	return;
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
		PlaySoundA("eiffel_blue.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
		LoadPictures(pSelf);
		InitializeKeyStruct(pSelf);
		hThreadReceive = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartReceiveData, NULL, 0, 0);
		hThreadCheckState = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GameWindow_ProcessData, pSelf, 0, 0);
		break;
	case WM_DESTROY:
		PlaySoundA(NULL, 0, 0);
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