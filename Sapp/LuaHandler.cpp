#include "LuaHandler.h"
#include "APIFunctions.h"

#include "Addresses.h"
#include "HaloUtilities.h"
#include "Prefs.h"
#include "TextUtilities.h"
#include "Threads.h"

#pragma comment(lib, "lua51.lib")

using namespace std;

namespace lua
{

DWORD Lua_Script::ID = 1;

vector <Lua_Script> Scripts;
map <Lua_Event, map <lua_State*, string>> Callbacks;
map <lua_State*, vector<weak_ptr<work>>> ScriptThreads;
Locker ScriptThreadLock;

void Lua_Threads(shared_ptr<work> &hThread)
{
	Lua_Thread* myArgs = static_cast<Lua_Thread*>(hThread->GetParams());
	lua_State* state = myArgs->state;

	bool ret = true;

	if (GlobalPrefs->Lua && !GlobalPrefs->ScrimMode)
	{
		HaloThreadLock.Lock();
		if (!hThread->ExitFlag)
		{
			lua_getglobal(state, myArgs->name.c_str());
			for (size_t i = 0; i < myArgs->args.size(); i++)
			{
				lua_pushstring(state, myArgs->args[i].c_str());
			}
			if (!lua_pcall(state, myArgs->args.size(), 1, 1))
			{
				if (lua_type(state, -1) != LUA_TBOOLEAN || lua_toboolean(state, -1) != 1)
				{
					ret = false;
				}
				lua_pop(state, 1);
			}
			else
			{
				ret = false;
				PrintConsole(12, FormatArgs("LuaTimer >> %hs >> Lua Error: %hs", myArgs->name.c_str(), lua_tostring(state, -1)).c_str());
				lua_pop(state, 1);	
			}

			if (ret)
			{
				ThreadPool.UpdateTimer(myArgs, LuaTimerDue);
			}
		}
		HaloThreadLock.Unlock();
	}

	ScriptThreadLock.Lock();
	for (size_t i = 0; i < ScriptThreads[state].size(); i++)
	{
		if (auto sp = ScriptThreads[state][i].lock())
		{
			if (sp == hThread)
			{
				ScriptThreads[state].erase(ScriptThreads[state].begin() + i);
				if (!ret)
				{
					ThreadPool.DeleteTimer(myArgs);
				}
				break;
			}
		}
	}
	ScriptThreadLock.Unlock();
}

void __stdcall LuaTimerDue(PVOID params, BOOLEAN)
{
	Lock_Object guard(ScriptThreadLock);
	Lua_Thread* newThread = static_cast<Lua_Thread*>(params);
	ScriptThreads[newThread->state].emplace_back(ThreadPool.NewWork(Lua_Threads, params));
}

#define LUA_ENUM(name) \
	lua_pushlstring(state, #name, sizeof(#name)-1); \
	lua_pushnumber(state, name); \
	lua_settable(state, -3);

void CreateEventTable(lua_State* state)
{
	lua_newtable(state);

	LUA_ENUM(EVENT_CUSTOM)
	LUA_ENUM(EVENT_TICK)
	LUA_ENUM(EVENT_ALIVE)
	LUA_ENUM(EVENT_AREA_ENTER)
	LUA_ENUM(EVENT_AREA_EXIT)
	LUA_ENUM(EVENT_PRESPAWN)
	LUA_ENUM(EVENT_SPAWN)
	LUA_ENUM(EVENT_KILL)
	LUA_ENUM(EVENT_DIE)
	LUA_ENUM(EVENT_ASSIST)
	LUA_ENUM(EVENT_SNAP)
	LUA_ENUM(EVENT_WARP)
	LUA_ENUM(EVENT_WEAPON_PICKUP)
	LUA_ENUM(EVENT_WEAPON_DROP)
	LUA_ENUM(EVENT_VEHICLE_ENTER)
	LUA_ENUM(EVENT_VEHICLE_EXIT)
	LUA_ENUM(EVENT_BETRAY)
	LUA_ENUM(EVENT_SUICIDE)
	LUA_ENUM(EVENT_SCORE)
	LUA_ENUM(EVENT_TEAM_SWITCH)
	LUA_ENUM(EVENT_PREJOIN)
	LUA_ENUM(EVENT_JOIN)
	LUA_ENUM(EVENT_LEAVE)
	LUA_ENUM(EVENT_CAMP)
	LUA_ENUM(EVENT_LOGIN)
	LUA_ENUM(EVENT_GAME_START)
	LUA_ENUM(EVENT_GAME_END)
	LUA_ENUM(EVENT_MAP_RESET)
	LUA_ENUM(EVENT_CHAT)
	LUA_ENUM(EVENT_COMMAND)
	LUA_ENUM(EVENT_ECHO)
	LUA_ENUM(EVENT_OBJECT_SPAWN)
	LUA_ENUM(EVENT_DAMAGE_APPLICATION)
	LUA_ENUM(EVENT_STICK)

	lua_setglobal(state, "cb");
}

void CreateGlobals(lua_State* state)
{
	CreateEventTable(state);

	lua_pushstring(state, GlobalPrefs->AppHaloType);
	lua_setglobal(state, "halo_type");

	lua_pushnumber(state, GetCurrentProcessId());
	lua_setglobal(state, "pid");

	lua_pushstring(state, GlobalPrefs->LuaAPIVersion);
	lua_setglobal(state, "lua_api_version");

	lua_pushstring(state, GlobalPrefs->AppVersion);
	lua_setglobal(state, "sapp_version");
}

void CreateEnviroment(lua_State* state)
{
	luaL_openlibs(state);

	RegisterFunctions(state);

	CreateGlobals(state);
}

int OnError(lua_State *state)
{
	lua_getglobal(state, "OnError");
	if (lua_type(state, -1) != LUA_TNIL)
	{
		lua_pushstring(state, lua_tostring(state, 1));
		if (lua_pcall(state, 1, 0, 0))
		{
			PrintConsole(12, FormatArgs("OnError >> Lua Error: %hs", lua_tostring(state, -1)).c_str());
			lua_pop(state, 1);
		}
	}
	else
	{
		lua_pop(state, 1);
	}

	return 1;
}

void CacheScripts()
{
	wstring lua_mask = FormatWArgs(L"%hs\\sapp\\lua\\*.lua", GametypesFolder);

	WIN32_FIND_DATAW FindData;
	HANDLE hFind = FindFirstFileW(lua_mask.c_str(), &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			HANDLE hFile = CreateFileW(FormatWArgs(L"%hs\\sapp\\lua\\%ls", GametypesFolder, FindData.cFileName).c_str(), GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);	
			if (hFile != INVALID_HANDLE_VALUE)
			{
				DWORD file_lenght = GetFileSize(hFile, NULL);
				if (file_lenght != INVALID_FILE_SIZE)
				{
					unique_ptr<char[]> filebuff(new char[file_lenght]);

					DWORD readed = 0;
					ReadFile(hFile, filebuff.get(), file_lenght, &readed, NULL);
					CloseHandle(hFile);

					wstring name = FindData.cFileName;
					name.erase(name.end() - 4, name.end());
						
					lua_State* state = luaL_newstate();
					CreateEnviroment(state);
					lua_pushcfunction(state, OnError);
					if (!luaL_loadbuffer(state, filebuff.get(), file_lenght, WToStr(name).c_str()))
					{
						Scripts.emplace_back(move(name), state);
						continue;
					}
					else
					{
						PrintConsole(12, FormatArgs("Lua Syntax Error: %hs", lua_tostring(state, -1)).c_str());
						lua_close(state);
					}
				}
				else
				{
					CloseHandle(hFile);
				}
			}
			PrintConsole(12, FormatArgs("Failed to read %ls!", FindData.cFileName).c_str());			
		} while (FindNextFileW(hFind, &FindData));
		FindClose(hFind);
	}
}

bool InitializeScript(lua_State* state, const wstring &name)
{
	if (!lua_pcall(state, 0, 0, 1))
	{
		bool incompatible = false;
		lua_getglobal(state, "api_version");
		if (lua_isstring(state, -1))
		{
			string version_str = lua_tostring(state, -1);
			vector <string> VersionNumbers;
			if (Split(VersionNumbers, version_str, '.') && VersionNumbers.size() > 1)
			{
				size_t MayorVersion, MinorVersion;
				if (ToNumber(VersionNumbers[0], MayorVersion) && ToNumber(VersionNumbers[1], MinorVersion))
				{
					byte* version = (byte*)&LuaVersion;
					if (MayorVersion == (size_t)version[3])
					{
						if (MinorVersion > (size_t)version[2])
						{
							PrintConsole(14, FormatArgs("The script %ls.lua might not work correctly with this Sapp version!", name.c_str()).c_str());
						}

						lua_pop(state, 1);
						return true;
					}
					else
					{
						incompatible = true;
					}
				}
			}
		}
		if (!incompatible)
			PrintConsole(12, FormatArgs("Version string is invalid or not found in %ls.lua", name.c_str()).c_str());
		else
			PrintConsole(12, FormatArgs("The script %ls.lua is incompatible with this Sapp version!", name.c_str()).c_str());
	}
	else
	{
		PrintConsole(12, FormatArgs("LoadScript >> Lua Error: %hs", lua_tostring(state, -1)).c_str());
	}

	return false;
}

void ClearScripts()
{
	for (auto &it : Scripts)
	{
		lua_State* state = it.state;

		if (it.loaded)
		{
			lua_getglobal(state, "OnScriptUnload");
			if (lua_pcall(state, 0, 0, 1))
			{
				PrintConsole(12, FormatArgs("OnScriptUnload >> Lua Error: %hs", lua_tostring(state, -1)).c_str());
				lua_pop(state, 1);
			}
		}

		lua_close(state);
	}

	ScriptThreads.clear();

	Callbacks.clear();
	
	Scripts.clear();
}

bool LoadScript(const wstring &name)
{
	auto it = find_if(Scripts.begin(), Scripts.end(), [&name](const Lua_Script &it)
	{
		return !it.loaded && !_wcsicmp(it.name.c_str(), name.c_str());
	});

	if (it == Scripts.end())
		return false;

	lua_State* state = it->state;

	if (!it->initialized)
	{
		if (!InitializeScript(state, name))
		{
			lua_close(state);
			Scripts.erase(it);
			return false;
		}
		it->initialized = true;
	}

	it->loaded = true;

	lua_getglobal(state, "OnScriptLoad");
	if (lua_pcall(state, 0, 0, 1))
	{
		PrintConsole(12, FormatArgs("OnScriptLoad >> Lua Error: %hs", lua_tostring(state, -1)).c_str());
		lua_pop(state, 1);
	}

	return true;
}

bool UnloadScript(const wstring &name)
{
	auto it = find_if(Scripts.begin(), Scripts.end(), [&name](const Lua_Script &it)
	{
		return it.loaded && !_wcsicmp(it.name.c_str(), name.c_str());
	});

	if (it == Scripts.end())
		return false;

	lua_State* state = it->state;

	lua_getglobal(state, "OnScriptUnload");
	if (lua_pcall(state, 0, 0, 1))
	{
		PrintConsole(12, FormatArgs("OnScriptUnload >> Lua Error: %hs", lua_tostring(state, -1)).c_str());
		lua_pop(state, 1);
	}

	ScriptThreadLock.Lock();

	ThreadPool.DeleteTimerGroup(it->ScriptID);

	for (auto &jt : ScriptThreads[state])
	{
		if (auto sp = jt.lock())
		{
			sp->ExitFlag = true;
		}
	}
	ScriptThreads.erase(state);

	ScriptThreadLock.Unlock();

	for (auto &jt : Callbacks)
	{
		jt.second.erase(state);
	}

	it->loaded = false;

	return true;
}

wostringstream StreamOut()
{
	wostringstream os;
	if (Scripts.empty())
	{
		os << L"There are no Lua scripts." << endl;
	}
	else
	{
		os << left << setw(16) << L"Name" << L"Status" << endl;
		for (auto &it : Scripts)
		{
			const wchar_t* const initalised = it.initialized ? L"Initalized" : L"Uninitalized";
			const wchar_t* const loaded = it.loaded ? L"Loaded" : L"Unloaded";
			os << left << setw(16) << it.name << setw(12) << loaded << initalised << endl;
		}
	}
	return os;
}

void EventCallback(Sapp_Event event, size_t PlayerIndex, const vector <pair<wstring, wstring>> &SpecialArguments)
{
	int PlayerNumber = IndexToNumber(PlayerIndex) + 1;
	for (auto &it : Callbacks[static_cast<Lua_Event>(event)])
	{
		lua_State* state = it.first;
		int argc = 1;

		int base = lua_gettop(state) + 1;
		lua_pushcfunction(state, OnError);
		lua_getglobal(state, it.second.c_str());
		lua_pushinteger(state, PlayerNumber);
		if (!SpecialArguments.empty())
		{
			for (auto &jt : SpecialArguments)
			{
				lua_pushstring(state, WToStr(jt.second).c_str());
				argc++;
			}
		}
		if (!lua_pcall(state, argc, 0, base))
		{
			lua_pop(state, 1);
		}
		else
		{
			PrintConsole(12, FormatArgs("EventCallback >> %hs >> Lua Error: %hs", it.second.c_str(), lua_tostring(state, -1)).c_str());
			lua_pop(state, 2);	
		}
	}	
}

bool LuaEventCallback(Lua_Event event, size_t PlayerIndex, const vector<unique_ptr<LuaEventArgBase>> &Arguments, double* OptRet)
{
	bool ret = true;
	int PlayerNumber = IndexToNumber(PlayerIndex) + 1;
	for (auto &it : Callbacks[event])
	{
		lua_State* state = it.first;

		int base = lua_gettop(state) + 1;
		lua_pushcfunction(state, OnError);		
		lua_getglobal(state, it.second.c_str());
		lua_pushinteger(state, PlayerNumber);
		for (size_t i = 0; i < Arguments.size(); i++)
			Arguments[i]->PushArg(state);
		if (!lua_pcall(state, Arguments.size() + 1, 2, base))
		{
			if (lua_type(state, -2) == LUA_TBOOLEAN && lua_toboolean(state, -2) == 0)
			{
				ret = false;
			}
			if (OptRet != nullptr && lua_type(state, -1) == LUA_TNUMBER)
			{
				*OptRet = lua_tonumber(state, -1);
			}
			lua_pop(state, 3);
		}
		else
		{
			PrintConsole(12, FormatArgs("LuaEventCallback >> %hs >> Lua Error: %hs", it.second.c_str(), lua_tostring(state, -1)).c_str());
			lua_pop(state, 2);
		}
	}
	return ret;
}

bool LuaCallCustomFunc(ArgContainer &args)
{
	auto it = find_if(Scripts.begin(), Scripts.end(), [&args](const Lua_Script &it)
	{
		return it.loaded && !_wcsicmp(it.name.c_str(), args[1].c_str());
	});

	if (it == Scripts.end())
		return false;

	lua_State* state = it->state;

	string func = WToStr(args[2]);

	int base = lua_gettop(state) + 1;
	lua_pushcfunction(state, OnError);
	lua_getglobal(state, func.c_str());
	for (size_t i = 3; i < args.argc; i++)
	{
		lua_pushstring(state, WToStr(args[i]).c_str());
	}
	if (!lua_pcall(state, args.argc - 3, 0, base))
	{
		lua_pop(state, 1);
		return true;
	}
	else
	{
		PrintConsole(12, FormatArgs("CallFunction >> %hs >> Lua Error: %hs", func.c_str(), lua_tostring(state, -1)).c_str());
		lua_pop(state, 2);
		return false;
	}	
}

}