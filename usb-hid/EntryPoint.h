#include <Windows.h>
#include "GameWindow.h"

#define ARROW_WIDTH 96
#define ARROW_HEIGHT 96

HWND hWndGame;

HWND btnStart;

HINSTANCE hInst;
ATOM atomGame;

HDC hdcBack, hBackBmp;
HBITMAP hbmBack;
HDC hButton;
RECT clientRect;