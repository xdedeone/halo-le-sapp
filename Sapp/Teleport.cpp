#include "Teleport.h"

#include "Addresses.h"
#include "AimbotScan.h"
#include "EventScan.h"
#include "Patcher.h"

DWORD __stdcall TeleportHandler(DWORD ObjectIndex)
{
	for (size_t i = 0; i < 16; i++)
	{
		if (PlayerTable[i].ObjectIndex == ObjectIndex)
		{
			LastTeleport[i] = 0;
			Teleported[i] = true;
			break;
		}
	}
	return ObjectIndex;
}

DWORD ContinueTeleport;
void __declspec(naked) TeleportCodecave()
{
	__asm
	{
		push eax
		call TeleportHandler

		lea edx, [esp + 0x20]
		push edx
		push eax
		jmp ContinueTeleport
	}
}

void InstallTeleportHandler()
{
	Patches.NewPatch(new CodeCaveHook("Teleport", TeleportCode, 6, 0, &ContinueTeleport, TeleportCodecave));
}