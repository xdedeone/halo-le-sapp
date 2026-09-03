#include "ConsoleHook.h"

#include "Addresses.h"
#include "CommandHandler.h"
#include "Patch.h"

short ConsolePrintSig[] = { 0x83, 0xEC, 0x10, 0x57, 0x8B, 0xF8, 0xA0, -1, -1, -1, -1, 0x84, 0xC0, 0xC7, 0x44, 0x24, 0x04, 0x00, 0x00, 0x80, 0x3F }; // 83EC10578BF8A0????????84C0C74424040000803F
short ConsoleHookSig[] = { 0x8D, -1, 0x24, 0x08, -1, 0x68, -1, -1, -1, -1, 0x53 }; // 8D??2408??68????????53

DWORD BackToConsoleHook;
void __declspec(naked) ConsoleHookCC()
{
	__asm
	{
		pushad
		ENCRYPT_BEGIN	
		push edi
		call CommandHandler
		ENCRYPT_END
		test al, al
		popad
		jnz command_found
		lea eax, [esp + 8]
		push eax	
		jmp BackToConsoleHook

	command_found:
		pop esi
		mov al, bl
		pop ebx
		add esp, 0x500
		retn
	}
}

void __declspec(noinline) InstallConsoleHook()
{
	ENCRYPT_BEGIN
	Patches.NewPatch(new FunctionAddress(hhalo, ConsolePrintSig, sizeof(ConsolePrintSig), 0, &PrintConsoleFunc));
	Patches.NewPatch(new CodeCaveHook(hhalo, ConsoleHookSig, sizeof(ConsoleHookSig), 5, 0, &BackToConsoleHook, ConsoleHookCC));
	ENCRYPT_END
}