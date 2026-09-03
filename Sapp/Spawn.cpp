#include "Spawn.h"

#include "Addresses.h"
#include "AimbotScan.h"
#include "Event.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "NoLead.h"
#include "Patcher.h"

DWORD FuncGetSpawnPoint;
void __declspec(naked) AFKCodecave()
{
	__asm
	{
		mov eax, ebx
		and eax, 0xffff
		lea ecx, [AFK + eax]
		cmp byte ptr ds:[ecx], 0
		jnz dont_spawn
		lea ecx, [ValidationTimer + eax * 4]
		cmp dword ptr ds:[ecx], 0
		jnz dont_spawn
		push ebx
		call FuncGetSpawnPoint
		add esp, 4
		ret

	dont_spawn:
		mov ax, 0xffff
		ret
	}
}


size_t NextPlayerToSpawn;

DWORD BackToPlayerSpawn;
void __declspec(naked) CreatePlayerObjectCodecave()
{
	__asm
	{
		mov ecx, ebx
		and ecx, 0xffff
		mov NextPlayerToSpawn, ecx
		call dword ptr ds:[FUNC_CREATEOBJECT]
		jmp BackToPlayerSpawn
	}
}


void __stdcall EventPreSpawnHandler(Player_Table_Item* PlayerItem/*, Dynamic_Player* Player*/)
{
	for (int i = 0; i < 16; i++)
	{
		if(PlayerTable[i].PlayerID == PlayerItem->PlayerID)
		{		
			GlobalEventHandler.Execute(i, EVENT_PRESPAWN, std::vector <std::pair<std::wstring, std::wstring>>());
			break;
		}
	}
}

DWORD BackToPreSpawn;
void __declspec(naked) EventPreSpawnCodecave()
{
	__asm
	{
		//push ebp
		push edi
		call EventPreSpawnHandler

		mov NextPlayerToSpawn, -1

		push 0x7ff8
		jmp BackToPreSpawn
	}
}


void __stdcall EventSpawnHandler(Player_Table_Item* PlayerItem, Dynamic_Player* Player)
{
	for (int i = 0; i < 16; i++)
	{
		if (PlayerTable[i].PlayerID == PlayerItem->PlayerID)
		{
			GlobalEventHandler.Execute(i, EVENT_SPAWN, std::vector <std::pair<std::wstring, std::wstring>>());
			if (PlayerAliveOnly(i)) // In case the player got killed in the event (for ex. st command)
			{
				PrevLock[i] = false;
				Locked[i] = 0;
				PendingScores[i] = 0;
				PlayerWarps[i] = 0;
				PlayerCampKills[i] = 0;
				Teleported[i] = true;
				afk_help_timer[i] += 2;
				prev_camera[i] = Player->Camera;
				PrevCamera[i] = Player->Camera;
				PrevHeadPos[i] = Player->HeadCoord;
				GetPlayerPosition(i, PlayerPrevPos[i]);

				LastColor[i][0] = (size_t)(Player->Color.x * 255);
				LastColor[i][1] = (size_t)(Player->Color.y * 255);
				LastColor[i][2] = (size_t)(Player->Color.z * 255);

				if (LastTeleport[i] < 0)
				{
					LastTeleport[i] -= 60;
				}
				else
				{
					LastTeleport[i] = -60;
				}
			}
			break;
		}
	}
}

DWORD SpawnEndFunc;
DWORD BackToSpawn;
void __declspec(naked) EventSpawnCodecave()
{
	__asm
	{
		call SpawnEndFunc

		push ebp
		push edi
		call EventSpawnHandler

		jmp BackToSpawn
	}
}


void InstallEventSpawnHandler()
{
	Patches.NewPatch(new RelativeMemoryValue("GetSpawnPoint Func", AFKCode, 2, &FuncGetSpawnPoint));
	Patches.NewPatch(new CodeCaveHook("AFK", AFKCode, 9, 0, NULL, AFKCodecave, true));
	Patches.NewPatch(new CodeCaveHook("SpawnPlayerObject", EventCreatePlayerObject, 5, 7, &BackToPlayerSpawn, CreatePlayerObjectCodecave));
	Patches.NewPatch(new CodeCaveHook("Event PreSpawn", EventPreSpawnCode, 5, 0, &BackToPreSpawn, EventPreSpawnCodecave));
	Patches.NewPatch(new RelativeMemoryValue("Event Spawn Func", EventSpawnCode, -4, &SpawnEndFunc));
	Patches.NewPatch(new CodeCaveHook("Event Spawn", EventSpawnCode, 5, -5, &BackToSpawn, EventSpawnCodecave));
}