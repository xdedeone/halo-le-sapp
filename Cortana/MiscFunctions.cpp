#include "MiscFunctions.h"

#include <Windows.h>
#include <iphlpapi.h>
#include <Psapi.h>
#include <tlhelp32.h>

#include "Addresses.h"
#include "NetworkClient.h"
#include "StringCrypter.h"

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Psapi.lib")

unsigned char strings_str[] = { 0x8c, 0x8b, 0x8d, 0x96, 0x91, 0x98, 0x8c, 0xd1, 0x9b, 0x93, 0x93, 0xff }; // strings.dll

void HashHalo(unsigned char* hash)
{
	ENCRYPT_BEGIN
	PIMAGE_NT_HEADERS NtHeader = (PIMAGE_NT_HEADERS)((DWORD)hhalo + ((PIMAGE_DOS_HEADER)hhalo)->e_lfanew);
	PIMAGE_SECTION_HEADER Section = (PIMAGE_SECTION_HEADER)((DWORD)NtHeader + sizeof(IMAGE_NT_HEADERS));
	for (WORD i = 0; i < NtHeader->FileHeader.NumberOfSections; i++, Section++)
	{
		if (Section->Characteristics & IMAGE_SCN_MEM_EXECUTE)
		{
			SHA256_Data((unsigned char*)((DWORD)hhalo + Section->VirtualAddress), Section->SizeOfRawData, hash);
			break;
		}
	}
	ENCRYPT_END
}

void ListModules()
{
	ENCRYPT_BEGIN
	DWORD entries;
	HMODULE modules[256];
	EnumProcessModules(cProc, modules, sizeof(modules), &entries);
	entries /= sizeof(HMODULE);
	
	for (size_t i = 0; i < entries; i++)
	{
		char buff[288];
		if (GetModuleFileName(modules[i], buff, 288))
		{
			myClient->SendBigEvent(event_module, buff, 288);
		}
	}
	ENCRYPT_END
}

void ListProcess()
{
	ENCRYPT_BEGIN
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hProcessSnap, &pe32))
	{
		do
		{
			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, 0, pe32.th32ProcessID);
			if (hProcess != NULL)
			{
				char buff[288];
				if (GetProcessImageFileName(hProcess, buff, 288))
				{
					myClient->SendBigEvent(event_process, buff, 288);
				}
				CloseHandle(hProcess);
			}
			
		} while (Process32Next(hProcessSnap, &pe32));
	}	
	CloseHandle(hProcessSnap);
	ENCRYPT_END
}

void GetHashesAndMACs()
{
	ENCRYPT_BEGIN
	char hash[32];
	char name[288];

	GetModuleFileName(0, name, 288);
	SHA256_File(name, (unsigned char*)hash);
	myClient->SendBigEvent(event_halo_checksum, hash, 32);

	SHA256_File(CryptExtractor(StringCrypt(strings_str, sizeof(strings_str))), (unsigned char*)hash);
	myClient->SendBigEvent(event_strings_checksum, hash, 32);

	ULONG ulOutBufLen = 0;
	GetAdaptersInfo(NULL, &ulOutBufLen);
	PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO *)new byte[ulOutBufLen];

	DWORD dwRetVal = 0;
	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{
		size_t count = 0;
		PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
		while (pAdapter)
		{
			count++;
			pAdapter = pAdapter->Next;
		}

		if (count % 4 != 0)
		{
			count += 4 - (count % 4);
		}
		ULONGLONG* buff = new ULONGLONG[count];
		memset(buff, 0, count * sizeof(ULONGLONG));

		pAdapter = pAdapterInfo;
		for (size_t i = 0; pAdapter; pAdapter = pAdapter->Next, i++)
		{
			memcpy(&buff[i], &pAdapter->Address, sizeof(ULONGLONG));
		}

		myClient->SendBigEvent(event_mac_addresses, (char*)buff, count * sizeof(ULONGLONG));

		delete[] buff;
	}

	delete[](byte*)pAdapterInfo;
	ENCRYPT_END
}

std::wstring FormatWArgs(const wchar_t* fmt, ...)
{
	va_list alist;
	va_start(alist, fmt);

	wchar_t buff[4096];

	_vsnwprintf_s(buff, 4096, 4095, fmt, alist);

	va_end(alist);

	return std::wstring(buff);
}

void PrintConsole(const char* pString, const float* fColour)
{
	__asm
	{
		mov eax, fColour
		push pString
		call PrintConsoleFunc
		add esp, 4
	}
}