#include "DirectXPatch.h"

#include "Addresses.h"
#include "Patch.h"

short CheatSig[] = { 0x55, 0x8B, 0xEC, 0x53, 0x8B, 0x5D, 0x08, 0x56, 0x53, 0xFF, 0x15, -1, -1, -1, -1, 0x8B, 0xF0, 0x85, 0xF6, 0x74, -1, 0x57, 0x6A, 0x08, 0xFF, 0x15, -1, -1, -1, -1, 0x83, 0xC4, 0x04, 0x85, 0xC0 }; // 558BEC538B5D085653FF15????????8BF085F674??576A08FF15????????83C40485C0
short NvidiaSig[] = { 0x55, 0x8B, 0xEC, 0xFF, 0x75, 0x08, 0xFF, 0x15, -1, -1, -1, -1, 0x85, 0xC0, 0x75, 0x04, 0x5D, 0xC2, 0x04, 0x00, 0x89, 0x45, 0x08, 0x5D, 0xE9 }; // 558BECFF7508FF15????????85C075045DC204008945085DE9

short DXCreateSig1[] = { 0x6A, 0x1F, 0xFF, 0x15, -1, -1, -1, -1, 0x3B, 0xC3 }; // 6A1FFF15????????3BC3
short DXCreateSig2[] = { 0xA1, -1, -1, -1, -1, 0x6A, 0x1F, 0xFF, 0xD0, 0x8B, 0xF0 }; // A1????????6A1FFFD08BF0

byte DXCreateOriginal[5] = { 0x8B, 0xFF, 0x55, 0x8B, 0xEC }; // mov edi, edi / push ebp / mov ebp, esp

DWORD* pDXCreate9 = NULL;

void  DXCreate9Hook()
{
	ENCRYPT_BEGIN
	if (*(unsigned char*)(*pDXCreate9) == 0xE9)
	{
		DWORD address = *pDXCreate9 + 1;
		DWORD HookedFunc = *(DWORD*)address + address + 4;

		bool nvidia = false;
		if (*(unsigned char*)HookedFunc == 0xE9)
		{
			DWORD HookedJmpFunc = *(DWORD*)(HookedFunc + 1) + HookedFunc + 1 + 4;

			if (FindCode(HookedJmpFunc, sizeof(NvidiaSig), NvidiaSig, sizeof(NvidiaSig)))
				nvidia = true;
		}

		if (!nvidia)
		{
			DWORD OldProtect; // Remove DirectXCreate9 hook
			VirtualProtect((LPVOID)*pDXCreate9, 5, PAGE_EXECUTE_READWRITE, &OldProtect);
			memcpy((LPVOID)*pDXCreate9, DXCreateOriginal, 5); // patch the code
			VirtualProtect((LPVOID)*pDXCreate9, 5, OldProtect, &OldProtect);

			if (FindCode(HookedFunc, sizeof(CheatSig), CheatSig, sizeof(CheatSig)))
				myClient->SendSmallEvent(event_illegal_dxcreate_hook);
		}
	}
	ENCRYPT_END
}

void __declspec(naked) DXCreate9CC()
{
	__asm
	{
		ENCRYPT_BEGIN
		pushad
		call DXCreate9Hook
		popad
		push 0x1F
		mov eax, pDXCreate9
		mov eax, [eax]
		call eax
		mov esi, eax
		ENCRYPT_END
		retn
	}
}

void __declspec(noinline) InstallDXPatchCode()
{
	ENCRYPT_BEGIN
	Patches.NewPatch(new MemoryAddress(hhalo, DXCreateSig1, sizeof(DXCreateSig1), 4, (DWORD*)&pDXCreate9));
	Patches.NewPatch(new CodeCaveHook(hhalo, DXCreateSig1, sizeof(DXCreateSig1), 8, 0, NULL, DXCreate9CC, true));
	Patches.NewPatch(new CodeCaveHook(hhalo, DXCreateSig2, sizeof(DXCreateSig2), 6, 5, NULL, DXCreate9CC, true));
	ENCRYPT_END
}