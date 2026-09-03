#include "Prefs.h"

const DWORD CurrentVersion = 0x0A020100;
const DWORD LuaVersion = 0x010C0000;

bool Initalised = false;
bool AntiCheat = false;

DWORD ServerID = 0;

ServerStats MyServerStats = {};

HMODULE hModule = GetModuleHandleW(0);
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

Locker HaloThreadLock;

std::unique_ptr<prefs> GlobalPrefs;

namespace stats
{
	unsigned long JoinAttempts = 0;
	unsigned long ServerQueries = 0;
	unsigned long Commands = 0;
	unsigned long NumberOfExecutedEvents = 0;
	unsigned long Chat = 0;
	unsigned long Games = 0;
	unsigned long Captures = 0;
	unsigned long Kills = 0;
	unsigned long Betrays = 0;
	unsigned long Suicides = 0;
	time_t load_time = time(0);
#ifdef HALORANK
	time_t last_game;
#endif
};