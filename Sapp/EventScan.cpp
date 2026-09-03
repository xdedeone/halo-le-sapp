#include "EventScan.h"

#include "Addresses.h"
#include "AimbotScan.h"
#include "AliasThread.h"
#include "Areas.h"
#include "CDKeyCheck.h"
#include "chat.h"
#include "ComboFix.h"
#include "Event.h"
#include "GSQueryPatch.h"
#include "HaloUtilities.h"
#include "LogThread.h"
#include "MapCheckAndExecute.h"
#include "Mapcycle.h"
#include "MapVote.h"
#include "MiscellaneousFunctions.h"
#include "NetworkClient.h"
#include "NetworkHook.h"
#include "NetworkEvents.h"
#include "NoLead.h"
#include "ObjectInteractions.h"
#include "ObjectManagement.h"
#include "OnlineAdmins.h"
#include "Patcher.h"
#include "PlayerJoin-Leave.h"
#include "Prefs.h"
#include "rcon.h"
#include "TextUtilities.h"
#include "ThreadCycleFix.h"
#include "UserList.h"

using namespace std;

GameType CurrentGameType = GameType::Unknown;

string CurrentMap;
string CurrentMode;
string CurrentGT;
wstring CurrentMapW;
wstring CurrentModeW;
wstring CurrentGTW;

deque <vect3f> PlayerCoords[16];

vector <LocalAntcheatPlayer> AnticheatPlayers;

int timeout = 0;
int game_break = 0;

size_t ValidationTimer[16];

DWORD RespawnTimeLeft[16];

size_t LastKiller[16];

int KillsPrevCamp[16];
bool AFK[16];

ULONGLONG PlayerID[16];

size_t AliveTimer[16];

vect3f prev_camera[16];

size_t afk_timer[16];
size_t afk_help_timer[16];
size_t ping_timer[16];
size_t ping_help_timer[16];
bool WarnedAFK[16];
bool WarnedPing[16];

char camp_timer[16];
char alive_timer[16];

bool PlayerPrevState[16];
bool JustSpawned[16];
size_t SpawnTimer[16];

size_t PlayerCampKills[16];

bool Teleported[16];

DWORD PlayerWarps[16];
vect3f PlayerPrevPos[16];

DWORD LastWarp[16];

size_t LastColor[16][3];

uint32_t PlayerApps[16];

bool IsCamping(const deque <vect3f> &Pos)
{
	vector <vect3f> Buffer(Pos.begin(), Pos.end());

	for(size_t i = 0; i < Buffer.size() - 1; i++)
		for(size_t j = i + 1; j < Buffer.size(); j++)
			if((Buffer[i] - Buffer[j]).Length() > GlobalPrefs->CampDist)
				return false;

	return true;
}

void SetCurrentGameType(const string& gt)
{
	if (!strcmp(gt.c_str(), "ctf"))
	{
		CurrentGameType = GameType::CTF;
	}
	else if (!strcmp(gt.c_str(), "slayer"))
	{
		CurrentGameType = GameType::Slayer;
	}
	else if (!strcmp(gt.c_str(), "oddball"))
	{
		CurrentGameType = GameType::Oddball;
	}
	else if (!strcmp(gt.c_str(), "king"))
	{
		CurrentGameType = GameType::King;
	}
	else if (!strcmp(gt.c_str(), "race"))
	{
		CurrentGameType = GameType::Race;
	}
	else
	{
		CurrentGameType = GameType::Unknown;
	}
}

void CacheCurrentGameVars()
{
	if (ServerRunning())
	{
		CurrentMap = Server->MapName;
		CurrentMapW.assign(CurrentMap.begin(), CurrentMap.end());

		CurrentMode = WToStr(Server->GameType);
		CurrentModeW = Server->GameType;

		if (*CurrentGTPointer != NULL && **CurrentGTPointer != NULL)
		{
			CurrentGT = **CurrentGTPointer;
			CurrentGTW.assign(CurrentGT.begin(), CurrentGT.end());
			SetCurrentGameType(CurrentGT);
		}
	}
}

