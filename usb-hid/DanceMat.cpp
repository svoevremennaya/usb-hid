#include <stdio.h>
#include <iostream>
#include <chrono>

#include <iostream>
#include <iomanip>
#include <sstream>

#pragma comment (lib, "hid.lib")
#pragma comment (lib, "setupapi.lib")

#include "DanceMat.h"

std::string pressedKeyStr = "q";
int pressedKey = -1;
int pressedPrev = -1;

std::string strPrev = "w";

BOOL isRunning;
BYTE* inReport;
int	inReportSize;

OVERLAPPED devReadOverlapped;
BOOL devReadPending;

int devCount; // number of hid-devices
int devSelected; // chosen device
PSP_DEVICE_INTERFACE_DETAIL_DATA devDetailData[DEV_NUM]; // the path for a device interface
int devDetailDataSize[DEV_NUM];
int devInputReportSize[DEV_NUM];
HANDLE hDevice;
HANDLE hSelected;

// HID Initialization
void HID_Init()
{
	int  i;

	devCount = 0;																// �������� ���������� ��������� ���������
	devSelected = -1;															// ���������� ��������� ����������
	for (i = 0; i < DEV_NUM; i++)												// ���������� ���� ������ � ����������� �� ���������, �������� ���� � �����������
	{
		devDetailData[i] = NULL;												// �������� ��������� ��������� �� ���������, �������� ���� � ����������
	}
}

// HID UnInitialization
void HID_UnInit()
{
	int  i;

	for (i = 0; i < DEV_NUM; i++)												// ���������� ���� ������ � ����������� �� ���������, �������� ���� � �����������
	{
		if (devDetailData[i])
		{
			free(devDetailData[i]);												// ������� ������ ���������, �������� ���� � ����������
		}
	}
}

// Find Devices
int FindDevices()
{
	GUID hidGuid;
	HANDLE hDevice;
	HDEVINFO devInfo;
	SP_DEVICE_INTERFACE_DATA devData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA devDetail;
	PSP_DEVICE_INTERFACE_DETAIL_DATA devDetailSelected;
	PHIDP_PREPARSED_DATA preparsedData;
	HIDD_ATTRIBUTES attributes;
	HIDP_CAPS capabilities;
	ULONG length;
	int ind;
	int size;
	BOOL ok;

	HidD_GetHidGuid(&hidGuid);
	devInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	devData.cbSize = sizeof(devData);

	devDetail = NULL;
	if (devSelected != -1)
	{
		devDetailSelected = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(devDetailDataSize[devSelected]);
		memcpy(devDetailSelected, devDetailData[devSelected], devDetailDataSize[devSelected]);
	}
	else
	{
		devDetailSelected = NULL;
	}

	// ���������� ��� ��������� �� ��������� ������ � ������
	for (ind = 0; ind < DEV_NUM; ind++)
	{
		if (devDetailData[ind])
		{
			free(devDetailData[ind]);
			devDetailData[ind] = NULL;
		}
	}

	/* Scan all Devices */

	ind = -1;
	devCount = 0;
	hDevice = INVALID_HANDLE_VALUE;

	do
	{
		ind++;
		if (hDevice != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hDevice);
		}

		ok = SetupDiEnumDeviceInterfaces(devInfo, 0, &hidGuid, ind, &devData);
		if (!ok)
		{
			break;
		}

		
		ok = SetupDiGetDeviceInterfaceDetail(devInfo, &devData, NULL, 0, &length, NULL);
		// allocation memory for buffer with detail data
		if (devDetail)
		{
			free(devDetail);
		}
		devDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(length);
		size = length;
		devDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// getting path to device
		ok = SetupDiGetDeviceInterfaceDetail(devInfo, &devData, devDetail, length, NULL, NULL);
		if (!ok)
		{
			continue;
		}

		// checking the availability of the device
		hDevice = CreateFile(devDetail->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, 0, NULL);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			continue;
		}

		attributes.Size = sizeof(attributes);
		ok = HidD_GetAttributes(hDevice, &attributes);
		if (!ok) { continue; }

		devCount++;
		devDetail = NULL;
		CloseHandle(hDevice);
		hDevice = INVALID_HANDLE_VALUE;

	} while (devCount < DEV_NUM);

	if (devDetail)
	{
		free(devDetail);
	}

	if (devDetailSelected)
	{
		free(devDetailSelected);
	}

	SetupDiDestroyDeviceInfoList(devInfo);
	return devCount;
}

