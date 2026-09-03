#include "PlayerJoin-Leave.h"

#include "Addresses.h"
#include "AimbotScan.h"
#include "AliasThread.h"
#include "chat.h"
#include "ComboFix.h"
#include "CommandExecuter.h"
#include "Event.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "LogThread.h"
#include "Mapcycle.h"
#include "NetworkClient.h"
#include "OnlineAdmins.h"
#include "Patcher.h"
#include "Prefs.h"
#include "rcon.h"
#include "RemoteConsole.h"
#include "TextUtilities.h"
#include "ThreadCycleFix.h"
#include "UserList.h"

using namespace std;

map <ULONGLONG, unique_ptr<PL>> PlayersLeft;
map <ULONGLONG, unique_ptr<PL_Stats>> PlayersLeftStats;
Player_Stats PlayerStats[16];
size_t LoginFails[16] = {};
bool stfu[16] = {};

extern DWORD sv_say_player;

void __stdcall MJoin(PresenceItem* Machine)
{
	size_t PlayerIndex = Machine->player_index;
	size_t MachineIndex = Machine->machine_index;	

	PlayerIndexes[MachineIndex] = PlayerIndex;
	MachineIndexes[PlayerIndex] = MachineIndex;

	if (++Server->PlayerNumber == 1)
	{
		sv_say_player = PlayerIndex;
	}

	for (size_t i = 0; i < 11; i++)
	{
		switch (Server->PlayerData[PlayerIndex].name[i])
		{
		case '\t':
		case '\n':
		case '\r':
		case ',':
		case ':':
		case ';':
		case '\\':
			Server->PlayerData[PlayerIndex].name[i] = ' ';
			break;
		}
	}

	Server->PlayerData[PlayerIndex].name[11] = 0;

	NetInfoStruct* pInfo = **Server->MachineInfo[IndexToNumber(PlayerIndex)].IPAddress;
	PlayerID[PlayerIndex] = ((ULONGLONG)pInfo->IP << 16) | pInfo->Port;

	if (AntiCheat)
	{
		ValidationTimer[PlayerIndex] = 600;
	}

	WriteLog(L"JOIN", FormatWArgs(L"Player Joined, Name: %ls  PlayerNumber: %d  IP: %hs  CD-Key Hash: %hs", Server->PlayerData[PlayerIndex].name, PlayerIndex, GetPlayerIP(MachineIndex).c_str(), GetPlayerHash(MachineIndex).c_str()));
	int level = GlobalUserList.GetPlayerPrivilegeLevel(PlayerIndex);
	if (level > -1)
	{
		Login(PlayerIndex, level);
	}

	if (GlobalPrefs->SappConsole)
		PrintConsole(FormatArgs("Player %2d%12hs joined, IP:%22hs | Players: %2u / %u", MachineIndex + 1, WToStr(Server->PlayerData[PlayerIndex].name).c_str(), GetPlayerIP(MachineIndex).c_str(), Server->PlayerNumber, Server->MaxPlayers).c_str());

	if (GlobalPrefs->CustomSleep != GlobalPrefs->OriginalCustomSleep)
	{
		GlobalPrefs->CustomSleep = GlobalPrefs->OriginalCustomSleep;
		SetHaloTimer();
	}

	if (GlobalPrefs->Alias && !GlobalPrefs->OnlyValidAlias)
		AddAlias(GetPlayerHash(MachineIndex), Server->PlayerData[PlayerIndex].name);

	GlobalEventHandler.Execute(PlayerIndex, EVENT_PREJOIN, vector <pair<wstring, wstring>>());
}

void __declspec(naked) MJoinCodecave()
{
	__asm
	{
		sub edi, 0x20
		push edi
		call MJoin
		ret
	}
}

void InstallMJoinHandler()
{
	Patches.NewPatch(new CodeCaveHook("Event Machine Join", EventMJoinCode, 7, 0, NULL, MJoinCodecave, true));
}


