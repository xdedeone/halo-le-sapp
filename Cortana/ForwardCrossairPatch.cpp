#include "ForwardCrossairPatch.h"

#include "Addresses.h"
#include "Patch.h"

short CrossairSig[] = { 0x6A, 0x04, 0x8D, 0x54, 0x24, 0x38, 0x89, 0x35, -1, -1, -1, -1, 0x8D, 0x34, 0x08, 0x52, 0x83, 0xC0, 0x30, 0x68 }; // 6A048D5424388935????????8D34085283C03068
short CrossairEAXSig[5] = { 0xA1, 0x00, 0x00, 0x00, 0x00 };
short CrossairECXSig[6] = { 0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00 };
short CrossairEDXSig[6] = { 0x8B, 0x15, 0x00, 0x00, 0x00, 0x00 };
short CrossairEBXSig[6] = { 0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00 };
short CrossairEBPSig[6] = { 0x8B, 0x2D, 0x00, 0x00, 0x00, 0x00 };
short CrossairESISig[6] = { 0x8B, 0x35, 0x00, 0x00, 0x00, 0x00 };
short CrossairEDISig[6] = { 0x8B, 0x3D, 0x00, 0x00, 0x00, 0x00 };

DWORD Crossair = NULL;

void __stdcall ForwardCrossair(DWORD pCrossair)
{
	ENCRYPT_BEGIN
	Crossair = DWORD(new byte[124]) ^ MagicUserKey;
	ENCRYPT_END
}

void __declspec(naked) ForwardCrossairCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		pushad
		pushfd
		push esi
		call ForwardCrossair
		popfd
		popad
		ENCRYPT_END
		retn
	}
}

void __declspec(naked) ForwardCrossairEAXCC()
{
	__asm
	{
		pushfd
		mov eax, [MagicUserKey]
		xor eax, [Crossair]
		popfd
		retn
	}
}

void __declspec(naked) ForwardCrossairECXCC()
{
	__asm
	{
		pushfd
		mov ecx, [MagicUserKey]
		xor ecx, [Crossair]
		popfd
		retn
	}
}

void __declspec(naked) ForwardCrossairEDXCC()
{
	__asm
	{
		pushfd
		mov edx, [MagicUserKey]
		xor edx, [Crossair]
		popfd
		retn
	}
}

void __declspec(naked) ForwardCrossairEBXCC()
{
	__asm
	{
		pushfd
		mov ebx, [MagicUserKey]
		xor ebx, [Crossair]
		popfd
		retn
	}
}

void __declspec(naked) ForwardCrossairEBPCC()
{
	__asm
	{
		pushfd
		mov ebp, [MagicUserKey]
		xor ebp, [Crossair]
		popfd
		retn
	}
}

void __declspec(naked) ForwardCrossairESICC()
{
	__asm
	{
		pushfd
		mov esi, [MagicUserKey]
		xor esi, [Crossair]
		popfd
		retn
	}
}

void __declspec(naked) ForwardCrossairEDICC()
{
	__asm
	{
		pushfd
		mov edi, [MagicUserKey]
		xor edi, [Crossair]
		popfd
		retn
	}
}

void __declspec(noinline) InstallForwardCrossairPatch()
{
	ENCRYPT_BEGIN
	DWORD Addr = FindCodeFromHandle(hhalo, CrossairSig, sizeof(CrossairSig)) + 8;
	BYTE CrossairPtr[4];
	*((DWORD*)(&CrossairPtr)) = *(DWORD*)Addr;

	CrossairEAXSig[1] = CrossairPtr[0];
	CrossairECXSig[2] = CrossairPtr[0];
	CrossairEDXSig[2] = CrossairPtr[0];
	CrossairEBXSig[2] = CrossairPtr[0];
	CrossairEBPSig[2] = CrossairPtr[0];
	CrossairESISig[2] = CrossairPtr[0];
	CrossairEDISig[2] = CrossairPtr[0];
	CrossairEAXSig[2] = CrossairPtr[1];
	CrossairECXSig[3] = CrossairPtr[1];
	CrossairEDXSig[3] = CrossairPtr[1];
	CrossairEBXSig[3] = CrossairPtr[1];
	CrossairEBPSig[3] = CrossairPtr[1];
	CrossairESISig[3] = CrossairPtr[1];
	CrossairEDISig[3] = CrossairPtr[1];
	CrossairEAXSig[3] = CrossairPtr[2];
	CrossairECXSig[4] = CrossairPtr[2];
	CrossairEDXSig[4] = CrossairPtr[2];
	CrossairEBXSig[4] = CrossairPtr[2];
	CrossairEBPSig[4] = CrossairPtr[2];
	CrossairESISig[4] = CrossairPtr[2];
	CrossairEDISig[4] = CrossairPtr[2];
	CrossairEAXSig[4] = CrossairPtr[3];
	CrossairECXSig[5] = CrossairPtr[3];
	CrossairEDXSig[5] = CrossairPtr[3];
	CrossairEBXSig[5] = CrossairPtr[3];
	CrossairEBPSig[5] = CrossairPtr[3];
	CrossairESISig[5] = CrossairPtr[3];
	CrossairEDISig[5] = CrossairPtr[3];

	Patches.NewPatch(new CodeCaveHook(hhalo, CrossairSig, sizeof(CrossairSig), 6, 6, NULL, ForwardCrossairCC, true));
	while (DWORD Address = FindCodeFromHandle(hhalo, CrossairEAXSig, sizeof(CrossairEAXSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, CrossairEAXSig, sizeof(CrossairEAXSig), 5, 0, NULL, ForwardCrossairEAXCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, CrossairECXSig, sizeof(CrossairECXSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, CrossairECXSig, sizeof(CrossairECXSig), 6, 0, NULL, ForwardCrossairECXCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, CrossairEDXSig, sizeof(CrossairEDXSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, CrossairEDXSig, sizeof(CrossairEDXSig), 6, 0, NULL, ForwardCrossairEDXCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, CrossairEBXSig, sizeof(CrossairEBXSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, CrossairEBXSig, sizeof(CrossairEBXSig), 6, 0, NULL, ForwardCrossairEBXCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, CrossairEBPSig, sizeof(CrossairEBPSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, CrossairEBPSig, sizeof(CrossairEBPSig), 6, 0, NULL, ForwardCrossairEBPCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, CrossairESISig, sizeof(CrossairESISig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, CrossairESISig, sizeof(CrossairESISig), 6, 0, NULL, ForwardCrossairESICC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, CrossairEDISig, sizeof(CrossairEDISig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, CrossairEDISig, sizeof(CrossairEDISig), 6, 0, NULL, ForwardCrossairEDICC, true, Address));

	ENCRYPT_END
}