void LoadEventScan()
{
	if(GlobalPrefs->SappMapcycle)
	{
		const MapcycleItem* Next = GlobalMapcycle.WhatsNext();
		if(Next)
		{
			NextMap = Next->GetMap();
			NextMode = Next->GetMode();
		}
	}

	FormatQueryString();

	time_t Time = time(0);
#ifdef HALORANK
	stats::last_game = Time;
#endif

	if (ServerRunning())
	{
		if (Server->PlayerNumber > 0 && Server->ServerState == 1 && GlobalPrefs->CustomSleep != GlobalPrefs->OriginalCustomSleep)
		{
			GlobalPrefs->CustomSleep = GlobalPrefs->OriginalCustomSleep;
			SetHaloTimer();
		}

		GlobalAreas.CacheAreas();
	}

	for(int i = 0; i < 16; i++) 
	{
		RespawnTimeLeft[i] = 0;
		AliveTimer[i] = 0;
		afk_help_timer[i] = 10;
		ping_help_timer[i] = 300;
		afk_timer[i] = 0;
		ping_timer[i] = 0;
		camp_timer[i] = 0;
		alive_timer[i] = 0;
		KillsPrevCamp[i] = PlayerTable[i].kills;
		WarnedAFK[i] = false;
		WarnedPing[i] = false;
		AFK[i] = false;
		JustSpawned[i] = false;
		SpawnTimer[i] = 0;

		PlayerCampKills[i] = 0;

		Teleported[i] = false;

		PlayerWarps[i] = 0;
		LastWarp[i] = 0;

		PlayerApps[i] = APP_Nothing;

		if(GlobalPrefs->NoLead)
			Lead[i] = 0;
		else
			Lead[i] = 1;

		CustomLead[i] = 0;

		if (AntiCheat)
		{
			ValidationTimer[i] = 600;
		}
		else
		{
			ValidationTimer[i] = 0;
		}

		LastKiller[i] = 0;

		if (ServerRunning() && PlayerPresent(i))
		{		
			int MachineIndex = Server->PlayerData[i].machine_index;

			if (PlayerAliveOnly(i))
			{
				PlayerPrevState[i] = 1;
				GetPlayerPosition(i, PlayerPrevPos[i]);

				Dynamic_Player* Player = ObjectTable[PlayerTable[i].ObjectIndexNum].Offset;
				LastColor[i][0] = (size_t)(Player->Color.x * 255);
				LastColor[i][1] = (size_t)(Player->Color.y * 255);
				LastColor[i][2] = (size_t)(Player->Color.z * 255);
			}
			else
			{
				PlayerPrevState[i] = 0;

				LastColor[i][0] = 0;
				LastColor[i][1] = 0;
				LastColor[i][2] = 0;
			}

			LastKill[i] = PlayerTable[i].LastKillTimestamp;

			PlayerStats[i].Joined = Time;

			NetInfoStruct* pInfo = **Server->MachineInfo[MachineIndex].IPAddress;
			if (pInfo != NULL)
			{
				PlayerID[i] = ((ULONGLONG)pInfo->IP << 16) | pInfo->Port;
			}

			Server->PlayerData[i].team = (char)PlayerTable[i].Team;

			WriteLog(L"PLAYER", FormatWArgs(L"Player Present, Name: %ls  PlayerNumber: %d  IP: %hs  CD-Key Hash: %hs", Server->PlayerData[i].name, i, GetPlayerIP(MachineIndex).c_str(), GetPlayerHash(MachineIndex).c_str()));
			int lvl = GlobalUserList.GetPlayerPrivilegeLevel(i);

			if (lvl > -1)
			{
				Login(i, lvl);
			}

			if (GlobalPrefs->Alias && !GlobalPrefs->OnlyValidAlias)
			{
				AddAlias(GetPlayerHash(MachineIndex), Server->PlayerData[i].name);
			}
		}
		else
		{
			PlayerPrevState[i] = 0;
		}
	}
}

