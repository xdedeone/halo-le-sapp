#include "SendChatMessage.h"

#include "Addresses.h"
#include "Patch.h"

short ChatFuncSig[] = { 0xA1, -1, -1, -1, -1, 0x83, 0xEC, 0x08, 0x83, 0xF8, 0x08, 0x7C, 0x05, 0xE8 }; // A1????????83EC0883F8087C05E8

DWORD ChatFunc;

void __declspec(naked) SendChatMessage(const wchar_t* msg)
{
	__asm
	{
		ENCRYPT_BEGIN
		push ebp
		mov ebp, esp
		pushad
		pushfd
		mov eax, msg
		push eax
		call ChatFunc
		add esp, 4
		popfd
		popad
		mov esp, ebp
		pop ebp
		ENCRYPT_END
		retn
	}
}

void __declspec(noinline) InstallChatMessageSender()
{
	ENCRYPT_BEGIN
	Patches.NewPatch(new FunctionAddress(hhalo, ChatFuncSig, sizeof(ChatFuncSig), 0, &ChatFunc));
	ENCRYPT_END
}