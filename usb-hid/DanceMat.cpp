#include <stdio.h>
#include <iostream>
#include <chrono>

#include <iostream>
#include <iomanip>
#include <sstream>

#pragma comment (lib, "hid.lib")
#pragma comment (lib, "setupapi.lib")

#include "DanceMat.h"

Key arrowLeft, arrowRight, arrowDown, arrowUp, circle, triangle, square, cross, selectKey, startKey, empty, centre;

//						AL        AD     AU     AR      T     SQ     CR    CIR     ST     SE    CEN
BOOL pressedKeys[11] = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE };

std::string pressedKeyStr = "q";
//int pressedKey = -1;
Key pressedKey;
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

	devCount = 0;																// обнуляем количество найденных устройств
	devSelected = -1;															// сбрасываем выбранное устройство
	for (i = 0; i < DEV_NUM; i++)												// перебираем весь массив с указателями на структуры, хранящие пути к устройствам
	{
		devDetailData[i] = NULL;												// обнуляем очередной указатель на структуру, хранящую путь к устройству
	}
}

// HID UnInitialization
void HID_UnInit()
{
	int  i;

	for (i = 0; i < DEV_NUM; i++)												// перебираем весь массив с указателями на структуры, хранящие пути к устройствам
	{
		if (devDetailData[i])
		{
			free(devDetailData[i]);												// очищаем память структуры, хранящей путь к устройству
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

	// сбрасываем все указатели на детальные данные в списке
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

	// сбрасываем все указатели на детальные данные в списке
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

		ok = HidD_GetPreparsedData(hDevice, &preparsedData);					// запрашиваем блок данных для обработки репортов
		if (!ok)	continue;

		ok = HidP_GetCaps(preparsedData, &capabilities);   						// получаем возможности из дескриптора репорта
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
	if (str == SQUARE_STR || str == SQUARE_STR_CEN)
	{
		std::cout << "|_|" << std::endl;
		pressedKeyStr = SQUARE_STR;
		pressedKey = square;
	}
	else if (str == ARROW_DOWN_STR || str == ARROW_DOWN_STR_CEN)
	{
		std::cout << "\\/" << std::endl;
		pressedKeyStr = ARROW_DOWN_STR;
		pressedKey = arrowDown;
	}
	else if (str == TRIANGLE_STR || str == TRIANGLE_STR_CEN)
	{
		std::cout << "/_\\" << std::endl;
		pressedKeyStr = TRIANGLE_STR;
		pressedKey = triangle;
	}
	else if (str == ARROW_LEFT_STR || str == ARROW_LEFT_STR_CEN)
	{
		std::cout << "<-" << std::endl;
		pressedKeyStr = ARROW_LEFT_STR;
		pressedKey = arrowLeft;
	}
	else if (str == CROSS_STR || str == CROSS_STR_CEN)
	{
		std::cout << "cross" << std::endl;
		pressedKeyStr = CROSS_STR;
		pressedKey = cross;
	}
	else if (str == ARROW_UP_STR || str == ARROW_UP_STR_CEN)
	{
		std::cout << "/\\" << std::endl;
		pressedKeyStr = ARROW_UP_STR;
		pressedKey = arrowUp;
	}
	else if (str == CIRCLE_STR || str == CIRCLE_STR_CEN)
	{
		std::cout << "circle" << std::endl;
		pressedKeyStr = CIRCLE_STR;
		pressedKey = circle;
	}
	else if (str == ARROW_RIGHT_STR || str == ARROW_RIGHT_STR_CEN)
	{
		std::cout << "->" << std::endl;
		pressedKeyStr = ARROW_RIGHT_STR;
		pressedKey = arrowRight;
	}
	else if (str == SELECT_STR || str == SELECT_STR_CEN)
	{
		std::cout << "select" << std::endl;
		pressedKeyStr = SELECT_STR;
		pressedKey = selectKey;
	}
	else if (str == START_STR || str == START_STR_CEN)
	{
		std::cout << "start" << std::endl;
		pressedKeyStr = START_STR;
		pressedKey = startKey;
	}
	else if (str == EMPTY_STR)
	{
		std::cout << "empty" << std::endl;
		pressedKeyStr = EMPTY_STR;
		pressedKey = empty;
	}
	else if (str == CENTRE_STR)
	{
		std::cout << "centre" << std::endl;
		pressedKeyStr = CENTRE_STR;
		pressedKey = centre;
	}
	return pressedKey.keyId;
}

std::string OnInput()
{
	std::string str;

	str = "";
	for (int i = 0; i < inReportSize; i++)
	{
		str = str + " " + IntToHex(inReport[i]);
		BYTE byte = inReport[i];
		if (i == 5)
		{
			if (byte == 0x0f)
			{
				pressedKeys[10] = TRUE;
			}
			else
			{
				pressedKeys[10] = FALSE;
			}
		}
		if (i == 6)
		{
			if (byte == 0x0f) { pressedKeys[0] = FALSE; pressedKeys[1] = FALSE; pressedKeys[2] = FALSE; pressedKeys[3] = FALSE; }
			if (byte == 0x1f) { pressedKeys[0] = TRUE; pressedKeys[1] = FALSE; pressedKeys[2] = FALSE; pressedKeys[3] = FALSE; }
			if (byte == 0x2f) { pressedKeys[1] = TRUE; pressedKeys[0] = FALSE; pressedKeys[2] = FALSE; pressedKeys[3] = FALSE; }
			if (byte == 0x3f) { pressedKeys[1] = TRUE; pressedKeys[0] = TRUE; pressedKeys[2] = FALSE; pressedKeys[3] = FALSE; }
			if (byte == 0x4f) { pressedKeys[2] = TRUE; pressedKeys[0] = FALSE; pressedKeys[1] = FALSE;  pressedKeys[3] = FALSE; }
			if (byte == 0x5f) { pressedKeys[0] = TRUE; pressedKeys[2] = TRUE; pressedKeys[1] = FALSE;  pressedKeys[3] = FALSE; }
			if (byte == 0x6f) { pressedKeys[1] = TRUE; pressedKeys[2] = TRUE;  pressedKeys[0] = FALSE;  pressedKeys[3] = FALSE; }
			if (byte == 0x7f) { pressedKeys[0] = TRUE; pressedKeys[1] = TRUE; pressedKeys[2] = TRUE; pressedKeys[3] = FALSE; }
			if (byte == 0x8f) { pressedKeys[3] = TRUE; pressedKeys[0] = FALSE; pressedKeys[1] = FALSE; pressedKeys[2] = FALSE; }
			if (byte == 0x9f) { pressedKeys[0] = TRUE; pressedKeys[3] = TRUE; pressedKeys[1] = FALSE; pressedKeys[2] = FALSE;  }
			if (byte == 0xaf) { pressedKeys[1] = TRUE; pressedKeys[3] = TRUE; pressedKeys[0] = FALSE; pressedKeys[2] = FALSE;  }
			if (byte == 0xbf) { pressedKeys[0] = TRUE; pressedKeys[1] = TRUE; pressedKeys[3] = TRUE; pressedKeys[2] = FALSE; }
			if (byte == 0xcf) { pressedKeys[2] = TRUE; pressedKeys[3] = TRUE; pressedKeys[0] = FALSE; pressedKeys[1] = FALSE;  }
			if (byte == 0xdf) { pressedKeys[0] = TRUE; pressedKeys[2] = TRUE; pressedKeys[3] = TRUE; pressedKeys[1] = FALSE; }
			if (byte == 0xef) { pressedKeys[1] = TRUE; pressedKeys[2] = TRUE; pressedKeys[3] = TRUE; pressedKeys[0] = FALSE; }
			if (byte == 0xff) { pressedKeys[0] = TRUE; pressedKeys[1] = TRUE; pressedKeys[2] = TRUE; pressedKeys[3] = TRUE; }
		}
		if (i == 7)
		{
			if (byte == 0x00) { pressedKeys[4] = FALSE; pressedKeys[5] = FALSE; pressedKeys[6] = FALSE; pressedKeys[7] = FALSE; }
			if (byte == 0x01) { pressedKeys[4] = TRUE; pressedKeys[5] = FALSE; pressedKeys[6] = FALSE; pressedKeys[7] = FALSE; }
			if (byte == 0x02) { pressedKeys[5] = TRUE; pressedKeys[4] = FALSE; pressedKeys[6] = FALSE; pressedKeys[7] = FALSE; }
			if (byte == 0x03) { pressedKeys[5] = TRUE; pressedKeys[4] = TRUE; pressedKeys[6] = FALSE; pressedKeys[7] = FALSE; }
			if (byte == 0x04) { pressedKeys[6] = TRUE; pressedKeys[4] = FALSE; pressedKeys[5] = FALSE; pressedKeys[7] = FALSE; }
			if (byte == 0x05) { pressedKeys[4] = TRUE; pressedKeys[6] = TRUE; pressedKeys[5] = FALSE; pressedKeys[7] = FALSE; }
			if (byte == 0x06) { pressedKeys[5] = TRUE; pressedKeys[6] = TRUE; pressedKeys[4] = FALSE; pressedKeys[7] = FALSE; }
			if (byte == 0x07) { pressedKeys[4] = TRUE; pressedKeys[5] = TRUE; pressedKeys[6] = TRUE; pressedKeys[7] = FALSE; }
			if (byte == 0x08) { pressedKeys[7] = TRUE; pressedKeys[4] = FALSE; pressedKeys[5] = FALSE; pressedKeys[6] = FALSE; }
			if (byte == 0x09) { pressedKeys[4] = TRUE; pressedKeys[7] = TRUE; pressedKeys[5] = FALSE; pressedKeys[6] = FALSE; }
			if (byte == 0x0a) { pressedKeys[5] = TRUE; pressedKeys[7] = TRUE; pressedKeys[4] = FALSE; pressedKeys[6] = FALSE; }
			if (byte == 0x0b) { pressedKeys[4] = TRUE; pressedKeys[5] = TRUE; pressedKeys[7] = TRUE; pressedKeys[6] = FALSE; }
			if (byte == 0x0c) { pressedKeys[6] = TRUE; pressedKeys[7] = TRUE; pressedKeys[4] = FALSE; pressedKeys[5] = FALSE; }
			if (byte == 0x0d) { pressedKeys[4] = TRUE; pressedKeys[6] = TRUE; pressedKeys[7] = TRUE; pressedKeys[5] = FALSE; }
			if (byte == 0x0e) { pressedKeys[5] = TRUE; pressedKeys[6] = TRUE; pressedKeys[7] = TRUE; pressedKeys[4] = FALSE; }
			if (byte == 0x0f) { pressedKeys[4] = TRUE; pressedKeys[5] = TRUE; pressedKeys[6] = TRUE; pressedKeys[7] = TRUE; }

			if ((byte & 0b00100000) == 0x20) { pressedKeys[8] = TRUE; }
			if ((byte & 0b00010000) == 0x10) { pressedKeys[9] = TRUE; }
			if ((byte & 0b00110000) == 0x30) { pressedKeys[8] = TRUE; pressedKeys[9] = TRUE; }
		}
	}
	
	if (str != strPrev)
	{
		pressedPrev = pressedKey.keyId;
		strPrev = pressedKeyStr;
		Output(str);
	}
	pressedPrev = pressedKey.keyId;
	strPrev = pressedKeyStr;
	return str;
}

void OnError()
{
	isRunning = false;
	Hid_Close();
	std::cout << "Device closed\n" << std::endl;
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
	pressedKey.keyId = -1;
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