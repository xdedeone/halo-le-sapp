#pragma once

//#define HALORANK

#ifdef HALORANK

//#include <Windows.h>

#define HR_EVENT_START 0
#define HR_EVENT_END 1
#define HR_EVENT_GAME_START 2
#define HR_EVENT_GAME_END 3
#define HR_EVENT_CDKEY_VALIDATION 4
#define HR_EVENT_PLAYER_JOIN 5
#define HR_EVENT_PLAYER_LEAVE 6
#define HR_EVENT_TEAM_SWITCH 7
#define HR_EVENT_KILL 8
#define HR_EVENT_ASSIST 9

#pragma pack(push, 1)

struct HR_Event // Event Header
{
	DWORD Type; // HR_EVENT_
	DWORD IP; // Player's IP, 0 == Server Event
	DWORD Time; // Relative to game start
};

struct HREventStart : public HR_Event
{
	char ServerName[64];
	char ServerKey[32];
};

struct HREventEnd : public HR_Event
{
	DWORD Reserved; // O.o
};

struct HREventGameStart : public HR_Event
{
	char MapName[32];
	char GTName[32];
	char GameType[31];
	bool TeamGame;
};

struct HREventGameEnd : public HR_Event
{
	DWORD Reserved; // mdr
};

struct HREventCDKeyValidation : public HR_Event
{
	DWORD Value; // 1 == Valid, 2 == Invalid, 3 == Authentication Failed (steal attend), 0 == Unknown (prolly disabled "public key") or pending
};

struct HREventJoin : public HR_Event
{
	wchar_t Name[12];
	char Hash[32];
	char IPstr[32];
	DWORD Team;
};

struct HREventLeave : public HR_Event
{
	DWORD Reserved; // lol
};

struct HREventTeamSwitch : public HR_Event
{
	DWORD NewTeam;
};

struct HREventKill : public HR_Event
{
	//DWORD Killer; // -1 == afterkill with melee ('guardians'), 'suicide-betray', vehicle, AI, 0 == died, 1-16 == Killer's PlayerIndex
	DWORD Killer; // 0 == died, otherwise Killer's IP
	WORD Multi; // double, triple, etc,
	WORD Spree; // killing spree, running riot, etc.
};

struct HREventAssist : public HR_Event
{
	DWORD Reserved; // :3
};

#pragma pack(pop)

#endif