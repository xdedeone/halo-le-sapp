#include "Killed.h"

#include "Addresses.h"
#include "AimbotScan.h"
#include "Areas.h"
#include "ComboFix.h"
#include "Event.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "LogThread.h"
#include "NoLead.h"
#include "Patcher.h"
#include "Prefs.h"
#include "TextUtilities.h"

using namespace std;

void __stdcall EventKilledHandler(DWORD Killed, DWORD Killer, DWORD KillerObject)
{
	if (!ServerRunning() || Server->ServerState != 1)
		return;

	Killed &= 0xFFFF;
	Killer &= 0xFFFF;

	if (Killed >= 16)
		return;

	if (Killer < 16)
	{
		if (Killed == Killer)
		{
			stats::Suicides++;
			GlobalEventHandler.Execute(Killed, EVENT_SUICIDE, vector <pair<wstring, wstring>>());
		}
		else if (PlayerTable[Killed].Team == PlayerTable[Killer].Team)
		{
			stats::Betrays++;
			vector <pair<wstring, wstring>> EventArgs;
			EventArgs.emplace_back(L"$killed", ToWStr(IndexToNumber(Killed) + 1));
			GlobalEventHandler.Execute(Killer, EVENT_BETRAY, EventArgs);
		}
		else
		{
			stats::Kills++;
			vector <pair<wstring, wstring>> EventArgs;
			EventArgs.emplace_back(L"$killed", ToWStr(IndexToNumber(Killed) + 1));
			GlobalEventHandler.Execute(Killer, EVENT_KILL, EventArgs);

			if (*GameTicks < PlayerTable[Killer].LastBulletTimestamp + 16)
				LastBulletKill[Killer] = 0;
		}
	}

	LastKill[Killed] = 0;

	AliveTimer[Killed] = 0;
	PlayerCoords[Killed].clear();
	GlobalAreas.DeletePlayer(Killed);
	ResetHistory(Killed);

#ifdef HALORANK
	if (GlobalPrefs->HaloRank)
	{
		HREventKill Event;

		Event.Type = HR_EVENT_KILL;
		Event.IP = GetIP(Server->MachineInfo[IndexToNumber(Killed)].ip);
		Event.Time = (DWORD)(stats::last_game - time(0));
		if (PlayerKillerIndex > 0)
		{
			Event.Multi = PlayerTable[Killer].combo;
			Event.Spree = PlayerTable[Killer].killstreak;
			Event.Killer = GetIP(Server->MachineInfo[IndexToNumber(Killer)].ip);
		}
		else
			Event.Killer = 0;

		GlobalSAPPServer.SendHREvent(&Event, sizeof(HREventKill));
	}
#endif

	int eKiller;
	if (KillerObject != 0xFFFFFFFF)
	{
		if (Killer != -1)
			eKiller = IndexToNumber(Killer) + 1;
		else
			eKiller = 0;
	}
	else
		eKiller = -1;

	if (AntiCheat && eKiller > 0)
	{
		DWORD MachineIndex = eKiller - 1;
		for (size_t i = 0; i < AnticheatPlayers.size(); i++)
		{
			if (AnticheatPlayers[i].MachineID == MachineIndex)
			{
				LastKiller[Killed] = AnticheatPlayers[i].SessionKey;
				break;
			}
		}
	}

	vector <pair<wstring, wstring>> EventArgs;
	EventArgs.emplace_back(L"$killer", ToWStr(eKiller));
	GlobalEventHandler.Execute(Killed, EVENT_DIE, EventArgs);
}

DWORD HaloEventKilledFunc;
DWORD BackToEventKilled;
#ifndef HPC
void __declspec(naked) EventKilledCodecave()
{
	__asm
	{
		push eax
		push edi
		push ecx
		mov edx, esi
		call HaloEventKilledFunc

		mov ecx, [esp]
		push ecx
		push esi
		push edi
		call EventKilledHandler

		jmp BackToEventKilled
	}
}
#else
void __declspec(naked) EventKilledCodecave()
{
	__asm
	{
		push edx
		push eax
		push ecx
		mov edx, esi
		call HaloEventKilledFunc

		mov ecx, [esp]
		mov eax, [esp + 4]
		push ecx
		push esi
		push eax
		call EventKilledHandler

		jmp BackToEventKilled
	}
}
#endif

void InstallEventKilledHandler()
{
	Patches.NewPatch(new RelativeMemoryValue("Event Killed Func", EventKilled, 6, &HaloEventKilledFunc));
	Patches.NewPatch(new CodeCaveHook("Event Killed", EventKilled, 10, 0, &BackToEventKilled, EventKilledCodecave));
}