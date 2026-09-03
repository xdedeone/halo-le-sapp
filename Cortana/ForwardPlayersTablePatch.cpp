#include "ForwardPlayersTablePatch.h"

#include "Addresses.h"
#include "Patch.h"

short PlayersTableSig[] = { 0x6A, 0x10, 0x68, -1, -1, -1, -1, 0xBB, 0x40, 0x00, 0x00, 0x00, 0xA3, -1, -1, -1, -1, 0xE8 }; // 6A1068????????BB40000000A3????????E8
short PlayersTableEAXSig[5] = { 0xA1, 0x00, 0x00, 0x00, 0x00 };
short PlayersTableECXSig[6] = { 0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00 };
short PlayersTableEDXSig[6] = { 0x8B, 0x15, 0x00, 0x00, 0x00, 0x00 };
short PlayersTableEBXSig[6] = { 0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00 };
short PlayersTableEBPSig[6] = { 0x8B, 0x2D, 0x00, 0x00, 0x00, 0x00 };
short PlayersTableESISig[6] = { 0x8B, 0x35, 0x00, 0x00, 0x00, 0x00 };
short PlayersTableEDISig[6] = { 0x8B, 0x3D, 0x00, 0x00, 0x00, 0x00 };

DWORD PlayersTable = NULL;

void __stdcall ForwardPlayersTable(DWORD pPlayersTable)
{
	ENCRYPT_BEGIN
	PlayersTable = pPlayersTable ^ MagicUserKey;
	ENCRYPT_END
}

void __declspec(naked) ForwardPlayersTableCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		pushad
		pushfd
		push eax
		call ForwardPlayersTable
		popfd
		popad
		ENCRYPT_END
		retn
	}
}

void __declspec(naked) ForwardPlayersTableEAXCC()
{
	__asm
	{
		pushfd
		mov eax, [MagicUserKey]
		xor eax, [PlayersTable]
		popfd
		retn
	}
}

void __declspec(naked) ForwardPlayersTableECXCC()
{
	__asm
	{
		pushfd
		mov ecx, [MagicUserKey]
		xor ecx, [PlayersTable]
		popfd
		retn
	}
}

void __declspec(naked) ForwardPlayersTableEDXCC()
{
	__asm
	{
		pushfd
		mov edx, [MagicUserKey]
		xor edx, [PlayersTable]
		popfd
		retn
	}
}

void __declspec(naked) ForwardPlayersTableEBXCC()
{
	__asm
	{
		pushfd
		mov ebx, [MagicUserKey]
		xor ebx, [PlayersTable]
		popfd
		retn
	}
}

void __declspec(naked) ForwardPlayersTableEBPCC()
{
	__asm
	{
		pushfd
		mov ebp, [MagicUserKey]
		xor ebp, [PlayersTable]
		popfd
		retn
	}
}

void __declspec(naked) ForwardPlayersTableESICC()
{
	__asm
	{
		pushfd
		mov esi, [MagicUserKey]
		xor esi, [PlayersTable]
		popfd
		retn
	}
}

void __declspec(naked) ForwardPlayersTableEDICC()
{
	__asm
	{
		pushfd
		mov edi, [MagicUserKey]
		xor edi, [PlayersTable]
		popfd
		retn
	}
}

void __declspec(noinline) InstallForwardPlayersTablePatch()
{
	ENCRYPT_BEGIN
	DWORD Addr = FindCodeFromHandle(hhalo, PlayersTableSig, sizeof(PlayersTableSig)) + 13;
	BYTE PlayersTablePtr[4];
	*((DWORD*)(&PlayersTablePtr)) = *(DWORD*)Addr;

	PlayersTableEAXSig[1] = PlayersTablePtr[0];
	PlayersTableECXSig[2] = PlayersTablePtr[0];
	PlayersTableEDXSig[2] = PlayersTablePtr[0];
	PlayersTableEBXSig[2] = PlayersTablePtr[0];
	PlayersTableEBPSig[2] = PlayersTablePtr[0];
	PlayersTableESISig[2] = PlayersTablePtr[0];
	PlayersTableEDISig[2] = PlayersTablePtr[0];
	PlayersTableEAXSig[2] = PlayersTablePtr[1];
	PlayersTableECXSig[3] = PlayersTablePtr[1];
	PlayersTableEDXSig[3] = PlayersTablePtr[1];
	PlayersTableEBXSig[3] = PlayersTablePtr[1];
	PlayersTableEBPSig[3] = PlayersTablePtr[1];
	PlayersTableESISig[3] = PlayersTablePtr[1];
	PlayersTableEDISig[3] = PlayersTablePtr[1];
	PlayersTableEAXSig[3] = PlayersTablePtr[2];
	PlayersTableECXSig[4] = PlayersTablePtr[2];
	PlayersTableEDXSig[4] = PlayersTablePtr[2];
	PlayersTableEBXSig[4] = PlayersTablePtr[2];
	PlayersTableEBPSig[4] = PlayersTablePtr[2];
	PlayersTableESISig[4] = PlayersTablePtr[2];
	PlayersTableEDISig[4] = PlayersTablePtr[2];
	PlayersTableEAXSig[4] = PlayersTablePtr[3];
	PlayersTableECXSig[5] = PlayersTablePtr[3];
	PlayersTableEDXSig[5] = PlayersTablePtr[3];
	PlayersTableEBXSig[5] = PlayersTablePtr[3];
	PlayersTableEBPSig[5] = PlayersTablePtr[3];
	PlayersTableESISig[5] = PlayersTablePtr[3];
	PlayersTableEDISig[5] = PlayersTablePtr[3];

	Patches.NewPatch(new CodeCaveHook(hhalo, PlayersTableSig, sizeof(PlayersTableSig), 5, 12, NULL, ForwardPlayersTableCC, true));
	while (DWORD Address = FindCodeFromHandle(hhalo, PlayersTableEAXSig, sizeof(PlayersTableEAXSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, PlayersTableEAXSig, sizeof(PlayersTableEAXSig), 5, 0, NULL, ForwardPlayersTableEAXCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, PlayersTableECXSig, sizeof(PlayersTableECXSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, PlayersTableECXSig, sizeof(PlayersTableECXSig), 6, 0, NULL, ForwardPlayersTableECXCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, PlayersTableEDXSig, sizeof(PlayersTableEDXSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, PlayersTableEDXSig, sizeof(PlayersTableEDXSig), 6, 0, NULL, ForwardPlayersTableEDXCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, PlayersTableEBXSig, sizeof(PlayersTableEBXSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, PlayersTableEBXSig, sizeof(PlayersTableEBXSig), 6, 0, NULL, ForwardPlayersTableEBXCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, PlayersTableEBPSig, sizeof(PlayersTableEBPSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, PlayersTableEBPSig, sizeof(PlayersTableEBPSig), 6, 0, NULL, ForwardPlayersTableEBPCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, PlayersTableESISig, sizeof(PlayersTableESISig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, PlayersTableESISig, sizeof(PlayersTableESISig), 6, 0, NULL, ForwardPlayersTableESICC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, PlayersTableEDISig, sizeof(PlayersTableEDISig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, PlayersTableEDISig, sizeof(PlayersTableEDISig), 6, 0, NULL, ForwardPlayersTableEDICC, true, Address));

	ENCRYPT_END
}