#include "Main.h"

#include "Addresses.h"
#include "MiscFunctions.h"
#include "NetworkClient.h"
#include "Patch.h"

DWORD Version = 0x02000000;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fwdreason, LPVOID lpvReserved)
{
	BOOL ret = 1;
	switch (fwdreason)
	{
	case DLL_PROCESS_ATTACH:
		{
			MagicUserKey = ((NetworkCoreStruct*)lpvReserved)->Key;
			ENCRYPT_BEGIN
		#ifndef DEBUG
			NtSIT((HANDLE)-2, 0x11, 0, 0);
		#endif
			myClient = new NetworkClient(((NetworkCoreStruct*)lpvReserved)->s);
			if (ret = myClient->Start())
			{
				InstallPatches();
			#ifndef DEBUG
				HashHalo(hHash);	
			#endif
			}
			ENCRYPT_END
			break;
		 }
	case DLL_PROCESS_DETACH:
		{
			break;
		}
	}

	return ret;
}

// Unused functions

//GetModuleFileName();
//HMODULE GetModuleFromAddress(DWORD addr)
//{
//	HMODULE modules[256];
//	DWORD entries;
//
//	EnumProcessModules(cProc, modules, sizeof(modules), &entries);
//	entries /= sizeof(HMODULE*);
//
//	for (unsigned int i = 0; i < entries; i++)
//	{
//		MODULEINFO info;
//		GetModuleInformation(cProc, modules[i], &info, sizeof(MODULEINFO));
//		if (addr >= (DWORD)info.lpBaseOfDll && addr <= ((DWORD)info.lpBaseOfDll + info.SizeOfImage))
//		{
//			return modules[i];
//		}
//	}
//
//	return NULL;
//}

//void ReadPhysicalDriveInNTWithZeroRights()
//{
//	for (int drive = 0; drive < 16; drive++)
//	{
//		HANDLE hPhysicalDriveIOCTL = NULL;
//
//		//  Try to get a handle to PhysicalDrive IOCTL, report failure
//		//  and exit if can't.
//		char driveName[32];
//		sprintf(driveName, "\\\\.\\PhysicalDrive%d", drive);
//
//		//  Windows NT, Windows 2000, Windows XP - admin rights not required
//		hPhysicalDriveIOCTL = CreateFile(driveName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
//
//		if (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE)
//		{
//			STORAGE_PROPERTY_QUERY query;
//			DWORD cbBytesReturned = 0;
//			char buffer[4096];
//			memset(buffer, 0, sizeof (buffer));
//
//			memset((void *)&query, 0, sizeof(query));
//			query.PropertyId = StorageDeviceProperty;
//			query.QueryType = PropertyStandardQuery;
//
//			if (DeviceIoControl(hPhysicalDriveIOCTL, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &buffer, sizeof(buffer), &cbBytesReturned, NULL))
//			{
//				PSTORAGE_DEVICE_DESCRIPTOR descrip = (PSTORAGE_DEVICE_DESCRIPTOR)&buffer;
//
//				printf("Drive%u Serial: %s\n", drive, buffer + (DWORD)descrip->SerialNumberOffset);
//			}
//			else
//			{
//				DWORD err = GetLastError();
//				printf("\nDeviceIOControl IOCTL_STORAGE_QUERY_PROPERTY error = %d\n", err);
//			}
//
//			CloseHandle(hPhysicalDriveIOCTL);
//		}
//	}
//}