void __stdcall MLeave(PresenceItem* Machine)
{
	size_t PlayerIndex = Machine->player_index;
	size_t MachineIndex = Machine->machine_index;
	
	if (sv_say_player == PlayerIndex)
	{
		for (size_t i = 0; i < 16; i++)
		{
			if (PlayerIndex != i && Server->PlayerData[i].player_status == 0)
			{
				sv_say_player = i;
				break;
			}
		}
	}

	for (size_t i = 0; i < AnticheatPlayers.size(); i++)
	{
		if (AnticheatPlayers[i].MachineID == MachineIndex)
		{
			GlobalSAPPServer.SendBigEvent(event_leave_server, (char*)&AnticheatPlayers[i].SessionKey, 4);
			AnticheatPlayers.erase(AnticheatPlayers.begin() + i);
			break;
		}
	}

	if (pUpdateServerQueuesTableHeader != NULL)
	{
		Update_Server_Queues_Table* UpdateServerQueuesTable = (*pUpdateServerQueuesTableHeader)->First;

		if (UpdateServerQueuesTable[PlayerIndex].PlayerID != 0)
		{
			if (UpdateServerQueuesTable[PlayerIndex].ptr5280b != NULL)
			{
				GlobalFree(UpdateServerQueuesTable[PlayerIndex].ptr5280b);
				UpdateServerQueuesTable[PlayerIndex].ptr5280b = 0;
			}
			if (UpdateServerQueuesTable[PlayerIndex].ptr480b != NULL)
			{
				GlobalFree(UpdateServerQueuesTable[PlayerIndex].ptr480b);
				UpdateServerQueuesTable[PlayerIndex].ptr480b = 0;
			}
		}
	}

	unique_ptr<PL> Player = unique_ptr<PL>(new PL);
	Player->joined = PlayerStats[PlayerIndex].Joined;
	Player->left = time(0);
	Player->afk = afk_timer[PlayerIndex];
	Player->AimbotScores = AimbotScores[PlayerIndex];
	Player->LoginFails = LoginFails[PlayerIndex];
	Player->RconFails = RconFails[PlayerIndex];
	Player->IsAFK = AFK[PlayerIndex];
	Player->stfu = stfu[PlayerIndex];
	Player->RespawnTimer = PlayerTable[PlayerIndex].RespawnTimer;
	Player->tk_number = PlayerTable[PlayerIndex].tk_number;
	Player->tk_timer = PlayerTable[PlayerIndex].tk_timer;
	Player->Kills = PlayerStats[PlayerIndex].Kills + PlayerTable[PlayerIndex].kills;
	Player->Assists = PlayerStats[PlayerIndex].Assists + PlayerTable[PlayerIndex].assists;
	Player->Deaths = PlayerStats[PlayerIndex].Deaths + PlayerTable[PlayerIndex].deaths;
	Player->LoggedInLvl = GlobalOnlineAdminList.GetAdminLevel(PlayerIndex);
	
	GlobalOnlineAdminList.DeleteAdmin(PlayerIndex);

	ResetPlayerVars(PlayerIndex);

	auto& it = PlayersLeft.find(PlayerID[PlayerIndex]);
	if (it != PlayersLeft.end())
	{
		it->second = move(Player);
	}
	else
	{
		PlayersLeft.emplace(PlayerID[PlayerIndex], move(Player));
	}

	if (GlobalPrefs->SaveScores && Server->ServerState == 1)
	{
		unique_ptr<PL_Stats> NewPlayerStats(new PL_Stats);
		NewPlayerStats->Kills = PlayerTable[PlayerIndex].kills;
		NewPlayerStats->Assists = PlayerTable[PlayerIndex].assists;
		NewPlayerStats->Deaths = PlayerTable[PlayerIndex].deaths;
		NewPlayerStats->RaceLastLap = PlayerTable[PlayerIndex].race_last_lap;
		NewPlayerStats->RaceScore = PlayerTable[PlayerIndex].race_score;
		NewPlayerStats->RaceFastestLap = PlayerTable[PlayerIndex].ctf_score;
		NewPlayerStats->RaceLapStart = PlayerTable[PlayerIndex].race_lap_start;
		NewPlayerStats->SlayerScores = SlayerScores->PlayerScores[PlayerIndex];
		NewPlayerStats->OddballScores = OddballScores->PlayerScores[PlayerIndex];
		NewPlayerStats->RaceFirstFlag = RaceScores->FirstFlagtouched[PlayerIndex];
		NewPlayerStats->RaceFlags = RaceScores->FlagsTouched[PlayerIndex];

		auto& jt = PlayersLeftStats.find(PlayerID[PlayerIndex]);
		if (jt != PlayersLeftStats.end())
		{
			jt->second = move(NewPlayerStats);
		}
		else
		{
			PlayersLeftStats.emplace(PlayerID[PlayerIndex], move(NewPlayerStats));
		}
	}

	WriteLog(L"QUIT", FormatWArgs(L"Player Quit, Name: %ls  PlayerNumber: %d", Server->PlayerData[PlayerIndex].name, PlayerIndex));

	if (GlobalPrefs->SappConsole)
		PrintConsole(FormatArgs("Player %2d%12hs left | Players: %2u / %u", IndexToNumber(PlayerIndex) + 1, WToStr(Server->PlayerData[PlayerIndex].name).c_str(), Server->PlayerNumber - 1, Server->MaxPlayers).c_str());

#ifdef HALORANK
	if (GlobalPrefs->HaloRank)
	{
		HREventLeave Event;

		Event.Type = HR_EVENT_PLAYER_LEAVE;
		Event.IP = GetIP(Server->MachineInfo[MachineIndex].ip);
		Event.Time = (DWORD)(stats::last_game - Time);
		Event.Reserved = 0;

		GlobalSAPPServer.SendHREvent(&Event, sizeof(HREventLeave));
	}
#endif

	if (GlobalPrefs->MapSkip && Server->ServerState == 1)
	{
		size_t RealPlayerNumber = 0;
		size_t Skippers = 0;
		for (size_t i = 0; i < PlayerHeader->SlotsTaken; i++)
		{
			if (PlayerIndex != i && PlayerPresent(i))
			{
				RealPlayerNumber++;
				if (Skips[i])
					Skippers++;
			}
		}

		if (RealPlayerNumber > 0 && (int)ceil((float)(GlobalPrefs->MapSkip / (100.0f / RealPlayerNumber))) - Skippers <= 0)
		{
			sv_say(FormatWArgs(L"%ls quit, now there are enough votes to skip!", Server->PlayerData[PlayerIndex].name).c_str());
			GlobalPrefs->MapSkip = 0;
			if (!GlobalPrefs->SappMapcycle || !GlobalMapcycle.Next())
				ExecuteHaloCommand(-1, Enviroment::CONSOLE, "sv_map_next");
		}
	}

	if (GlobalPrefs->CustomSleep < 33 && Server->PlayerNumber == 0)
	{
		GlobalPrefs->CustomSleep = 33;
		SetHaloTimer();
	}

	GlobalEventHandler.Execute(PlayerIndex, EVENT_LEAVE, vector <pair<wstring, wstring>>());

	if (GlobalPrefs->RemoteConsole)
	{
		GlobalRemoteConsole.SendPlayerLeave(PlayerIndex);
	}

	PlayerIndexes[MachineIndex] = -1;
	MachineIndexes[PlayerIndex] = -1;

	Machine->machine_index = -1;
	Machine->player_status = -1;
	Machine->team = -1;
	Machine->player_index = -1;
}

