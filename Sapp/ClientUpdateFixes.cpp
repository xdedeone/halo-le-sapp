#include "ClientUpdateFixes.h"

#include "Addresses.h"
#include "EventScan.h"
#include "Patcher.h"
#include "Prefs.h"

void __stdcall ClientUpdateFixes(Player_Table_Item* player, byte* Flags)
{
	if (AliveTimer[player->Prescene.player_index] > 60)
	{
		if (player->ObjectIndexNum != 0xFFFF)
		{
			Dynamic_Player* Player = ObjectTable[player->ObjectIndexNum].Offset;
			if (Player->Zoomed != 0xFF)
			{
				*Flags &= 0x7f; // block melee
			}
		}
	}
	else if (GlobalPrefs->AntiLagSpawn)
	{
		*Flags &= 0xfd; // block jump
	}
}

__declspec(naked) void ClientUpdateCodecave()
{
	__asm
	{
		lea ebp, [edx + esp + 0x1b4]

		push eax

		push ebp
		push edi
		call ClientUpdateFixes

		pop eax

		cmp al, 1
		ret
	}
}

void InstallClientUpdateFixes()
{
	Patches.NewPatch(new CodeCaveHook("Client Update", ClientUpdateCode, 9, 0, NULL, ClientUpdateCodecave, true));
}