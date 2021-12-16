#include <Windows.h>

#define ARROW_WIDTH 96
#define ARROW_HEIGHT 96

HWND hWndGame;

HWND btnStart;

HINSTANCE hInst;
ATOM atomGame;

HDC hdcBack;
HBITMAP hbmBack;
RECT clientRect;