BOOL Hid_Open(int num)
{
	ULONG numBuffers;

	if (devDetailData[num] == NULL) { return FALSE; }

	hDevice = CreateFile(devDetailData[num]->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hDevice == INVALID_HANDLE_VALUE) { return FALSE; }
	devSelected = num;

	return TRUE;
}

void Hid_Close()
{
	devSelected = -1;
	CancelIo(hDevice);
	devReadPending = FALSE;

	if (hDevice != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hDevice);
		hDevice = INVALID_HANDLE_VALUE;
	}
}

BOOL Hid_Read(BYTE* buffer, DWORD size, DWORD* bytesRead)
{
	int lastError;

	if (!devReadPending)
	{
		if (ReadFile(hDevice, buffer, size, bytesRead, &devReadOverlapped))
		{
			return(true);
		}
		devReadPending = true;
	}
	else
	{
		if (GetOverlappedResult(hDevice, &devReadOverlapped, bytesRead, false))
		{
			devReadPending = false;
			return true;
		}
	}

	lastError = GetLastError();
	if (lastError != ERROR_IO_INCOMPLETE && lastError != ERROR_IO_PENDING)
	{
		devReadPending = false;
		CancelIo(hDevice);
		return false;
	}
	return true;
}

int FindDevice()
{
	GUID hidGuid;
	HANDLE hDevice;
	HDEVINFO devInfo;
	SP_DEVICE_INTERFACE_DATA devData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA devDetail;
	PSP_DEVICE_INTERFACE_DETAIL_DATA devDetailSelected;
	PHIDP_PREPARSED_DATA preparsedData;
	HIDD_ATTRIBUTES attributes;
	HIDP_CAPS capabilities;
	ULONG length;
	int ind;
	int size;
	BOOL ok;

	HidD_GetHidGuid(&hidGuid);
	devInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	devData.cbSize = sizeof(devData);

	devDetail = NULL;
	if (devSelected != -1)
	{
		devDetailSelected = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(devDetailDataSize[devSelected]);
		memcpy(devDetailSelected, devDetailData[devSelected], devDetailDataSize[devSelected]);
	}
	else
	{
		devDetailSelected = NULL;
	}

	// ���������� ��� ��������� �� ��������� ������ � ������
	for (ind = 0; ind < DEV_NUM; ind++)
	{
		if (devDetailData[ind])
		{
			free(devDetailData[ind]);
			devDetailData[ind] = NULL;
		}
	}

	/* Scan all Devices */

	ind = -1;
	devCount = 0;
	hDevice = INVALID_HANDLE_VALUE;

	do
	{
		ind++;
		if (hDevice != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hDevice);
		}

		ok = SetupDiEnumDeviceInterfaces(devInfo, 0, &hidGuid, ind, &devData);
		if (!ok)
		{
			break;
		}


		ok = SetupDiGetDeviceInterfaceDetail(devInfo, &devData, NULL, 0, &length, NULL);
		// allocation memory for buffer with detail data
		if (devDetail)
		{
			free(devDetail);
		}
		devDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(length);
		size = length;
		devDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// getting path to device
		ok = SetupDiGetDeviceInterfaceDetail(devInfo, &devData, devDetail, length, NULL, NULL);
		if (!ok)
		{
			continue;
		}

		// checking the availability of the device
		hDevice = CreateFile(devDetail->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, 0, NULL);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			continue;
		}

		attributes.Size = sizeof(attributes);
		ok = HidD_GetAttributes(hDevice, &attributes);
		if (!ok) { continue; }

		if (attributes.VendorID == VID && attributes.ProductID == PID)
		{
			hSelected = hDevice;
			devSelected = devCount;
			printf("found");
		}

		ok = HidD_GetPreparsedData(hDevice, &preparsedData);					// ����������� ���� ������ ��� ��������� ��������
		if (!ok)	continue;

		ok = HidP_GetCaps(preparsedData, &capabilities);   						// �������� ����������� �� ����������� �������
		if (!ok) continue;

		devDetailData[devCount] = devDetail;
		devDetailDataSize[devCount] = size;
		devInputReportSize[devCount] = capabilities.InputReportByteLength;

		devCount++;
		devDetail = NULL;
		CloseHandle(hDevice);
		hDevice = INVALID_HANDLE_VALUE;

	} while (devCount < DEV_NUM);

	if (devDetail) { free(devDetail); }

	if (devDetailSelected) { free(devDetailSelected); }

	SetupDiDestroyDeviceInfoList(devInfo);
	return devCount;
}

