#include <Windows.h>
#include <iostream>
#include <time.h>
#include "DanceMat.h"

#pragma comment (lib, "winmm.lib")

#define ARROW_WIDTH 96
#define ARROW_HEIGHT 96
#define NUMBERS_KEY_TO_PRESS 8

typedef struct _GameWindow
{
	HWND hWndSelf;
	RECT rcClient;

	HDC hdcBack;
	HBITMAP hbmBack;
	HDC hArrowUp, hArrowLeft, hArrowDown, hArrowRight, hSquare, hTriangle, hCircle, hCross;
} GameWindow, * PGameWindow;

ATOM GameWindow_RegisterClass(HINSTANCE hInstance);