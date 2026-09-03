#include "TeamChange.h"

#include "Addresses.h"
#include "Event.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "Patcher.h"
#include "Prefs.h"
#include "RemoteConsole.h"

void __stdcall EventTeamChange(DWORD i)
{
	if (!PlayerPresent(i))
		return;

	Server->PlayerData[i].team = (char)PlayerTable[i].Team;

#ifdef HALORANK
	if (GlobalPrefs->HaloRank)
	{
		HREventTeamSwitch Event;

		Event.Type = HR_EVENT_TEAM_SWITCH;
		Event.IP = GetIP(Server->MachineInfo[MachineIndex].ip);
		Event.Time = (DWORD)(stats::last_game - Time);
		Event.NewTeam = PlayerTable[i].Team;

		GlobalSAPPServer.SendHREvent(&Event, sizeof(HREventTeamSwitch));
	}
#endif

	GlobalEventHandler.Execute(i, EVENT_TEAM_SWITCH, std::vector <std::pair<std::wstring, std::wstring>>());

	if (GlobalPrefs->RemoteConsole)
	{
		GlobalRemoteConsole.SendTeamChange(i);
	}
	
	//wstring team = (Server->PlayerData[i].team = (char)PlayerTable[i].Team) ? L"blue" : L"red";

	//WriteLog(L"TEAMSWITCH", FormatWArgs(L"Player %s switched to %s team", PlayerTable[i].name, team.c_str()));
}

DWORD TeamChangeEndFunc;
void __declspec(naked) EventTeamChangeCodecave()
{
	__asm
	{
		call TeamChangeEndFunc
		add esp, 0x34

		mov eax, dword ptr ss : [esp + 0x20]
		and eax, 0xffff
		push eax
		call EventTeamChange

		pop edi
		pop esi
		pop ebx
		add esp, 0x1c
		ret
	}
}

void InstallTeamChangeHandler()
{
	Patches.NewPatch(new RelativeMemoryValue("Event TeamChange Addr", EventTeamChangeCode, 1, &TeamChangeEndFunc));
	Patches.NewPatch(new CodeCaveHook("Event TeamChange", EventTeamChangeCode, 5, 0, NULL, EventTeamChangeCodecave));
}