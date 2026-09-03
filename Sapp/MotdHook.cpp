#include "MotdHook.h"

#include "Addresses.h"
#include "HaloUtilities.h"
#include "Patcher.h"

__declspec(naked) void MotdCodeCave()
{
	__asm
	{
		push Motd
		call BroadcastMotd

		add esp, 0x0c
		pop edi
		pop esi
		pop ebx
		mov esp, ebp
		pop ebp
		retn
	}
}

void InstallMotdHook()
{
	Patches.NewPatch(new CodeCaveHook("Motd Hook", MotdCode, 6, 12, NULL, MotdCodeCave));
}