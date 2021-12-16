#include <Windows.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

#pragma comment (lib, "hid.lib")
#pragma comment (lib, "setupapi.lib")

extern "C"
{
#include <hidsdi.h>
#include <setupapi.h>
}

#define DEV_NUM 10
#define VID 0x79
#define PID 0x11

extern BOOL pressedKeys[11];

void HID_Init();
void HID_UnInit();
BOOL Hid_Open(int num);
void Hid_Close();
BOOL Hid_Read(BYTE* buffer, DWORD size, DWORD* bytesRead);
int FindDevice();
void StartReceiveData();