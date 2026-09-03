#include "JoinPatch.h"

#include "Addresses.h"
#include "Patch.h"
#include "MiscFunctions.h"
#include "NetworkEvents.h"

// short MyNameSig[] = { 0x33, 0xC0, 0x50, 0x50, 0xB9, -1, -1, -1, -1, 0xBF, -1, -1, -1, -1, 0x68, -1, -1, -1, -1, 0xF3, 0xAB, 0xE8, -1, -1, -1, -1, 0x83, 0xCF, 0xFF, 0x33, 0xDB };
short JoinSig[] = { 0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0xB8, 0x2C, 0x20, 0x00, 0x00, 0xE8 }; // 558BEC83E4F8B82C200000E8
//short JoinMiddleSig[] = { 0xF3, 0xA5, 0x8A, 0x48, 0x0C, 0x6A, 0x0B }; // F3A58A480C6A0B
short JoinEndSig[] = {  0x83, 0xC4, 0x08, 0xC6, 0x46, 0x1C, 0x00, 0x80, 0x8B, 0x1E, 0x0F, 0x00, 0x00, 0x02 }; // 83C408C6461C00808B1E0F000002
short NameSig[] = { 0xB9, 0xFF, 0x07, 0x00, 0x00, 0xBE, -1, -1, -1, -1, 0x8D, 0x7C, 0x24, 0x10 }; // B9FF070000BE????????8D7C2410

wchar_t* name = NULL;

struct ServerInfo
{
	DWORD ip;
	BYTE zeros[12];
	WORD protocol;
	WORD port;
};

void __stdcall JoinHook(ServerInfo* info)
{
	ENCRYPT_BEGIN
	Info.ServerID = 0;
	Info.IP = info->ip;
	Info.Port = info->port;
	memcpy(&Info.Name, name, 24);
	ENCRYPT_END
}

DWORD BackToJoin;
void __declspec(naked) JoinHookCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		pushad
		push ecx
		call JoinHook
		popad
		ENCRYPT_END
		push ebp
		mov ebp, esp
		and esp, 0xfffffff8
		jmp BackToJoin;
	}
}

//DWORD BackToJoinMiddle;
//void __declspec(naked) JoinMiddleHookCC()
//{
//	__asm
//	{
//		ENCRYPT_BEGIN
//		rep movs dword ptr es : [edi], dword ptr ds : [esp]
//		mov cl, byte ptr ds : [eax + 0x0c]
//		pushad
//		call JoinMiddleHook
//		popad
//		ENCRYPT_END
//		jmp BackToJoinMiddle;
//	}
//}

void __stdcall JoinEndHook()
{
	ENCRYPT_BEGIN
	ServerApp = 0;
	ServerAppVersion = 0;
	NetworkEvent<ACSession>(MagicUserKey);
	NetworkEvent<DSHandshake>(APP_AC, Version);
	ENCRYPT_END
}

DWORD BackToJoinEnd;
void __declspec(naked) JoinEndHookCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		add esp, 8
		mov byte ptr ds : [esi + 0x1c], 0
		call JoinEndHook
		ENCRYPT_END
		jmp BackToJoinEnd;
	}
}

void __declspec(noinline) InstallJoinPatch()
{
	ENCRYPT_BEGIN
	Patches.NewPatch(new CodeCaveHook(hhalo, JoinSig, sizeof(JoinSig), 6, 0, &BackToJoin, JoinHookCC));
	//Patches.NewPatch(new CodeCaveHook(hhalo, JoinMiddleSig, sizeof(JoinMiddleSig), 5, 0, &BackToJoinMiddle, JoinMiddleHookCC));
	Patches.NewPatch(new CodeCaveHook(hhalo, JoinEndSig, sizeof(JoinEndSig), 7, 0, &BackToJoinEnd, JoinEndHookCC));
	Patches.NewPatch(new MemoryAddress(hhalo, NameSig, sizeof(NameSig), 6, (DWORD*)&name));
	name++;
	ENCRYPT_END
}