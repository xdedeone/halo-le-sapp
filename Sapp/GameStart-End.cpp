#include "GameStart-End.h"

#include "Addresses.h"
#include "Areas.h"
#include "bans.h"
#include "Event.h"
#include "EventScan.h"
#include "GSQueryPatch.h"
#include "HaloUtilities.h"
#include "LogThread.h"
#include "MapCheckAndExecute.h"
#include "Mapcycle.h"
#include "MapVote.h"
#include "NetworkHook.h"
#include "NoLead.h"
#include "ObjectInteractions.h"
#include "ObjectManagement.h"
#include "Patcher.h"
#include "PlayerJoin-Leave.h"
#include "Prefs.h"
#include "RemoteConsole.h"
#include "TextUtilities.h"
#include "ThreadCycleFix.h"

using namespace std;

void __stdcall GameStartHandler()
{
	if(ServerRunning())
	{
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	#ifdef HALORANK
		stats::last_game = time(0);
	#endif

		PlayersLeftStats.clear();

		MapVoteCanceled = false;

		MapExecuted = false;

		GlobalPrefs->MapSkip = GlobalPrefs->OriginalMapSkip;

		CacheCurrentGameVars();

		GlobalAreas.CacheAreas();

		WriteLog(L"NEW GAME", FormatWArgs(L"Map: %ls  GameType: %ls", CurrentMapW.c_str(), CurrentModeW.c_str()));

		if(GlobalPrefs->SappConsole)
			PrintConsole(15, FormatArgs("New Game, Map: %hs, Mode: %hs", CurrentMap.c_str(), CurrentMode.c_str()).c_str());

		if(GlobalPrefs->SappMapcycle)
		{
			const MapcycleItem* Next = GlobalMapcycle.WhatsNext();
			if(Next)
			{
				NextMap = Next->GetMap();
				NextMode = Next->GetMode();
				FormatQueryString();
			}
		}

	#ifdef HALORANK
		if(GlobalPrefs->HaloRank)
		{
			HREventGameStart Event;

			Event.Type = HR_EVENT_GAME_START;
			Event.IP = 0;
			Event.Time = 0;
			strcpy_s(Event.MapName, 32, Server->MapName);
			strcpy_s(Event.GTName, 32, tmpGT.c_str());
			strcpy_s(Event.GameType, 31, tmp.c_str());
			Event.TeamGame = *TeamGame;

			GlobalSAPPServer.SendHREvent(&Event, sizeof(HREventGameStart));
		}
	#endif

		GlobalEventHandler.Execute(-1, EVENT_GAME_START, vector <pair<wstring, wstring>>());

		if (GlobalPrefs->RemoteConsole)
		{
			GlobalRemoteConsole.SendNewGame();
		}
	}
}

void __declspec(naked) GameStartCodecave()
{
	__asm
	{
		call GameStartHandler

		pop edi
		pop esi
		pop ebp
		pop ebx
		pop ecx
		ret
	}
}

void InstallGameStartHandler()
{
	Patches.NewPatch(new CodeCaveHook("GameStart", GameStartCode, 6, 8, NULL, GameStartCodecave));
}


DWORD __stdcall GameEndHandler(DWORD Type)
{
	if (Type == 1)
	{
		GlobalEventHandler.Execute(-1, EVENT_GAME_END, vector <pair<wstring, wstring>>());
	}
	else if(Type == 2)
	{
		if(GlobalPrefs->CustomSleep < 33)
		{
			GlobalPrefs->CustomSleep = 33;
			SetHaloTimer();
		}

		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

		stats::Games++;

		time_t Time = time(0) - 86400; // 1 day ago		
		for(auto& it = PlayersLeft.begin(); it != PlayersLeft.end();)
		{
			if(it->second->left < Time)
			{
				it = PlayersLeft.erase(it);
			}
			else ++it;
		}

		GlobalIPBanlist.Refresh();
		GlobalTextBans.Refresh();
		GlobalMutedPlayers.Refresh();

		GlobalMapVotes.ClearVotes();

		SpawnedVehicles.clear();

		ObjectCache.clear();

		JoinReqests.clear();

		DoSList.clear();

		IPs.clear();

		game_break = 0;
		timeout = (*MapcycleTimeout + 5) * 30;

#ifdef HALORANK
		if(GlobalPrefs->HaloRank)
		{
			HREventGameEnd Event;

			Event.Type = HR_EVENT_GAME_END;
			Event.IP = 0;
			Event.Time = (DWORD)(stats::last_game - Time);
			Event.Reserved = 0;

			GlobalSAPPServer.SendHREvent(&Event, sizeof(HREventGameEnd));
		}
#endif
	}

	return Type;
}

DWORD ContinueGameEnd;
void __declspec(naked) GameEndCodecave()
{
	__asm
	{
		push eax
		call GameEndHandler

		sub esp, 0x0c
		push 0
		jmp ContinueGameEnd
	}
}

#ifdef HPC
void __declspec(naked) GameEndRaceCodecave()
{
	__asm
	{
		push ecx
		push 1
		call GameEndHandler
		pop ecx

		mov edx, 0x7ff8
		ret
	}
}
#else
void __declspec(naked) GameEndRaceCodecave()
{
	__asm
	{
		push 1
		call GameEndHandler

		mov edx, 0x7ff8
		ret
	}
}
#endif

void InstallGameEndHandler()
{
	Patches.NewPatch(new CodeCaveHook("GameEnd", GameEndCode, 5, 0, &ContinueGameEnd, GameEndCodecave));
#ifdef HPC
	Patches.NewPatch(new CodeCaveHook("GameEndRace", GameEndRaceCode, 5, 7, NULL, GameEndRaceCodecave, true));
#else
	Patches.NewPatch(new CodeCaveHook("GameEndRace", GameEndRaceCode, 5, 8, NULL, GameEndRaceCodecave, true));
#endif	
}