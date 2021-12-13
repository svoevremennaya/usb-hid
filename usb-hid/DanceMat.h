#include <Windows.h>
#include <stdlib.h>
#include <string>

extern "C"
{
#include <hidsdi.h>
#include <setupapi.h>
}

#define DEV_NUM 10
#define VID 0x79
#define PID 0x11

#define ARROW_LEFT 0
#define ARROW_UP 1
#define ARROW_RIGHT 2
#define ARROW_DOWN 3
#define SQUARE 4
#define TRIANGLE 5
#define CIRCLE 6
#define CROSS 7
#define SELECT 8
#define START 9
#define EMPTY 10

typedef struct _key
{
	BYTE keyId;
	HDC hGreen;
	HDC hRed;
} Key, *PKey;

extern Key arrowLeft, arrowRight, arrowDown, arrowUp, circle, triangle, square, cross, selectKey, startKey, empty;

const std::string ARROW_LEFT_STR = " 0 1 7f 7f 7f 7f 1f 0 0";
const std::string ARROW_UP_STR = " 0 1 7f 7f 7f 7f 4f 0 0";
const std::string ARROW_RIGHT_STR = " 0 1 7f 7f 7f 7f 8f 0 0";
const std::string ARROW_DOWN_STR = " 0 1 7f 7f 7f 7f 2f 0 0";
const std::string SQUARE_STR = " 0 1 7f 7f 7f 7f f 2 0";
const std::string TRIANGLE_STR = " 0 1 7f 7f 7f 7f f 1 0";
const std::string CIRCLE_STR = " 0 1 7f 7f 7f 7f f 8 0";
const std::string CROSS_STR = " 0 1 7f 7f 7f 7f f 4 0";
const std::string SELECT_STR = " 0 1 7f 7f 7f 7f f 10 0";
const std::string START_STR = " 0 1 7f 7f 7f 7f f 20 0";
const std::string EMPTY_STR = " 0 1 7f 7f 7f 7f f 0 0";

extern std::string pressedKeyStr;
extern Key pressedKey;

extern std::string strPrev;
extern int pressedRrev;

void HID_Init();
void HID_UnInit();
int FindDevices();
BOOL Hid_Open(int num);
void Hid_Close();
BOOL Hid_Read(BYTE* buffer, DWORD size, DWORD* bytesRead);
int FindDevice();
void StartReceiveData();