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

BOOL isRunning;
BYTE* inReport;
int	inReportSize;

int devCount; // number of hid-devices
int devSelected; // chosen device
PSP_DEVICE_INTERFACE_DETAIL_DATA devDetailData[DEV_NUM]; // the path for a device interface
int devDetailDataSize[DEV_NUM];
int devInputReportSize[DEV_NUM];
HANDLE hDevice;
HANDLE hSelected;

OVERLAPPED devReadOverlapped;
BOOL devReadPending;

std::string strPrev;

void HID_Init();
void HID_UnInit();
int FindDevices();
BOOL Hid_Open(int num);
void Hid_Close();
BOOL Hid_Read(BYTE* buffer, DWORD size, DWORD* bytesRead);
int FindDevice();