void EventScan()
{
	// DoS protection
	if(*GameTicks % 30 == 0)
	{
		for(auto& it : IPs)
		{
			it.second = 0;
		}
	}

	// Player Events
	for(int i = 0; i < 16; i++)
	{
		if (PlayerPresent(i))
		{
			if (AimbotScores[i] > 0 && *GameTicks % 15 == 0)
				AimbotScores[i]--;

			if (ChatSpam[i] > 0)
				ChatSpam[i]--;

			// anticheat validation
			if (ValidationTimer[i])
			{
				if (ValidationTimer[i] == 450)
				{
					MsgSend(i, "This server requires anticheat in order to play here!");
				}
				else if (ValidationTimer[i] == 300)
				{
					MsgSend(i, "For more information, visit http://halo.isimaginary.com/anticheat");
				}
				else if (ValidationTimer[i] == 150)
				{
					MsgSend(i, "You will be kicked in 5 seconds!");
				}
				else if (ValidationTimer[i] == 1)
				{
					sv_say(FormatWArgs(L"%ls was kicked for not having anticheat.", PlayerTable[i].name).c_str());
					sv_kick(i);
					ValidationTimer[i] = 600;
				}
				ValidationTimer[i]--;
			}

			if (PlayerAliveOnly(i))
			{
				if (RespawnTimeLeft[i] > 90 && GlobalPrefs->SaveRespawnTime)
				{
					ForceSpawn(i, RespawnTimeLeft[i]);
					RespawnTimeLeft[i] = 0;
				}
				else
				{
					AliveTimer[i]++;
					alive_timer[i] = (alive_timer[i] + 1) % 30;
					if (alive_timer[i] == (2 * i) % 30)
					{
						GlobalEventHandler.Execute(i, EVENT_ALIVE, vector <pair<wstring, wstring>>());
					}
				}
			}
			
			if (Server->ServerState == 1)
			{
				// afk check
				if (*GameTicks % 30 == 0)
				{
					bool still;
					if (PlayerAliveOnly(i))
					{
						Dynamic_Player* Player = ObjectTable[PlayerTable[i].ObjectIndexNum].Offset;
						if (GetCameraAngle(Player->Camera, prev_camera[i]) < 0.1f)
						{
							still = true;
						}
						else
						{
							still = false;
						}
						prev_camera[i] = Player->Camera;
					}
					else
					{
						still = true;
					}

					if (afk_help_timer[i] != 0)
						afk_help_timer[i]--;
					else
					{
						if (still)
						{
							afk_timer[i]++;

							if (GlobalPrefs->AFKKick)
							{
								if (afk_timer[i] > GlobalPrefs->AFKKick - 30 && !WarnedAFK[i] && GlobalOnlineAdminList.GetAdminLevel(i) < 0)
								{
									WarnedAFK[i] = true;
									MsgSend(i, FormatWArgs(L"%ls, you will be kicked for inactivity", PlayerTable[i].name).c_str());
								}
								else if (afk_timer[i] > GlobalPrefs->AFKKick && GlobalOnlineAdminList.GetAdminLevel(i) < 0)
								{
									afk_timer[i] = 0;
									WarnedAFK[i] = false;
									wstring wstr = FormatWArgs(L"%ls was kicked for inactivity", PlayerTable[i].name);
									sv_say(wstr.c_str());
									WriteLog(L"KICK", move(wstr));
									sv_kick(i);
								}
							}
						}
						else
						{
							afk_timer[i] = 0;
						}
					}
				}
				// end of afk check

				// high ping check
				if (GlobalPrefs->PingKick)
				{
					if (ping_help_timer[i] != 0)
						ping_help_timer[i]--;
					else if (PlayerTable[i].ping > GlobalPrefs->PingKick)
					{
						ping_timer[i] += 4;
						if (ping_timer[i] > 600 && !WarnedPing[i] && GlobalOnlineAdminList.GetAdminLevel(i) < 0)
						{
							WarnedPing[i] = true;
							MsgSend(i, FormatWArgs(L"%ls, you will be kicked for high ping (%u ms)", PlayerTable[i].name, PlayerTable[i].ping).c_str());
						}
						else if (ping_timer[i] > 1800 && GlobalOnlineAdminList.GetAdminLevel(i) < 0)
						{
							ping_timer[i] = 0;
							WarnedPing[i] = false;
							wstring wstr = FormatWArgs(L"%ls was kicked for high ping (%u ms)", PlayerTable[i].name, PlayerTable[i].ping);
							sv_say(wstr.c_str());
							WriteLog(L"KICK", move(wstr));
							sv_kick(i);
						}
					}
					else if (ping_timer[i] > 0)
						ping_timer[i]--;
				}
				// end of high ping check
			}

			// anticamp
			if (GlobalPrefs->CampTime)
			{
				camp_timer[i] = (camp_timer[i] + 1) % 30;
				if(camp_timer[i] == (2*i) % 30)
				{
					if(PlayerAliveOnly(i))
					{
						vect3f Coords;
						GetPlayerPosition(i, Coords);
						PlayerCoords[i].emplace_back(Coords);

						if (PlayerCoords[i].size() > GlobalPrefs->CampTime)
						{
							PlayerCoords[i].pop_front();

							if(IsCamping(PlayerCoords[i]))
							{
								if(PlayerTable[i].kills > KillsPrevCamp[i])
								{
									PlayerCampKills[i]++;
									GlobalEventHandler.Execute(i, EVENT_CAMP, vector <pair<wstring, wstring>>());
								}
							}
							else
								PlayerCampKills[i] = 0;
						}
					}
					KillsPrevCamp[i] = PlayerTable[i].kills;
				}
			}
			// end of anticamp

			// spawnkill protection
			if (GlobalPrefs->SpawnProtect)
			{
				bool AliveS = PlayerAliveOnly(i);
				if(PlayerPrevState[i] == 0 && AliveS && SpawnTimer[i] == 0)
				{
					Dynamic_Player *Player = ObjectTable[PlayerTable[i].ObjectIndexNum].Offset;
					Player->IsInvincible |= 8;
					PlayerPrevState[i] = 1;
					SpawnTimer[i] = GlobalPrefs->SpawnProtect;
					JustSpawned[i] = true;
				}
				else if(!AliveS)
				{
					PlayerPrevState[i] = 0;
					SpawnTimer[i] = 0;
					JustSpawned[i] = false;
				}

				if(JustSpawned[i])
				{
					if (SpawnTimer[i] == 0 || *GameTicks < PlayerTable[i].LastBulletTimestamp + 16)
					{
						Dynamic_Player *Player = ObjectTable[PlayerTable[i].ObjectIndexNum].Offset;
						Player->IsInvincible &= ~8;
						Player->Health = 1;
						Player->Shield = 1;
						JustSpawned[i] = false;
						SpawnTimer[i] = 0;
					}
					else
						SpawnTimer[i]--;
				}
			}
			// end of spawnkill protection

			// anti-warp
			if (GlobalPrefs->MaxWarps)
			{
				if(PlayerAliveOnly(i))
				{
					vect3f loc;
					GetPlayerPosition(i, loc);
					float d = Get3DDistance(loc, PlayerPrevPos[i]);
					PlayerPrevPos[i] = loc;

					if(!Teleported[i])
					{
						Dynamic_Player *Player = ObjectTable[PlayerTable[i].ObjectIndexNum].Offset;
						float movLen;
						if (Player->VehicleIndexNum != 0xFFFF)
						{
							char *VehicleAddr = (char*)ObjectTable[Player->VehicleIndexNum].Offset;
							movLen = (*(vect3f*)(VehicleAddr + 0x68)).Length();
						}
						else
						{
							movLen = Player->PlayerVectors.Mov.Length();
						}
				
						if (movLen > 0.05f && d > movLen * 2.5f)
						{
							PlayerWarps[i]++;
							LastWarp[i] = *GameTicks;

							if (PlayerWarps[i] >= GlobalPrefs->MaxWarps)
							{
								vector <pair<wstring, wstring>> EventArgs;
								EventArgs.emplace_back(L"$warps", ToWStr((int)PlayerWarps[i]));
								GlobalEventHandler.Execute(i, EVENT_WARP, EventArgs);
							}
						}
							
						if (PlayerWarps[i] && (*GameTicks - LastWarp[i]) > 150)
						{
							PlayerWarps[i] = 0;	
						}
					}
					else
						Teleported[i] = false;
				}
			}
			// end of anti-warp

			// anti-glitch
			if (GlobalPrefs->AntiGlitch)
			{
				if (PlayerAliveOnly(i))
				{
					Dynamic_Player* Player = ObjectTable[PlayerTable[i].ObjectIndexNum].Offset;
					if (Player->VehicleIndexNum == 0xffff)
					{
						if (Player->SomeFlags & 0x00200000)
						{
							KillPlayer(i);
							sv_say(FormatWArgs(L"%ls was killed for glitching", PlayerTable[i].name).c_str());
						}
					}
					else
					{
						Dynamic_Player* Vehicle = ObjectTable[Player->VehicleIndexNum].Offset;
						if (Vehicle->SomeFlags & 0x00200000)
						{
							KillPlayer(i);
							sv_say(FormatWArgs(L"%ls was killed for glitching with a vehicle", PlayerTable[i].name).c_str());
						}
					}
				}
			}
			// end of anti-glitch
		}
	}

	GlobalAreas.RaiseEvents();

	GlobalEventHandler.Execute(-1, EVENT_TICK, vector <pair<wstring, wstring>>());

	if(Server->ServerState == 2)
	{
		if (GlobalPrefs->SappMapcycle)
		{
			if (!game_break && !MapExecuted)
			{
				GlobalMapcycle.Next();
			}
		}
		else if(GlobalPrefs->MapVote && !MapVoteCanceled)
		{
			if(game_break % 150 == 0 && game_break + 30 < timeout)
			{
				GlobalMapVotes.List();
			}
			else if(game_break + 30 == timeout)
			{
				MapVoteCanceled = true;
				GlobalMapVotes.Launch();
			}
		}

		game_break++;
	}
}

