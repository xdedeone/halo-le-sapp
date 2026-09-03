#include "ForwardCameraStructPatch.h"

#include "Addresses.h"
#include "Patch.h"

short CameraStructSig[] = { 0x83, 0xC4, 0x28, 0x89, 0x35, -1, -1, -1, -1, 0x5E, 0x5B, 0x59, 0xC3 }; // 83C4288935????????5E5B59C3
short CameraEAXSig[5] = { 0xA1, 0x00, 0x00, 0x00, 0x00 };
short CameraECXSig[6] = { 0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00 };
short CameraEDXSig[6] = { 0x8B, 0x15, 0x00, 0x00, 0x00, 0x00 };
short CameraESISig[6] = { 0x8B, 0x35, 0x00, 0x00, 0x00, 0x00 };
short CameraEDISig[6] = { 0x8B, 0x3D, 0x00, 0x00, 0x00, 0x00 };

DWORD CameraStruct = NULL;

void ForwardCameraStruct()
{
	ENCRYPT_BEGIN
	if (CameraStruct != NULL)
	{
		delete[] (byte*)(CameraStruct ^ MagicUserKey);
	}	
	CameraStruct = DWORD(new byte[80]) ^ MagicUserKey;
	ENCRYPT_END
}

void __declspec(naked) ForwardCameraStructCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		pushad
		pushfd
		call ForwardCameraStruct
		popfd
		popad
		ENCRYPT_END
		retn
	}
}

void __declspec(naked) ForwardCameraStructEAXCC()
{
	__asm
	{
		pushfd
		mov eax, [MagicUserKey]
		xor eax, [CameraStruct]
		popfd
		retn
	}
}

void __declspec(naked) ForwardCameraStructECXCC()
{
	__asm
	{
		pushfd
		mov ecx, [MagicUserKey]
		xor ecx, [CameraStruct]
		popfd
		retn
	}
}

void __declspec(naked) ForwardCameraStructEDXCC()
{
	__asm
	{
		pushfd
		mov edx, [MagicUserKey]
		xor edx, [CameraStruct]
		popfd
		retn
	}
}

void __declspec(naked) ForwardCameraStructESICC()
{
	__asm
	{
		pushfd
		mov esi, [MagicUserKey]
		xor esi, [CameraStruct]
		popfd
		retn
	}
}

void __declspec(naked) ForwardCameraStructEDICC()
{
	__asm
	{
		pushfd
		mov edi, [MagicUserKey]
		xor edi, [CameraStruct]
		popfd
		retn
	}
}

void __declspec(noinline) InstallForwardCameraStructPatch()
{
	ENCRYPT_BEGIN
	DWORD Addr = FindCodeFromHandle(hhalo, CameraStructSig, sizeof(CameraStructSig)) + 5;
	BYTE CameraStructPtr[4];
	*((DWORD*)(&CameraStructPtr)) = *(DWORD*)Addr;

	CameraEAXSig[1] = CameraStructPtr[0];
	CameraECXSig[2] = CameraStructPtr[0];
	CameraEDXSig[2] = CameraStructPtr[0];
	CameraESISig[2] = CameraStructPtr[0];
	CameraEDISig[2] = CameraStructPtr[0];
	CameraEAXSig[2] = CameraStructPtr[1];
	CameraECXSig[3] = CameraStructPtr[1];
	CameraEDXSig[3] = CameraStructPtr[1];
	CameraESISig[3] = CameraStructPtr[1];
	CameraEDISig[3] = CameraStructPtr[1];
	CameraEAXSig[3] = CameraStructPtr[2];
	CameraECXSig[4] = CameraStructPtr[2];
	CameraEDXSig[4] = CameraStructPtr[2];
	CameraESISig[4] = CameraStructPtr[2];
	CameraEDISig[4] = CameraStructPtr[2];
	CameraEAXSig[4] = CameraStructPtr[3];
	CameraECXSig[5] = CameraStructPtr[3];
	CameraEDXSig[5] = CameraStructPtr[3];
	CameraESISig[5] = CameraStructPtr[3];
	CameraEDISig[5] = CameraStructPtr[3];

	Patches.NewPatch(new CodeCaveHook(hhalo, CameraStructSig, sizeof(CameraStructSig), 6, 3, NULL, ForwardCameraStructCC, true));
	while (DWORD Address = FindCodeFromHandle(hhalo, CameraEAXSig, sizeof(CameraEAXSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, CameraEAXSig, sizeof(CameraEAXSig), 5, 0, NULL, ForwardCameraStructEAXCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, CameraECXSig, sizeof(CameraECXSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, CameraECXSig, sizeof(CameraECXSig), 6, 0, NULL, ForwardCameraStructECXCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, CameraEDXSig, sizeof(CameraEDXSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, CameraEDXSig, sizeof(CameraEDXSig), 6, 0, NULL, ForwardCameraStructEDXCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, CameraESISig, sizeof(CameraESISig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, CameraESISig, sizeof(CameraESISig), 6, 0, NULL, ForwardCameraStructESICC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, CameraEDISig, sizeof(CameraEDISig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, CameraEDISig, sizeof(CameraEDISig), 6, 0, NULL, ForwardCameraStructEDICC, true, Address));

	ENCRYPT_END
}