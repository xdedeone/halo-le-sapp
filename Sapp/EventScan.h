#pragma once

#include <deque>
#include <vector>

#include "SuperHeader.h"

enum class GameType { CTF, Slayer, Oddball, King, Race, Unknown };

extern GameType CurrentGameType;

extern std::string CurrentMap;
extern std::string CurrentMode;
extern std::string CurrentGT;
extern std::wstring CurrentMapW;
extern std::wstring CurrentModeW;
extern std::wstring CurrentGTW;

extern std::deque <vect3f> PlayerCoords[16];

extern std::vector <LocalAntcheatPlayer> AnticheatPlayers;

extern int timeout;
extern int game_break;

extern DWORD RespawnTimeLeft[16];

extern bool AFK[16];

extern ULONGLONG PlayerID[16];

extern size_t AliveTimer[16];

extern vect3f prev_camera[16];

extern size_t ValidationTimer[16];

extern size_t LastKiller[16];

extern size_t PlayerCampKills[16];

extern bool Teleported[16];

extern size_t afk_timer[16];
extern size_t afk_help_timer[16];

extern DWORD PlayerWarps[16];
extern vect3f PlayerPrevPos[16];

extern size_t LastColor[16][3];

extern uint32_t PlayerApps[16];

void CacheCurrentGameVars();

void LoadEventScan();

void EventScan();

void ResetPlayerVars(size_t i);

void ResetPlayers();

void ClearGlobalVectors();