#include "KeystonePatch.h"

#include "Addresses.h"
#include "Patch.h"
#include "StringCrypter.h"

using namespace std;

short KeystoneLoadSig[] = { 0x68, -1, -1, -1, -1, 0xFF, 0x15, -1, -1, -1, -1, 0x85, 0xC0, 0xA3, -1, -1, -1, -1, 0x0F, 0x84 }; // 68????????FF15????????85C0A3????????0F84
short KeystonePatchSig[] = { 0x8B, 0x44, 0x24, 0x10, 0x8B, 0x4C, 0x24, 0x14, 0x50, 0x51, 0xFF, 0x15, -1, -1, -1, -1, 0x85, 0xC0 }; //8B4424108B4C24145051FF15????????85C0
short KeystoneCrashSig[] = { 0x6A, 0x17, 0x57, 0x68, -1, -1, -1, -1, 0xB9, -1, -1, -1, -1, 0xE8, -1, -1, -1, -1, 0x85, 0xC0 }; //6A175768????????B9????????E8????????85C0

unsigned char KeystonePatch[8] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x33, 0xC0 };

unsigned char d3d9_str[] = { 0x9b, 0xcc, 0x9b, 0xc6, 0xd1, 0x9b, 0x93, 0x93, 0xff }; // "d3d9.dll"

short d3d3tablesig[] = { 0xC7, 0x06, -1, -1, -1, -1, 0x89, 0x86, -1, -1, -1, -1, 0x89, 0x86 }; // C706????????8986????????8986

HMODULE __stdcall PatchKeystone(char* name)
{
	ENCRYPT_BEGIN
	HMODULE hKeystone =	LoadLibrary(name);
	Patches.NewPatch(new CodePatch(hKeystone, KeystonePatchSig, sizeof(KeystonePatchSig), 8, 8, KeystonePatch));
	Patches.NewPatch(new MemoryAddress(hKeystone, KeystoneCrashSig, sizeof(KeystoneCrashSig), 9, (DWORD*)&NullMe));
	DWORD addr = FindCodeFromHandle(GetModuleHandle(CryptExtractor(StringCrypt(d3d9_str, sizeof(d3d9_str)))), d3d3tablesig, sizeof(d3d3tablesig));
	VTable = (DWORD*)*(DWORD*)(addr + 2);
	_beginthread(ScanThread, 0, 0);
	_beginthread(EventThread, 0, 0);
	ENCRYPT_END
	return hKeystone;
}

void __declspec(naked) __stdcall KeystoneLoadCC(char* name)
{
	__asm
	{
		ENCRYPT_BEGIN
		push ecx
		push edx
		push ebx
		mov eax, dword ptr ss : [esp + 0x10]
		push eax
		call PatchKeystone
		pop ebx
		pop edx
		pop ecx
		ENCRYPT_END
		retn 4
	}
}

void __declspec(noinline) InstallKeystonePatchCode()
{
	ENCRYPT_BEGIN
	Patches.NewPatch(new CodeCaveHook(hhalo, KeystoneLoadSig, sizeof(KeystoneLoadSig), 6, 5, NULL, KeystoneLoadCC, true));
	ENCRYPT_END
}