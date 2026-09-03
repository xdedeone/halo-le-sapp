#include "ForwardTagsTablePatch.h"
#include "ForwardCameraStructPatch.h"

#include "Addresses.h"
#include "Patch.h"

short TagsTableSig[] = { 0x89, 0x3D, -1, -1, -1, -1, 0x8B, 0x07, 0xA3}; // 893D????????8B07A3
short TagsTableEAXSig[5] = { 0xA1, 0x00, 0x00, 0x00, 0x00 };
short TagsTableECXSig[6] = { 0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00 };
short TagsTableEDXSig[6] = { 0x8B, 0x15, 0x00, 0x00, 0x00, 0x00 };
short TagsTableEBXSig[6] = { 0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00 };
short TagsTableEBPSig[6] = { 0x8B, 0x2D, 0x00, 0x00, 0x00, 0x00 };
short TagsTableESISig[6] = { 0x8B, 0x35, 0x00, 0x00, 0x00, 0x00 };
short TagsTableEDISig[6] = { 0x8B, 0x3D, 0x00, 0x00, 0x00, 0x00 };

DWORD TagsTable = 0x40440028;

ULONGLONG TagsTableHash;
bool TagHashReady = false;

void __stdcall ForwardTagsTable(Tag_Table_Header* pTagsTableHeader, Tag_Entry* pTagsTable)
{
	ENCRYPT_BEGIN
	TagsTable ^= MagicUserKey;

	if (TagsTable != 0x40440028)
		delete[] (byte*)TagsTable;

	size_t count = pTagsTableHeader->TagCount;
	size_t size = count * sizeof(Tag_Entry);

	TagsTable = DWORD(new byte[size]);

	memcpy((void*)TagsTable, pTagsTable, size);

	//TagsTableHash = 0;
	//for (size_t i = 0; i < count; i++)
	//	TagsTableHash += (pTagsTable[i].TagID ^ pTagsTable[i].TagStruct) * (i + 1);

	//TagHashReady = true;

	TagsTable ^= MagicUserKey;
	ENCRYPT_END
}

void __declspec(naked) ForwardTagsTableCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		pushad
		pushfd
		push eax
		push edi
		call ForwardTagsTable
		call ForwardCameraStruct
		popfd
		popad
		ENCRYPT_END
		retn
	}
}

void __declspec(naked) ForwardTagsTableEAXCC()
{
	__asm
	{
		pushfd
		mov eax, [MagicUserKey]
		xor eax, [TagsTable]
		popfd
		retn
	}
}

void __declspec(naked) ForwardTagsTableECXCC()
{
	__asm
	{
		pushfd
		mov ecx, [MagicUserKey]
		xor ecx, [TagsTable]
		popfd
		retn
	}
}

void __declspec(naked) ForwardTagsTableEDXCC()
{
	__asm
	{
		pushfd
		mov edx, [MagicUserKey]
		xor edx, [TagsTable]
		popfd
		retn
	}
}

void __declspec(naked) ForwardTagsTableEBXCC()
{
	__asm
	{
		pushfd
		mov ebx, [MagicUserKey]
		xor ebx, [TagsTable]
		popfd
		retn
	}
}

void __declspec(naked) ForwardTagsTableEBPCC()
{
	__asm
	{
		pushfd
		mov ebp, [MagicUserKey]
		xor ebp, [TagsTable]
		popfd
		retn
	}
}

void __declspec(naked) ForwardTagsTableESICC()
{
	__asm
	{
		pushfd
		mov esi, [MagicUserKey]
		xor esi, [TagsTable]
		popfd
		retn
	}
}

void __declspec(naked) ForwardTagsTableEDICC()
{
	__asm
	{
		pushfd
		mov edi, [MagicUserKey]
		xor edi, [TagsTable]
		popfd
		retn
	}
}

