#include <errno.h>
#include <stdio.h>
#include <Windows.h>

#include "codefinder.h"

const char* sapp_dat = "sapp.dat";
const char* sapp_dll = "sapp.dll";

DWORD HaloExecuteAddress;
void ExecuteCommand(const char* command)
{
	__asm
	{
		xor eax, eax
		mov edi, command
		call HaloExecuteAddress
	}
}

bool UninstallSappUpdateHandler();

void __stdcall AutoUpdate()
{
	UninstallSappUpdateHandler();

	ExecuteCommand("unload");

	size_t count = 120;
	while (remove(sapp_dll) && errno != ENOENT && --count)
	{
		Sleep(10000);
	}

	if (count)
		rename(sapp_dat, sapp_dll);

	ExecuteCommand("load");	
}

DWORD BackAddr;
void __declspec(naked)AutoUpdateHandlerCodecave()
{
	__asm {
		pushad
		pushfd
		call AutoUpdate
		popfd
		popad
		lea edx, [esp+0x78]
		push edx
		jmp BackAddr
	}
}

BYTE signature1[12] = { 0x8A, 0x07, 0x81, 0xEC, 0x00, 0x05, 0x00, 0x00, 0x3C, 0x3B, 0x74, 0x0E };
BYTE signature2[7] = {0x8D, 0x54, 0x24, 0x78, 0x52, 0xFF, 0xD5};

DWORD HaloThreadBeginCodeAddress;
char OrginalHaloThreadCode[5];
bool IsUpdateHandlerInstalled = false;
bool InstallSappUpdateHandler()
{
	HMODULE hModule = GetModuleHandle(0);
	HaloExecuteAddress = XXFindCode(hModule, (char*)signature1, 12);
	HaloThreadBeginCodeAddress = XXFindCode(hModule, (char*)signature2, 7);
	if(HaloThreadBeginCodeAddress == NULL)
	{
		return false;
	}

	char *thread_code = (char*)HaloThreadBeginCodeAddress;
	memcpy(OrginalHaloThreadCode, thread_code, 5);
	BackAddr = HaloThreadBeginCodeAddress + 5;

	// insert codecave jump
	DWORD OldProtect;
	VirtualProtect((LPVOID)HaloThreadBeginCodeAddress, 5, PAGE_EXECUTE_READWRITE, &OldProtect);
	thread_code[0] = '\xE9'; // opcode of jmp
	long CallOffset = (long)AutoUpdateHandlerCodecave-(HaloThreadBeginCodeAddress+5);
	*(long*)(&thread_code[1]) = CallOffset;

	VirtualProtect((LPVOID)HaloThreadBeginCodeAddress, 5, OldProtect, &OldProtect);
	IsUpdateHandlerInstalled = true;
	return true;
}

bool UninstallSappUpdateHandler()
{
	if(!IsUpdateHandlerInstalled)
		return false;

	char *thread_code = (char*)HaloThreadBeginCodeAddress;

	// delete codecave jump
	DWORD OldProtect;
	VirtualProtect((LPVOID)HaloThreadBeginCodeAddress, 5, PAGE_EXECUTE_READWRITE, &OldProtect);
	memcpy(thread_code, OrginalHaloThreadCode, 5);
	VirtualProtect((LPVOID)HaloThreadBeginCodeAddress, 5, OldProtect, &OldProtect);
	IsUpdateHandlerInstalled = false;
	return true;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fwdreason, LPVOID lpvReserved)
{
	switch (fwdreason)
    {
	case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hinstDLL);
			InstallSappUpdateHandler();		
			break;
		}
	case DLL_PROCESS_DETACH:
		{
			UninstallSappUpdateHandler();
			break;
		}
	}

	return 1;
}