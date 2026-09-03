#pragma once

#include <algorithm>
#include <iomanip>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "lua.hpp"

#include "ArgContainer.h"
#include "Events.h"
#include "StringCompare.h"
#include "SuperHeader.h"

enum Lua_Event { EVENT_CHAT = SAPP_EVENT_COUNT, EVENT_COMMAND, EVENT_ECHO, EVENT_OBJECT_SPAWN, EVENT_DAMAGE_APPLICATION, EVENT_STICK, LUA_EVENT_COUNT };

namespace lua
{

class LuaEventArgBase
{
public:
	virtual void PushArg(lua_State* state) = 0;
	virtual ~LuaEventArgBase() {};
};

class LuaEventArgInt final : public LuaEventArgBase
{
	int arg;
public:
	LuaEventArgInt(int i) : arg(i) {}
	void PushArg(lua_State* state)
	{
		lua_pushinteger(state, arg);
	}
};

class LuaEventArgDouble final : public LuaEventArgBase
{
	double arg;
public:
	LuaEventArgDouble(double d) : arg(d) {}
	void PushArg(lua_State* state)
	{
		lua_pushnumber(state, arg);
	}
};

class LuaEventArgString final : public LuaEventArgBase
{
	std::string arg;
public:
	LuaEventArgString(std::string &&s) : arg(move(s)) {}
	void PushArg(lua_State* state)
	{
		lua_pushstring(state, arg.c_str());
	}
};

struct Lua_Script
{
	static DWORD ID;
	DWORD ScriptID;

	std::wstring name;
	lua_State* state;
	
	bool initialized = false;
	bool loaded = false;
	bool safe_read = false;
	bool safe_write = false;

	Lua_Script(std::wstring &&name, lua_State* state) : ScriptID(ID++), name(move(name)), state(state) {}
};

struct Lua_Thread final : public TimerStructBase
{
	lua_State* state;
	std::string name;
	std::vector<std::string> args;

	Lua_Thread(DWORD GroupID, DWORD DueTime, lua_State* state, std::string &&name, std::vector<std::string> &&args) : TimerStructBase(GroupID, DueTime), state(state), name(move(name)), args(move(args)) {}
};

extern std::vector <Lua_Script> Scripts;
extern std::map <Lua_Event, std::map <lua_State*, std::string>> Callbacks;

void __stdcall LuaTimerDue(PVOID params, BOOLEAN);

void CacheScripts();
void ClearScripts();

bool LoadScript(const std::wstring &name);
bool UnloadScript(const std::wstring &name);

std::wostringstream StreamOut();

void EventCallback(Sapp_Event event, size_t PlayerIndex, const std::vector <std::pair<std::wstring, std::wstring>> &SpecialArguments);
bool LuaEventCallback(Lua_Event event, size_t PlayerIndex, const std::vector<std::unique_ptr<LuaEventArgBase>> &Arguments, double* OptRet = nullptr);

bool LuaCallCustomFunc(ArgContainer &args);

}