void Connect()
{
	if (Hid_Open(devSelected))
	{
		inReportSize = devInputReportSize[devSelected];
		if (inReport) { free(inReport); }
		inReport = (BYTE*)calloc(inReportSize, 1);
		isRunning = true;
	}
	else
	{
		std::cout << "Cannot open device\n";
	}
}

std::string IntToHex(const int i) {
	std::ostringstream ost;
	ost << std::hex << i;
	return ost.str();
}

int Output(std::string &str)
{
	if (str == SQUARE_STR)
	{
		std::cout << "|_|" << std::endl;
		pressedKeyStr = SQUARE_STR;
		pressedKey = SQUARE;
	}
	else if (str == ARROW_DOWN_STR)
	{
		std::cout << "\\/" << std::endl;
		pressedKeyStr = ARROW_DOWN_STR;
		pressedKey = ARROW_DOWN;
	}
	else if (str == TRIANGLE_STR)
	{
		std::cout << "/_\\" << std::endl;
		pressedKeyStr = TRIANGLE_STR;
		pressedKey = TRIANGLE;
	}
	else if (str == ARROW_LEFT_STR)
	{
		std::cout << "<-" << std::endl;
		pressedKeyStr = ARROW_LEFT_STR;
		pressedKey = ARROW_LEFT;
	}
	else if (str == CROSS_STR)
	{
		std::cout << "cross" << std::endl;
		pressedKeyStr = CROSS_STR;
		pressedKey = CROSS;
	}
	else if (str == ARROW_UP_STR)
	{
		std::cout << "/\\" << std::endl;
		pressedKeyStr = ARROW_UP_STR;
		pressedKey = ARROW_UP;
	}
	else if (str == CIRCLE_STR)
	{
		std::cout << "circle" << std::endl;
		pressedKeyStr = CIRCLE_STR;
		pressedKey = CIRCLE;
	}
	else if (str == ARROW_RIGHT_STR)
	{
		std::cout << "->" << std::endl;
		pressedKeyStr = ARROW_RIGHT_STR;
		pressedKey = ARROW_RIGHT;
	}
	else if (str == SELECT_STR)
	{
		std::cout << "select" << std::endl;
		pressedKeyStr = SELECT_STR;
		pressedKey = SELECT;
	}
	else if (str == START_STR)
	{
		std::cout << "start" << std::endl;
		pressedKeyStr = START_STR;
		pressedKey = START;
	}
	else if (str == EMPTY_STR)
	{
		pressedKeyStr = EMPTY_STR;
		pressedKey = EMPTY;
	}
	return pressedKey;
}

void OnInput()
{
	std::string str;

	str = "";
	for (int i = 0; i < inReportSize; i++)
	{
		str = str + " " + IntToHex(inReport[i]);
	}
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! pressedPrev must be with strPrev
	if (str != strPrev)
	{
		pressedPrev = Output(str);
	}
	strPrev = str;
	//Output(str);
}

void OnError()
{
	isRunning = false;
	Hid_Close();
	std::cout << "Device closed\n" << std::endl;
}

// doesnt work
void GetInputReport()
{
	DWORD bytesRead;
	if (isRunning)
	{
		if (HidD_GetInputReport(hDevice, inReport, inReportSize))
		{
			OnInput();
		}
		else
		{
			OnError();
		}
	}
	else
	{
		std::cout << "\nThere are no connected devices" << std::endl;
	}
}

void GetDataByTimer()
{
	DWORD bytesRead;
	if (isRunning)
	{
		if (Hid_Read(inReport, inReportSize, &bytesRead))
		{
			if (bytesRead)
			{
				OnInput();
			}
		}
		else
		{
			OnError();
		}
	}
	else
	{
		std::cout << "\nThere are no connected devices" << std::endl;
	}
}

void StartReceiveData()
{
	strPrev = "";
	pressedKey = -1;
	pressedKeyStr = "qwerty";
	HID_Init();
	FindDevice();
	Connect();

	while (1)
	{
		GetDataByTimer();
	}
}

//int main()
//{
//	strPrev = "";
//	HID_Init();
//	int num = FindDevices();
//	printf("%d", num);
//	FindDevice();
//	Connect();
//	//GetInputReport();
//	//GetDataByTimer();
//
//	std::chrono::steady_clock::time_point tend = std::chrono::steady_clock::now() + std::chrono::seconds(10);
//	while (std::chrono::steady_clock::now() < tend)
//	{
//		GetDataByTimer();
//	}
//
//	/*int i = 0;
//	while (i < 20)
//	{
//		Sleep(250);
//		GetDataByTimer();
//		i++;
//	}*/
//
//	return 1;
//}