#include "DanceMat.h"

//						  AL      AD     AU     AR      T     SQ     CR    CIR     ST     SE    CEN
BOOL pressedKeys[11] = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE };		// shows pressed keys

BOOL isRunning;												// shows whether device connected
BYTE* inReport;												// buffer for the input report
int	inReportSize;											// size of buffer for the input report

OVERLAPPED devReadOverlapped;								// structure for async input
BOOL devReadPending;										// flag for expecting data

int devCount;												// number of found hid-devices
int devSelected;											// chosen device
PSP_DEVICE_INTERFACE_DETAIL_DATA devDetailData[DEV_NUM];	// paths to device interfaces
int devDetailDataSize[DEV_NUM];								// length of paths
int devInputReportSize[DEV_NUM];							// sizes of input reports
HANDLE hDevice;												// handle of opened device
HANDLE hSelected;											// handle of selected device


// HID Initialization. Reset the selected device and paths to device interfaces
void HID_Init()
{
	int  i;

	devCount = 0;																
	devSelected = -1;															
	for (i = 0; i < DEV_NUM; i++)												
	{
		devDetailData[i] = NULL;											
	}
}

// HID UnInitialization. Free memory with device paths
void HID_UnInit()
{
	int  i;

	for (i = 0; i < DEV_NUM; i++)												
	{
		if (devDetailData[i])
		{
			free(devDetailData[i]);												
		}
	}
}

// Open hid-device (num is index in the list of found devices)
BOOL Hid_Open(int num)
{
	ULONG numBuffers;

	if (devDetailData[num] == NULL) { return FALSE; }

	hDevice = CreateFile(devDetailData[num]->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hDevice == INVALID_HANDLE_VALUE) { return FALSE; }
	devSelected = num;
	memset(&devReadOverlapped, 0, sizeof(OVERLAPPED));

	return TRUE;
}

// Close hid-device. Reset selected device and cancel all async operations for device
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

// Reading data from hid-device
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
	GUID hidGuid;											// id of class hid-devices
	HANDLE hDevice;											// handle for opened device
	HDEVINFO devInfo;										// list of received hid-devices
	SP_DEVICE_INTERFACE_DATA devData;						// data about hid-device interface
	PSP_DEVICE_INTERFACE_DETAIL_DATA devDetail;				// path to device
	PSP_DEVICE_INTERFACE_DETAIL_DATA devDetailSelected;		// path to selected device
	PHIDP_PREPARSED_DATA preparsedData;						// data for processing reports
	HIDD_ATTRIBUTES attributes;								// device attributes
	HIDP_CAPS capabilities;									// capabilities from report descriptor
	ULONG length;											// size of buffer for receiving detail data about device interface
	int ind;												// index of interface
	int size;												// path length
	BOOL ok;												// flag

	// Receiving GUID of class USB HID devices
	HidD_GetHidGuid(&hidGuid);

	// Receiving list of all registered in system hid-devices
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

	// Reset device paths
	for (ind = 0; ind < DEV_NUM; ind++)
	{
		if (devDetailData[ind])
		{
			free(devDetailData[ind]);
			devDetailData[ind] = NULL;
		}
	}

	ind = -1;
	devCount = 0;
	hDevice = INVALID_HANDLE_VALUE;

	do
	{
		ind++;
		if (hDevice != INVALID_HANDLE_VALUE) { CloseHandle(hDevice); }

		// Get information about hid-device
		ok = SetupDiEnumDeviceInterfaces(devInfo, 0, &hidGuid, ind, &devData);
		if (!ok) { break; }

		// Get the length of buffer for device path and allocate memory for this buffer
		ok = SetupDiGetDeviceInterfaceDetail(devInfo, &devData, NULL, 0, &length, NULL);
		if (devDetail)
		{
			free(devDetail);
		}
		devDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(length);
		size = length;
		devDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// Get path to device
		ok = SetupDiGetDeviceInterfaceDetail(devInfo, &devData, devDetail, length, NULL, NULL);
		if (!ok) { continue; } 

		// Check the availability of the device
		hDevice = CreateFile(devDetail->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, 0, NULL);
		if (hDevice == INVALID_HANDLE_VALUE) { continue; }

		// Get device attributes
		attributes.Size = sizeof(attributes);
		ok = HidD_GetAttributes(hDevice, &attributes);
		if (!ok) { continue; }

		// Check VID and PID
		if (attributes.VendorID == VID && attributes.ProductID == PID)
		{
			hSelected = hDevice;
			devSelected = devCount;
		}

		// Get information for processing reports
		ok = HidD_GetPreparsedData(hDevice, &preparsedData);					
		if (!ok) { continue; }

		ok = HidP_GetCaps(preparsedData, &capabilities);   						
		if (!ok) { continue; }

		HidD_FreePreparsedData(preparsedData);

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

// Connect to hid-device
void Connect()
{
	if (Hid_Open(devSelected))
	{
		inReportSize = devInputReportSize[devSelected];
		if (inReport)
		{ 
			free(inReport); 
		}
		inReport = (BYTE*)calloc(inReportSize, 1);
		isRunning = true;
	}
	else
	{
		MessageBox(HWND_DESKTOP, L"Cannot open device", L"Error", MB_OK);
	}
}

void Input()
{
	BYTE byte;
	for (int i = 0; i < inReportSize; i++)
	{
		byte = inReport[i];
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

	return;
}

void Error()
{
	isRunning = false;
	Hid_Close();
}

void GetData()
{
	DWORD bytesRead;
	if (isRunning)
	{
		if (Hid_Read(inReport, inReportSize, &bytesRead))
		{
			if (bytesRead)
			{
				Input();
			}
		}
		else
		{
			Error();
		}
	}
}

void StartReceiveData()
{
	HID_Init();
	FindDevice();
	Connect();

	while (1)
	{
		GetData();
	}
}