void __declspec(naked) MLeaveCodecave()
{
	__asm
	{
		push eax

		push eax
		call MLeave

		pop eax

		or ecx, -1
		ret
	}
}

void InstallMLeaveHandler()
{
	Patches.NewPatch(new CodeCaveHook("Event Machine Leave", EventMLeaveCode, 12, 0, NULL, MLeaveCodecave, true));
}


void __stdcall CleanMachines()
{
	Server->GamesPlayed++;

	for (size_t i = 0; i < 16; i++)
	{
		if (PlayerPresent(i))
		{
			if (pUpdateServerQueuesTableHeader != NULL)
			{
				Update_Server_Queues_Table* UpdateServerQueuesTable = (Update_Server_Queues_Table*)(*pUpdateServerQueuesTableHeader)->First;

				if (UpdateServerQueuesTable[i].PlayerID != 0)
				{
					if (UpdateServerQueuesTable[i].ptr5280b != NULL)
					{
						GlobalFree(UpdateServerQueuesTable[i].ptr5280b);
						UpdateServerQueuesTable[i].ptr5280b = 0;
					}
					if (UpdateServerQueuesTable[i].ptr480b != NULL)
					{
						GlobalFree(UpdateServerQueuesTable[i].ptr480b);
						UpdateServerQueuesTable[i].ptr480b = 0;
					}
				}
			}

			unique_ptr<PL> Player(new PL);
			Player->joined = PlayerStats[i].Joined;
			Player->left = time(0);
			Player->afk = afk_timer[i];
			Player->AimbotScores = AimbotScores[i];
			Player->LoginFails = LoginFails[i];
			Player->RconFails = RconFails[i];
			Player->IsAFK = AFK[i];
			Player->stfu = stfu[i];
			Player->RespawnTimer = 0;
			Player->tk_number = PlayerTable[i].tk_number;
			Player->tk_timer = PlayerTable[i].tk_timer;
			Player->Kills = PlayerStats[i].Kills + PlayerTable[i].kills;
			Player->Assists = PlayerStats[i].Assists + PlayerTable[i].assists;
			Player->Deaths = PlayerStats[i].Deaths + PlayerTable[i].deaths;
			Player->LoggedInLvl = GlobalOnlineAdminList.GetAdminLevel(i);
			
			ResetPlayerVars(i);

			auto& it = PlayersLeft.find(PlayerID[i]);
			if (it != PlayersLeft.end())
			{
				it->second = move(Player);
			}
			else
			{
				PlayersLeft.emplace(PlayerID[i], move(Player));
			}

			WriteLog(L"QUIT", FormatWArgs(L"Player Quit, Name: %ls  PlayerNumber: %d", Server->PlayerData[i].name, i));
		}
	}

	GlobalOnlineAdminList.Clear();

	for (size_t i = 0; i < AnticheatPlayers.size(); i++)
	{
		GlobalSAPPServer.SendBigEvent(event_leave_server, (char*)&AnticheatPlayers[i].SessionKey, 4);
	}
	AnticheatPlayers.clear();
}

