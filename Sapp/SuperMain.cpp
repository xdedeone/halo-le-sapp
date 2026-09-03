// Main

#include <fstream>

//#ifdef _DEBUG
//#include "vld.h"
//#endif

#include "Addresses.h"
#include "Admins.h"
#include "AliasThread.h"
#include "Areas.h"
#include "bans.h"
#include "CommandExecuter.h"
#include "CommandFactory.h"
#include "CustomCommandFactory.h" 
#include "Event.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "Locations.h"
#include "LogThread.h"
#include "LuaHandler.h"
#include "Mapcycle.h"
#include "MapLoader.h"
#include "MapVote.h"
#include "NetworkClient.h"
#include "NetworkThread.h"
#include "Patcher.h"
#include "Patches.h"
#include "Prefs.h"
#include "RemoteConsole.h"
#include "RemoteConsoleNetworking.h"
#include "Replace.h"
#include "TextUtilities.h"
#include "ThreadCycleFix.h"
#include "UserList.h"

using namespace std;

HMODULE hMyDLL;

int __declspec(dllexport) __stdcall CHandler(const char* cmd)
{
	if (CommandExecuting == -2) // A script is trying to execute an unknown command
	{
		if (ScriptEcho && GlobalPrefs->Lua && !GlobalPrefs->ScrimMode)
		{
			vector<unique_ptr<lua::LuaEventArgBase>> args;
			args.emplace_back(make_unique<lua::LuaEventArgString>(FormatArgs("Unknown command: %hs", cmd)));
			lua::LuaEventCallback(EVENT_ECHO, -1, args);
		}
		return 0;
	}
	else if(CommandExecuting > -1) // A player is trying to execute an unknown command
	{	
		MsgSend(CommandExecuting, FormatWArgs(L"Unknown command: %hs", cmd).c_str());
		return 1;
	}
	else if (CommandExecuting == -3) // Unknown command in init.txt or a custom command is trying to execute an unknown command from the console
	{
		PrintConsole(FormatArgs("Unknown command: %hs", cmd).c_str());
		return 1;
	}
	else if (CommandExecuting == -4) // A remote console is trying to execute an unknown command
	{
		GlobalRemoteConsole.SendRemoteOutput(FormatArgs("Unknown command: %hs", cmd).c_str());
		return 0;
	}

	if (GlobalPrefs->Lua && !GlobalPrefs->ScrimMode)
	{
		vector<unique_ptr<lua::LuaEventArgBase>> args;
		args.emplace_back(make_unique<lua::LuaEventArgString>(cmd));
		args.emplace_back(make_unique<lua::LuaEventArgInt>(0));
		if (!lua::LuaEventCallback(EVENT_COMMAND, -1, args))
			return 1;
	}

	wstring command = FormatWArgs(L"%hs", cmd);
	TrimSpaces(command);

	CommandEnviroment env(Enviroment::CONSOLE);
	return ExecuteSingleCommand(command, env);
}

#ifdef HPC
void LoadCustomMaps()
{
	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile("maps\\*.map", &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FindData.nFileSizeLow > sizeof(MapHeader))
			{
				HANDLE file = CreateFile(FormatArgs("maps\\%hs", FindData.cFileName).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (file != INVALID_HANDLE_VALUE)
				{
					HANDLE map = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);
					if (map != NULL)
					{
						MapHeader* view = static_cast<MapHeader*>(MapViewOfFile(map, FILE_MAP_READ, 0, 0, sizeof(MapHeader)));
						if (view != NULL && view->Signature0 == 0x68656164 && view->MapType == 1)
						{
							string name = FindData.cFileName;
							name.erase(name.end() - 4, name.end());
							if (!FindMap(name.c_str()))
							{
								AddMap(name.c_str(), 0);
							}
						}
						CloseHandle(map);
					}
					CloseHandle(file);
				}
			}
		} while (FindNextFile(hFind, &FindData) > 0);
		FindClose(hFind);
	}
}
#endif

void CacheIndexes()
{
	if (ServerRunning())
	{
		for (size_t i = 0; i < 16; i++)
		{
			if (PlayerPresent(i))
			{
				size_t PlayerIndex = Server->PlayerData[i].player_index;
				size_t MachineIndex = Server->PlayerData[i].machine_index;
				PlayerIndexes[MachineIndex] = PlayerIndex;
				MachineIndexes[PlayerIndex] = MachineIndex;
			}
		}
	}	
}

