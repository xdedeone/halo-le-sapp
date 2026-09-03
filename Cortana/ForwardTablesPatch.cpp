#include "ForwardTablesPatch.h"

#include "Addresses.h"
#include "Patch.h"

short ForwardTableAllocSig[] = { 0x56, 0x83, 0xC0, 0x38, 0x57, 0x8D, 0x34, 0x11, 0x03, 0xC8 }; // 5683C038578D341103C8

LPVOID __stdcall ForwardTables(DWORD size)
{
	ENCRYPT_BEGIN
	LPVOID table = new byte[size];
	ENCRYPT_END
	return table;
}

void __declspec(naked) ForwardTablesCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		add ecx, eax
		push eax
		push ecx
		push edx
		push ebx
		push eax
		call ForwardTables
		mov esi, eax
		pop ebx
		pop edx
		pop ecx
		pop eax	
		ENCRYPT_END
		retn
	}
}

void __declspec(noinline) InstallForwardTablesPatch()
{
	ENCRYPT_BEGIN
	Patches.NewPatch(new CodeCaveHook(hhalo, ForwardTableAllocSig, sizeof(ForwardTableAllocSig), 5, 5, NULL, ForwardTablesCC, true));
	ENCRYPT_END
}