void __declspec(noinline) InstallForwardTagsTablePatch()
{
	ENCRYPT_BEGIN
	TagsTable ^= MagicUserKey;

	DWORD Addr = FindCodeFromHandle(hhalo, TagsTableSig, sizeof(TagsTableSig)) + 9;
	BYTE TagsTablePtr[4];
	*((DWORD*)(&TagsTablePtr)) = *(DWORD*)Addr;

	TagsTableEAXSig[1] = TagsTablePtr[0];
	TagsTableECXSig[2] = TagsTablePtr[0];
	TagsTableEDXSig[2] = TagsTablePtr[0];
	TagsTableEBXSig[2] = TagsTablePtr[0];
	TagsTableEBPSig[2] = TagsTablePtr[0];
	TagsTableESISig[2] = TagsTablePtr[0];
	TagsTableEDISig[2] = TagsTablePtr[0];
	TagsTableEAXSig[2] = TagsTablePtr[1];
	TagsTableECXSig[3] = TagsTablePtr[1];
	TagsTableEDXSig[3] = TagsTablePtr[1];
	TagsTableEBXSig[3] = TagsTablePtr[1];
	TagsTableEBPSig[3] = TagsTablePtr[1];
	TagsTableESISig[3] = TagsTablePtr[1];
	TagsTableEDISig[3] = TagsTablePtr[1];
	TagsTableEAXSig[3] = TagsTablePtr[2];
	TagsTableECXSig[4] = TagsTablePtr[2];
	TagsTableEDXSig[4] = TagsTablePtr[2];
	TagsTableEBXSig[4] = TagsTablePtr[2];
	TagsTableEBPSig[4] = TagsTablePtr[2];
	TagsTableESISig[4] = TagsTablePtr[2];
	TagsTableEDISig[4] = TagsTablePtr[2];
	TagsTableEAXSig[4] = TagsTablePtr[3];
	TagsTableECXSig[5] = TagsTablePtr[3];
	TagsTableEDXSig[5] = TagsTablePtr[3];
	TagsTableEBXSig[5] = TagsTablePtr[3];
	TagsTableEBPSig[5] = TagsTablePtr[3];
	TagsTableESISig[5] = TagsTablePtr[3];
	TagsTableEDISig[5] = TagsTablePtr[3];

	Patches.NewPatch(new CodeCaveHook(hhalo, TagsTableSig, sizeof(TagsTableSig), 5, 8, NULL, ForwardTagsTableCC, true));
	while (DWORD Address = FindCodeFromHandle(hhalo, TagsTableEAXSig, sizeof(TagsTableEAXSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, TagsTableEAXSig, sizeof(TagsTableEAXSig), 5, 0, NULL, ForwardTagsTableEAXCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, TagsTableECXSig, sizeof(TagsTableECXSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, TagsTableECXSig, sizeof(TagsTableECXSig), 6, 0, NULL, ForwardTagsTableECXCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, TagsTableEDXSig, sizeof(TagsTableEDXSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, TagsTableEDXSig, sizeof(TagsTableEDXSig), 6, 0, NULL, ForwardTagsTableEDXCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, TagsTableEBXSig, sizeof(TagsTableEBXSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, TagsTableEBXSig, sizeof(TagsTableEBXSig), 6, 0, NULL, ForwardTagsTableEBXCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, TagsTableEBPSig, sizeof(TagsTableEBPSig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, TagsTableEBPSig, sizeof(TagsTableEBPSig), 6, 0, NULL, ForwardTagsTableEBPCC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, TagsTableESISig, sizeof(TagsTableESISig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, TagsTableESISig, sizeof(TagsTableESISig), 6, 0, NULL, ForwardTagsTableESICC, true, Address));
	while (DWORD Address = FindCodeFromHandle(hhalo, TagsTableEDISig, sizeof(TagsTableEDISig)))
		Patches.NewPatch(new CodeCaveHook(hhalo, TagsTableEDISig, sizeof(TagsTableEDISig), 6, 0, NULL, ForwardTagsTableEDICC, true, Address));

	ENCRYPT_END
}