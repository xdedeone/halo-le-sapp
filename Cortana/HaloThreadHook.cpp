#include "HaloThreadHook.h"

#include "Addresses.h"
#include "Patch.h"

short HaloThreadHookSig[] = { 0xA1, -1, -1, -1, -1, 0x8B, 0x50, 0x14, 0x8B, 0x48, 0x0C, 0x83, 0xC4, 0x04, 0x42, 0x41, 0x4E, 0x4F, 0x89, 0x50, 0x14, 0x89, 0x48, 0x0C }; // "A1????????8B50148B480C83C40442414E4F89501489480C"

DWORD* GameTicksAddress;
DWORD HaloThreadHook()
{
	if (TimeLeft > 1)
		TimeLeft--;
	return *GameTicksAddress;
}

void __declspec(noinline) InstallHaloThreadHookCode()
{
	ENCRYPT_BEGIN
	Patches.NewPatch(new MemoryAddress(hhalo, HaloThreadHookSig, sizeof(HaloThreadHookSig), 1, (DWORD*)&GameTicksAddress));
	Patches.NewPatch(new CodeCaveHook(hhalo, HaloThreadHookSig, sizeof(HaloThreadHookSig), 5, 0, NULL, HaloThreadHook, true));
	ENCRYPT_END
}