void ResetPlayerVars(size_t i)
{
	RespawnTimeLeft[i] = 0;
	AliveTimer[i] = 0;
	valid_cdkey[i] = false;
	WarnedAFK[i] = false;
	WarnedPing[i] = false;
	afk_help_timer[i] = 10;
	ping_help_timer[i] = 300;
	afk_timer[i] = 0;
	ping_timer[i] = 0;
	ChatSpam[i] = 0;
	Skips[i] = false;

	LastKiller[i] = 0;

	PlayerPrevState[i] = 0;
	SpawnTimer[i] = 0;
	JustSpawned[i] = false;

	PlayerWarps[i] = 0;

	AimbotScores[i] = 0;
	LoginFails[i] = 0;
	RconFails[i] = 0;
	AFK[i] = false;
	stfu[i] = false;

	camp_timer[i] = 0;
	PlayerCampKills[i] = 0;
	PlayerCoords[i].clear();

	PlayerApps[i] = APP_Nothing;
}

void ResetPlayers()
{
	if (ServerRunning())
	{
		for (size_t i = 0; i < 16; i++)
		{
			if (PlayerAlive(i))
			{
				Dynamic_Player *Player = ObjectTable[PlayerTable[i].ObjectIndexNum].Offset;
				Player->IsInvincible &= ~8;
				Player->SuspendUnit = 0;
			}
		}
	}
}

void ClearGlobalVectors()
{
	ClearNoLeadCache();

	DoSList.clear();
	
	JoinReqests.clear();

	IPs.clear();

	for (size_t i = 0; i < 16; i++)
		PlayerCoords[i].clear();

	DisabledObjects.clear();

	ObjectCache.clear();
}