void __declspec(naked) CleanMachinesCodecave()
{
	__asm
	{
		call CleanMachines
		ret
	}
}

void InstallCleanMachinesHandler()
{
	Patches.NewPatch(new CodeCaveHook("Event Clean Machines", EventCleanMachines, 6, 0, NULL, CleanMachinesCodecave, true));
}


void __stdcall PJoin(size_t PlayerIndex)
{
	int MachineIndex = IndexToNumber(PlayerIndex);
	if (MachineIndex == -1)
		return;

	LastKill[PlayerIndex] = 0;

	LastTeleport[PlayerIndex] = -300;

	time_t Time = time(0);

	PlayerStats[PlayerIndex].Joined = Time;
	afk_timer[PlayerIndex] = 0;

	PlayerStats[PlayerIndex].Kills = 0;
	PlayerStats[PlayerIndex].Assists = 0;
	PlayerStats[PlayerIndex].Deaths = 0;

	if (GlobalPrefs->SaveScores)
	{
		auto& it = PlayersLeftStats.find(PlayerID[PlayerIndex]);
		if (it != PlayersLeftStats.end())
		{
			auto& ptr = it->second;

			PlayerTable[PlayerIndex].kills = ptr->Kills;
			PlayerTable[PlayerIndex].assists = ptr->Assists;
			PlayerTable[PlayerIndex].deaths = ptr->Deaths;
			PlayerTable[PlayerIndex].race_last_lap = ptr->RaceLastLap;
			PlayerTable[PlayerIndex].race_score = ptr->RaceScore;
			PlayerTable[PlayerIndex].race_fastest_lap = ptr->RaceFastestLap;
			PlayerTable[PlayerIndex].race_lap_start = ptr->RaceLapStart;
			SlayerScores->PlayerScores[PlayerIndex] = ptr->SlayerScores;
			OddballScores->PlayerScores[PlayerIndex] = ptr->OddballScores;
			RaceScores->FirstFlagtouched[PlayerIndex] = ptr->RaceFirstFlag;
			RaceScores->FlagsTouched[PlayerIndex] = ptr->RaceFlags;			

			if (!*TeamGame)
			{
				SlayerScores->TeamScores[PlayerIndex] = ptr->SlayerScores;
				OddballScores->TeamScores[PlayerIndex] = ptr->OddballScores;
				KingScores->TeamScores[PlayerIndex] = ptr->RaceLastLap;
				RaceScores->TeamScores[PlayerIndex] = ptr->RaceScore;
			}
		}
	}

	auto& it = PlayersLeft.find(PlayerID[PlayerIndex]);
	if (it != PlayersLeft.end())
	{
		auto& ptr = it->second;
		if (Time - ptr->left < 20)
			afk_timer[PlayerIndex] = ptr->afk;

		AimbotScores[PlayerIndex] = ptr->AimbotScores;
		LoginFails[PlayerIndex] = ptr->LoginFails;
		RconFails[PlayerIndex] = ptr->RconFails;
		AFK[PlayerIndex] = ptr->IsAFK;
		stfu[PlayerIndex] = ptr->stfu;
		RespawnTimeLeft[PlayerIndex] = ptr->RespawnTimer;
		PlayerTable[PlayerIndex].tk_number = ptr->tk_number;
		PlayerTable[PlayerIndex].tk_timer = ptr->tk_timer;
		PlayerStats[PlayerIndex].Joined = ptr->joined;
		PlayerStats[PlayerIndex].Kills = ptr->Kills - PlayerTable[PlayerIndex].kills;
		PlayerStats[PlayerIndex].Assists = ptr->Assists - PlayerTable[PlayerIndex].assists;
		PlayerStats[PlayerIndex].Deaths = ptr->Deaths - PlayerTable[PlayerIndex].deaths;
		if (ptr->LoggedInLvl > -1)
		{
			GlobalOnlineAdminList.SetAdminLevel(PlayerIndex, ptr->LoggedInLvl);
		}
	}

	Server->PlayerData[PlayerIndex].team = (char)PlayerTable[PlayerIndex].Team;

	//PrintConsole(FormatArgs("Player %S joined to %d team, PlayerNumber: %d", Server->PlayerData[PlayerIndex].name, PlayerTable[PlayerIndex].Team, PlayerIndex).c_str());

	//if(*TeamGame)
	//{
	//	wstring team = PlayerTable[PlayerIndex].Team == 0 ? L"red" : L"blue";
	//	WriteLog(L"JOIN_TEAM", FormatWArgs(L"Player %s joined to %s team, PlayerNumber: %d", Server->PlayerData[PlayerIndex].name, team.c_str(), PlayerIndex));
	//}

#ifdef HALORANK
	if (GlobalPrefs->HaloRank)
	{
		HREventJoin Event;

		Event.Type = HR_EVENT_PLAYER_JOIN;
		Event.IP = GetIP(Server->MachineInfo[MachineIndex].ip);
		Event.Time = (DWORD)(stats::last_game - Time);
		wcscpy_s(Event.Name, 12, Server->PlayerData[PlayerIndex].name);
		strcpy_s(Event.IPstr, 32, Server->MachineInfo[MachineIndex].ip);
		strcpy_s(Event.Hash, 32, GetPlayerHash(MachineIndex));
		Event.Team = PlayerTable[PlayerIndex].Team;

		GlobalSAPPServer.SendHREvent(&Event, sizeof(HREventJoin));
	}
#endif

	GlobalEventHandler.Execute(PlayerIndex, EVENT_JOIN, vector <pair<wstring, wstring>>());

	if (GlobalPrefs->RemoteConsole)
	{
		GlobalRemoteConsole.SendPlayerJoin(PlayerIndex);
	}
}

DWORD JCall;
void __declspec(naked) PJoinCodecave()
{
	__asm
	{
		call JCall

		and esi, 0xffff
		push esi
		call PJoin
		ret
	}
}

void InstallPJoinHandler()
{
	Patches.NewPatch(new RelativeMemoryValue("Event Player Join Addr", EventPJoinCode, 1, &JCall));
	Patches.NewPatch(new CodeCaveHook("Event Player Join", EventPJoinCode, 5, 0, NULL, PJoinCodecave, true));
}