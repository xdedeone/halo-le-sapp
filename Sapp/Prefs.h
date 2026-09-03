#pragma once

#include <memory>

#include "Locker.h"
#include "SuperHeader.h"

extern const DWORD CurrentVersion;
extern const DWORD LuaVersion;

extern bool Initalised;
extern bool AntiCheat;

extern DWORD ServerID;

extern ServerStats MyServerStats;

extern HMODULE hModule;
extern HANDLE hConsole;

extern Locker HaloThreadLock;

struct prefs
{
#ifdef HPC
	const char* const AppHaloType = "PC";
#else
	const char* const AppHaloType = "CE";
#endif
	const char* const AppVersion = "10.2.1";
	const char* const LuaAPIVersion = "1.12.0.0";
	const wchar_t* const Me = L"seh鰰";
	const wchar_t* const Ye = L"yEyE";
	std::wstring msg_prefix = L"** SAPP ** ";
	std::wstring admin_prefix = L"** ADMIN ** ";
	wchar_t CmdStart1 = '\\';
	wchar_t CmdStart2 = '/';
	bool Lua = false;
	bool AntiCheat = false;
	byte AntiSpam = 0;
	bool AntiCaps = false;
	bool AntiHaloFP = false;
	bool AntiGlitch = false;
	bool AntiLagSpawn = false;
	bool SappRcon = false;
	bool SappConsole = false;
	bool SappMapcycle = false;
	bool MapVote = false;
	bool SPre = false;
	bool BlockTC = false;
	bool HideAdmin = false;
	bool SaveScores = false;
	bool SaveRespawnTime = false;
	bool ScrimMode = false;
	bool DisableBacktap = false;
	bool Log = false;
	bool ConChatEcho = false;
	bool FullIPBan = false;
	bool DebugStrings = false;
	bool Alias = false;
	bool OnlyValidAlias = false;
	bool NoLead = false;
	bool MTV = false;
	byte Zombies = 0;
	bool DisableObjectsPlayer[16] = {};
	bool DisableVehiclesPlayer[16] = {};
	bool DisableObjectsRed = false;
	bool DisableObjectsBlue = false;
	bool DisableVehiclesRed = false;
	bool DisableVehiclesBlue = false;
	bool DisableTimerOffsets = false;
	bool ObjectSyncCleanup = false;
	bool RemoteConsole = false;
#ifdef HALORANK
	bool HaloRank = false;
#endif
	bool ConsoleInput = true;
	bool AutoUpdate = true;
	bool NetworkThread = true;
	size_t AFKKick = 0;
	size_t PingKick = 0;
	size_t MapSkip = 0;
	size_t OriginalMapSkip = 0;
	size_t MaxWarps = 0;
	size_t SpawnProtect = 0;
	size_t CampTime = 0;
	float CampDist = 0;
	size_t AimbotScanScoreLimit = 0;
	size_t AimbotScanBanType = 2;
	size_t AimbotScanBanLength = 0;
	size_t PacketLimit = 1000;
	size_t LogRotation = 4194304;
	size_t MaxVotes = 5;
	size_t AdminBan = 0;
	size_t AdminAddSameLevel = 0;
	size_t AdminDelSameLevel = 0;
	size_t HillTimer = 1800;
	size_t MaxIdle = 1800;
	WORD ConAttrib = 7;
	size_t CustomSleep = 33;
	size_t OriginalCustomSleep = 8;
	int SJLevel = -1;
	DWORD Flags = 0;
};

extern std::unique_ptr<prefs> GlobalPrefs;

namespace stats
{
	extern unsigned long JoinAttempts;
	extern unsigned long ServerQueries;
	extern unsigned long Commands;
	extern unsigned long NumberOfExecutedEvents;
	extern unsigned long Chat;
	extern unsigned long Games;
	extern unsigned long Captures;
	extern unsigned long Kills;
	extern unsigned long Betrays;
	extern unsigned long Suicides;
	extern time_t load_time;
#ifdef HALORANK
	extern time_t last_game;
#endif
};