void Init()
{
	wstring init_file = FormatWArgs(L"%hs\\sapp\\init.txt", GametypesFolder);

	fstream InitFile;
	InitFile.open(init_file, ios::out | ios::app);
	InitFile.close(); // create if not exists

	InitFile.open(init_file, ios::in);
	if (!InitFile.is_open())
	{
		PrintConsole(12, "Failed to open SAPP init file!");
		return;
	}

	PrintConsole(15, "-- Loading SAPP settings --");

	size_t i = 0;	
	CommandEnviroment env(Enviroment::INIT);

	string line;
	while(getline(InitFile, line))
	{
		size_t n;
		if((n = line.find_first_not_of(" \t")) != string::npos)
		{
			if(line[n] == ';')
				continue;
		}

		if (line.size() > 254)
			line.resize(254);

		wstring command = FormatWArgs(L"%hs", line.c_str());
		TrimSpaces(command);

		ExecuteSingleCommand(command, env);
		i++;
	}
	InitFile.close();

	if(i == 0)
		PrintConsole("There are no SAPP settings.");

	PrintConsole(15, "-- Loading SAPP settings finished --");
}

void Load()
{
	HaloThreadLock.Lock();

	GlobalPrefs = make_unique<prefs>();

	Patches.NewPatch(new FunctionAddress("ConsolePrint function", PrintCode, 0, &HaloPrintAddress));

	PrintConsole(14, "Loading SAPP...");

	PrintConsole("Starting threads...");

	CreateHaloTimer();

	ThreadPool.StartAll(16);

	PrintConsole("Searching for Halo functions...");

	FindAddresses();

	PrintConsole("Patching Halo's code for codecaves...");

	InstallCCs();
#ifdef HPC
	if (!Initalised)
	{
		PrintConsole("Loading custom maps...");

		LoadCustomMaps();
	}
#endif
	PrintConsole("Loading commands...");

	CacheIndexes();

	CacheCurrentGameVars();

	LoadReplacements();

	GlobalCommandFactory.LoadCommands();

	LoadCustomCommands();

	PrintConsole("Loading Lua scripts...");

	lua::CacheScripts();

	PrintConsole("Loading settings...");

	GlobalEventHandler.LoadEvents();
	GlobalUserList.LoadUsersFile();
	GlobalAdminList.LoadAdmins();
	GlobalIPBanlist.LoadIPBans();
	GlobalLocations.LoadLocations();
	GlobalAreas.LoadAreas();
	GlobalMapVotes.LoadMapVotes();
	GlobalMapcycle.LoadMapcycle();

	Init();

	if (!Initalised)
	{
		WriteLog(L"LOAD", FormatWArgs(L"SAPP was loaded, Version %hs %hs", GlobalPrefs->AppVersion, GlobalPrefs->AppHaloType));
	}

	LoadEventScan();

	if (GlobalPrefs->NetworkThread && StartNetworkThread())
		PrintConsole("Network thread has been started!");

	if (GlobalPrefs->MapVote)
	{
		if (GlobalMapVotes.size() < 2)
		{
			GlobalPrefs->MapVote = false;
			PrintConsole(12, "Error: Less than 2 mapvote option was loaded. Map voting is now disabled!");
		}
		else if (*MapcycleTimeout < 10)
		{
			PrintConsole(12, "Warning! Too short of a mapcycle timeout!");
			PrintConsole(15, "It's recommended to have at least 10 seconds timeout for mapvote.");
		}
	}

	if (GlobalPrefs->SappMapcycle)
	{
		GlobalMapcycle.SetToCurrentMap();
	}
}

void Unload()
{
	HaloThreadLock.Unlock();

	PrintConsole(14, "Unloading SAPP...");

	PrintConsole("Stopping threads...");

	GlobalRemoteConsole.StopRemoteConsole();

	ThreadPool.StopAll();

	DeleteHaloTimer();

	PrintConsole("Unloading Lua scripts...");

	lua::ClearScripts();

	PrintConsole("Cleaning up...");

	ResetPlayers();

	ClearGlobalVectors();

	ClearAliases();

	ClearReplacements();

	GlobalAreas.Clear();
	GlobalLocations.Clear();
	GlobalMapcycle.Clear();
	GlobalMapVotes.Clear();
	GlobalEventHandler.Clear();

	GlobalCommandFactory.ClearCommands();

	PrintConsole("Restoring original code...");

	ClearPatches();
}

void OnUnload()
{
	if (!ThreadPool.IsShuttingDown()) // Workaround for Wine
	{
		WriteLog(L"UNLOAD", L"SAPP was unloaded");

		Unload();
	}
}

BOOL __stdcall DllMain(HINSTANCE hinstDLL, DWORD fwdreason, LPVOID lpvReserved)
{
	switch (fwdreason)
	{
	case DLL_PROCESS_ATTACH:
	{
		hMyDLL = hinstDLL;
		DisableThreadLibraryCalls(hinstDLL);

		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

		Load();

		Initalised = true;

		sv_say(L"SAPP was loaded!");

		PrintConsole(10, FormatArgs("SAPP Version %hs %hs by seh‚øø, originally by Termy || halo.isimaginary.com", GlobalPrefs->AppVersion, GlobalPrefs->AppHaloType).c_str());

		break;
	}
	case DLL_PROCESS_DETACH:
	{
		sv_say(L"SAPP was unloaded!");

		PrintConsole(10, "SAPP was unloaded, Bye =)");

		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

		break;
	}
	}
	return 1;
}