#include "Assist.h"

#include "Addresses.h"
#include "Event.h"
#include "Patcher.h"

void __stdcall EventAssistHandler(Player_Table_Item* Player)
{
	Player->assists++;

	for (size_t i = 0; i < 16; i++)
	{
		if (&PlayerTable[i] == Player)
		{
			GlobalEventHandler.Execute(i, EVENT_ASSIST, std::vector <std::pair<std::wstring, std::wstring>>());
			break;
		}
	}

#ifdef HALORANK
	if (GlobalPrefs->HaloRank)
	{
		HREventAssist Event;

		int PlayerIndex;
		for (PlayerIndex = 0; PlayerIndex < 16; PlayerIndex++)
		{
			if (&PlayerTable[PlayerIndex] == Player)
				break;
		}

		Event.Type = HR_EVENT_ASSIST;
		Event.IP = GetIP(Server->MachineInfo[IndexToNumber(PlayerIndex)].ip);
		Event.Time = (DWORD)(stats::last_game - time(0));
		Event.Reserved = 0;

		GlobalSAPPServer.SendHREvent(&Event, sizeof(HREventAssist));
	}
#endif
}

DWORD ContinueEventAssist;
void __declspec(naked) EventAssistCodecave()
{
	__asm
	{
		push esi
		call EventAssistHandler

		jmp ContinueEventAssist
	}
}

void InstallEventAssistHandler()
{
	Patches.NewPatch(new CodeCaveHook("Event Assist", EventAssist, 7, 0, &ContinueEventAssist, EventAssistCodecave));
}