#include <stdio.h>
#include <iostream>
#include <chrono>

#include <iostream>
#include <iomanip>
#include <sstream>

#pragma comment (lib, "hid.lib")
#pragma comment (lib, "setupapi.lib")

#include "EntryPoint.h"

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

void OnInput()
{
	std::string str;

	str = "";
	for (int i = 0; i < inReportSize; i++)
	{
		str = str + " " + IntToHex(inReport[i]);
	}

	std::cout << str;
}

void OnError()
{
	isRunning = false;
	Hid_Close();
	std::cout << "Device closed\n";
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
		std::cout << "\nThere are no connected devices";
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
		std::cout << "\nThere are no connected devices";
	}
}

int main()
{
	HID_Init();
	int num = FindDevices();
	printf("%d", num);
	FindDevice();
	Connect();
	//GetInputReport();
	//GetDataByTimer();

	std::chrono::steady_clock::time_point tend = std::chrono::steady_clock::now() + std::chrono::seconds(5);
	while (std::chrono::steady_clock::now() < tend)
	{
		GetDataByTimer();
	}

	return 1;
}