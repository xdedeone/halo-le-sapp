#include "APIFunctions.h"
#include "LuaHandler.h"

#include "codefinder.h"
#include "CommandEnviroment.h"
#include "CommandExecuter.h"
#include "HaloUtilities.h"
#include "Locations.h"
#include "MiscellaneousFunctions.h"
#include "NetworkEvents.h"
#include "ObjectManagement.h"
#include "PlayerJoin-Leave.h"
#include "Prefs.h"
#include "Replace.h"
#include "Threads.h"

#pragma warning(disable: 4800) // 'int': forcing value to bool 'true' or 'false' (performance warning)

namespace lua
{

int FunctionTop(lua_State* state, int min_argc)
{
	int argc = lua_gettop(state);
	if (argc < min_argc)
		luaL_error(state, "too few arguments");

	return argc;
}

void IsBoolean(lua_State* state, int arg)
{
	if (!lua_isboolean(state, arg))
		luaL_typerror(state, arg, "boolean");
}

void IsNumber(lua_State* state, int arg)
{
	if (!lua_isnumber(state, arg))
		luaL_typerror(state, arg, "number");
}

void IsString(lua_State* state, int arg)
{
	if (!lua_isstring(state, arg))
		luaL_typerror(state, arg, "string");
}

Lua_Script& GetScript(lua_State* state)
{
	auto it = find_if(Scripts.begin(), Scripts.end(), [&state](const Lua_Script &it)
	{
		return it.state == state && it.loaded;
	});

	if (it == Scripts.end())
		luaL_error(state, "attempt to call a function from an unloaded script");

	return *it;
}

int timer(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	int argc = FunctionTop(state, 2);

	IsNumber(state, 1);
	IsString(state, 2);
	for (int i = 3; i < argc + 1; i++)
	{
		IsString(state, i);
	}

	std::vector<std::string> args;
	for (int i = 3; i < argc + 1; i++)
	{
		args.emplace_back(lua_tostring(state, i));
	}

	auto newThread = std::make_unique<Lua_Thread>(script.ScriptID, lua_tointeger(state, 1), state, lua_tostring(state, 2), move(args));
	ThreadPool.AddTimer(move(newThread), LuaTimerDue);

	return 0;
}

int register_callback(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsString(state, 2);

	Lua_Event event = static_cast<Lua_Event>(lua_tointeger(state, 1));

	if (event >= EVENT_CUSTOM && event < LUA_EVENT_COUNT)
		Callbacks[event][state] = lua_tostring(state, 2);
	else
		luaL_error(state, "Invalid callback event ID!");

	return 0;
}

int unregister_callback(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	Lua_Event event = static_cast<Lua_Event>(lua_tointeger(state, 1));

	if (event >= EVENT_CUSTOM && event < LUA_EVENT_COUNT)
		Callbacks[event].erase(state);
	else
		luaL_error(state, "Invalid callback event ID!");

	return 0;
}

int execute_command(lua_State* state)
{
	GetScript(state);

	int argc = FunctionTop(state, 1);
	IsString(state, 1);

	size_t PlayerIndex;
	if (argc > 1)
	{
		IsNumber(state, 2);
		PlayerIndex = NumberToIndex((size_t)lua_tonumber(state, 2) - 1);
		if (argc > 2)
		{
			IsBoolean(state, 3);
			ScriptEcho = (bool)lua_toboolean(state, 3);
		}
	}
	else
	{
		PlayerIndex = -1;
	}

	CommandEnviroment env(Enviroment::SCRIPT, PlayerIndex);
	lua_pushboolean(state, ExecuteSingleCommand(FormatWArgs(L"%hs", lua_tostring(state, 1)), env));

	ScriptEcho = false;

	return 1;
}

int execute_command_sequence(lua_State* state)
{
	GetScript(state);

	int argc = FunctionTop(state, 1);
	IsString(state, 1);

	size_t PlayerIndex;
	if (argc > 1)
	{
		IsNumber(state, 2);
		PlayerIndex = NumberToIndex((size_t)lua_tonumber(state, 2) - 1);
		if (argc > 2)
		{
			IsBoolean(state, 3);
			ScriptEcho = (bool)lua_toboolean(state, 3);
		}
	}
	else
	{
		PlayerIndex = -1;
	}

	CommandEnviroment env(Enviroment::SCRIPT, PlayerIndex);
	ExecuteSequence(FormatWArgs(L"%hs", lua_tostring(state, 1)), env);

	ScriptEcho = false;

	return 0;
}

int say(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsString(state, 2);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	if (PlayerPresent(PlayerIndex) && !stfu[PlayerIndex])
	{
		MsgSend(PlayerIndex, lua_tostring(state, 2));
	}

	return 0;
}

int say_all(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 1);
	IsString(state, 1);

	sv_say(lua_tostring(state, 1));

	return 0;
}

int rprint(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsString(state, 2);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	if (PlayerPresent(PlayerIndex) && !stfu[PlayerIndex])
	{
		PrintRconAsm(MachineIndex, lua_tostring(state, 2));
	}

	return 0;
}

int cprint(lua_State* state)
{
	GetScript(state);

	int argc = FunctionTop(state, 1);
	IsString(state, 1);

	WORD attrib;
	if (argc > 1)
	{
		IsNumber(state, 2);
		attrib = (WORD)lua_tonumber(state, 2);
	}
	else
	{
		attrib = GlobalPrefs->ConAttrib;
	}

	PrintConsole(attrib, lua_tostring(state, 1));

	return 0;
}

int rand(lua_State* state)
{
	GetScript(state);

	int argc = FunctionTop(state, 0);

	DWORD min, max;
	if (argc == 0)
	{
		min = 0;
		max = 0x80000000;
	}
	else if (argc == 1)
	{
		IsNumber(state, 1);
		min = 0;
		max = lua_tointeger(state, 1);
	}
	else
	{
		IsNumber(state, 1);
		IsNumber(state, 2);
		min = lua_tointeger(state, 1);
		max = lua_tointeger(state, 2);
	}

	if (min != max)
	{
		lua_pushinteger(state, GetRand(min, max));
	}
	else
	{
		lua_pushinteger(state, min);
	}

	return 1;
}

int add_var(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 2);	
	IsString(state, 1);
	IsNumber(state, 2);

	std::wstring name = FormatWArgs(L"$%hs", lua_tostring(state, 1));
	CustomVariableFlags type = static_cast<CustomVariableFlags>((size_t)lua_tonumber(state, 2));

	lua_pushboolean(state, type < 6 ? AddCustomVar(name, type) : false);
	
	return 1;
}

int del_var(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 1);
	IsString(state, 1);

	std::wstring name = FormatWArgs(L"$%hs", lua_tostring(state, 1));

	lua_pushboolean(state, DelCustomVar(name));

	return 1;
}

int get_var(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsString(state, 2);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	std::string toreplace = lua_tostring(state, 2);

	StrReplaceOne(PlayerIndex, toreplace, toreplace);

	lua_pushstring(state, toreplace.c_str());

	return 1;
}

int set_var(lua_State* state)
{
	GetScript(state);

	int argc = FunctionTop(state, 3);
	IsNumber(state, 1);
	IsString(state, 2);
	IsString(state, 3);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	std::wstring name = FormatWArgs(L"%hs", lua_tostring(state, 2));
	std::wstring value = FormatWArgs(L"%hs", lua_tostring(state, 3));

	if (argc > 3)
	{
		IsNumber(state, 4);

		int OtherMachineIndex = lua_tointeger(state, 4) - 1;
		int OtherPlayerIndex = NumberToIndex(OtherMachineIndex);

		WStrReplaceOne(OtherPlayerIndex, value, value);
	}

	lua_pushboolean(state, SetCustomVar(name, value, PlayerIndex));

	return 1;
}

int to_real_index(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	lua_pushinteger(state, PlayerIndex);

	return 1;
}

int to_player_index(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	int PlayerIndex = lua_tointeger(state, 1);
	int MachineIndex = IndexToNumber(PlayerIndex) + 1;

	lua_pushinteger(state, MachineIndex);

	return 1;
}

int player_present(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	lua_pushboolean(state, PlayerPresent(PlayerIndex));

	return 1;
}

int player_alive(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	lua_pushboolean(state, PlayerAlive(PlayerIndex));

	return 1;
}

int get_player(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	if (PlayerPresent(PlayerIndex))
	{
		lua_pushnumber(state, (DWORD)(&PlayerTable[PlayerIndex]));
	}
	else
	{
		lua_pushnumber(state, 0);
	}

	return 1;
}

int get_dynamic_player(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	if (PlayerAlive(PlayerIndex))
	{
		lua_pushnumber(state, reinterpret_cast<DWORD>(ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset));
	}
	else
	{
		lua_pushnumber(state, 0);
	}

	return 1;
}

int get_object_memory(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	DWORD ObjectIndex = (DWORD)lua_tonumber(state, 1);

	lua_pushnumber(state, reinterpret_cast<DWORD>(GetObjectAddress(ObjectIndex)));

	return 1;
}

int lookup_tag(lua_State* state)
{
	GetScript(state);

	int argc = FunctionTop(state, 1);

	if (argc == 1)
	{
		IsNumber(state, 1);

		DWORD MapID = (DWORD)lua_tonumber(state, 1);
		lua_pushnumber(state, reinterpret_cast<DWORD>(LookupTag(MapID)));
	}
	else
	{
		IsString(state, 1);
		IsString(state, 2);

		const char* type = lua_tostring(state, 1);
		const char* name = lua_tostring(state, 2);
		lua_pushnumber(state, reinterpret_cast<DWORD>(LookupTag(type, name)));
	}

	return 1;
}

int spawn_object(lua_State* state)
{
	GetScript(state);

	int argc = FunctionTop(state, 2);
	IsString(state, 1);
	IsString(state, 2);

	const char* type = lua_tostring(state, 1);
	const char* name = lua_tostring(state, 2);

	if (argc > 2)
	{
		IsNumber(state, 3);
		IsNumber(state, 4);
		IsNumber(state, 5);

		vect3f loc((float)lua_tonumber(state, 3), (float)lua_tonumber(state, 4), (float)lua_tonumber(state, 5));

		float rot;
		DWORD mapID;
		if (argc > 5)
		{
			IsNumber(state, 6);
			rot = (float)lua_tonumber(state, 6);
			if (argc > 6)
			{
				IsNumber(state, 7);
				mapID = (DWORD)lua_tonumber(state, 7);
			}
			else
			{
				mapID = 0;
			}
		}
		else
		{
			rot = 0;
			mapID = 0;
		}

		lua_pushnumber(state, SpawnObject(type, name, loc, rot, -1, mapID));
	}
	else
	{
		lua_pushnumber(state, SpawnObject(type, name, vect3f(), 0.0f));
	}

	return 1;
}

int spawn_object_location(lua_State* state)
{
	GetScript(state);

	int argc = FunctionTop(state, 3);
	IsString(state, 1);
	IsString(state, 2);
	IsString(state, 3);

	const char* type = lua_tostring(state, 1);
	const char* name = lua_tostring(state, 2);
	std::string location = lua_tostring(state, 3);

	vect3f loc;
	size_t index = GlobalLocations.Find(std::wstring(location.begin(), location.end()));
	if (index != -1)
		loc = GlobalLocations.Get(index);

	float rot;
	DWORD mapID;
	if (argc > 3)
	{
		IsNumber(state, 4);
		rot = (float)lua_tonumber(state, 4);
		if (argc > 4)
		{
			IsNumber(state, 5);
			mapID = (DWORD)lua_tonumber(state, 5);
		}
		else
		{
			mapID = 0;
		}
	}
	else
	{
		rot = 0;
		mapID = 0;
	}

	lua_pushnumber(state, SpawnObject(type, name, loc, rot, -1, mapID));

	return 1;
}

int spawn_projectile(lua_State* state)
{
	GetScript(state);

	int argc = FunctionTop(state, 5);
	IsNumber(state, 1);
	IsNumber(state, 2);
	IsNumber(state, 3);
	IsNumber(state, 4);
	IsNumber(state, 5);

	DWORD mapID = (DWORD)lua_tonumber(state, 1);
	DWORD parentID = (DWORD)lua_tonumber(state, 2);

	vect3f loc((float)lua_tonumber(state, 3), (float)lua_tonumber(state, 4), (float)lua_tonumber(state, 5));
	vect3f mov;
	if (argc >= 8)
	{
		IsNumber(state, 6);
		IsNumber(state, 7);
		IsNumber(state, 8);
		mov.x = (float)lua_tonumber(state, 6);
		mov.y = (float)lua_tonumber(state, 7);
		mov.z = (float)lua_tonumber(state, 8);
	}

	lua_pushnumber(state, SpawnProjectile(mapID, parentID, loc, mov));

	return 1;
}

int destroy_object(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	DWORD ObjectIndex = (DWORD)lua_tonumber(state, 1);

	DestroyObject(ObjectIndex);

	for (size_t i = 0; i < SpawnedVehicles.size(); i++)
	{
		if (SpawnedVehicles[i].ObjectID == ObjectIndex)
		{
			SpawnedVehicles.erase(SpawnedVehicles.begin() + i);
			break;
		}
	}

	return 0;
}

int sync_ammo(lua_State* state)
{
	GetScript(state);

	int argc = FunctionTop(state, 1);
	IsNumber(state, 1);

	DWORD ObjectIndex = (DWORD)lua_tonumber(state, 1);
	DWORD MagazineIndex;

	if (argc > 1)
	{
		IsNumber(state, 2);
		MagazineIndex = (DWORD)lua_tonumber(state, 2);
		if (MagazineIndex > 1)
			MagazineIndex = 0;
	}
	else
	{
		MagazineIndex = 0;
	}

	SyncAmmo(ObjectIndex, MagazineIndex);

	return 0;
}

int powerup_interact(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsNumber(state, 2);

	DWORD ObjectID = (DWORD)lua_tonumber(state, 1);

	int MachineIndex = lua_tointeger(state, 2) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);	

	lua_pushboolean(state, InteractWithPowerup(PlayerIndex, ObjectID));

	return 1;
}

int assign_weapon(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsNumber(state, 2);

	DWORD ObjectIndex = (DWORD)lua_tonumber(state, 1);

	int MachineIndex = lua_tointeger(state, 2) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	lua_pushboolean(state, AssignPlayerWeapon(PlayerIndex, ObjectIndex));

	return 1;
}

int drop_weapon(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	DropWeapon(PlayerIndex);

	return 0;
}

int enter_vehicle(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 3);
	IsNumber(state, 1);
	IsNumber(state, 2);
	IsNumber(state, 3);

	DWORD ObjectIndex = (DWORD)lua_tonumber(state, 1);

	int MachineIndex = lua_tointeger(state, 2) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	DWORD Seat = lua_tointeger(state, 3);

	lua_pushboolean(state, EnterVehicle(PlayerIndex, ObjectIndex, Seat));

	return 1;
}

int exit_vehicle(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	EjectVehicle(PlayerIndex);

	return 0;
}

int kill(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	if (PlayerAlive(PlayerIndex))
	{
		KillPlayer(PlayerIndex);
	}

	return 0;
}

int camo(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsNumber(state, 2);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	DWORD duration = lua_tointeger(state, 2);

	MakeInvisible(PlayerIndex, duration);

	return 0;
}

int intersect(lua_State* state)
{
	GetScript(state);

	int argc = FunctionTop(state, 6);
	IsNumber(state, 1);
	IsNumber(state, 2);
	IsNumber(state, 3);
	IsNumber(state, 4);
	IsNumber(state, 5);
	IsNumber(state, 6);

	DWORD unit = 0;
	if (argc > 6)
	{
		IsNumber(state, 7);
		unit = (DWORD)lua_tonumber(state, 7);
	}

	vect3f pos((float)lua_tonumber(state, 1), (float)lua_tonumber(state, 2), (float)lua_tonumber(state, 3));
	vect3f vec((float)lua_tonumber(state, 4), (float)lua_tonumber(state, 5), (float)lua_tonumber(state, 6));
	vect3f out;
	DWORD unit_hit = 0xFFFFFFFF;
	bool ret = Intersect(unit, pos, vec, out, unit_hit);

	lua_pushboolean(state, ret);
	lua_pushnumber(state, out.x);
	lua_pushnumber(state, out.y);
	lua_pushnumber(state, out.z);
	lua_pushnumber(state, unit_hit);

	return 5;
}

int sig_scan(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 1);
	IsString(state, 1);

	const char* sig = lua_tostring(state, 1);

	DWORD res = FindCode(hModule, sig);

	lua_pushnumber(state, res);

	return 1;
}

int system_status(lua_State* state)
{
	GetScript(state);

	lua_pushnumber(state, GlobalSystemMonitor.GetCurrentProcessCPULoad());
	lua_pushnumber(state, GlobalSystemMonitor.GetTotalCPULoad());
	lua_pushnumber(state, GlobalSystemMonitor.GetMemoryLoad());

	return 3;
}

int safe_read(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 1);
	IsBoolean(state, 1);

	script.safe_read = (bool)lua_toboolean(state, 1);

	return 0;
}

int read_bit(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsNumber(state, 2);

	char* Address = (char*)(DWORD)lua_tonumber(state, 1);
	char* Value = ReadMemory(Address, script.safe_read);
	if (Value != nullptr)
	{
		size_t Bit = (size_t)lua_tonumber(state, 2);
		lua_pushnumber(state, (bool)(unsigned char)((*Value) & (1 << Bit)));
	}
	else
		lua_pushnil(state);

	return 1;
}

int read_char(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	char* Address = (char*)(DWORD)lua_tonumber(state, 1);
	char* Value = ReadMemory(Address, script.safe_read);
	if (Value != nullptr)
		lua_pushnumber(state, *Value);
	else
		lua_pushnil(state);

	return 1;
}

int read_byte(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	BYTE* Address = (BYTE*)(DWORD)lua_tonumber(state, 1);
	BYTE* Value = ReadMemory(Address, script.safe_read);
	if (Value != nullptr)
		lua_pushnumber(state, *Value);
	else
		lua_pushnil(state);

	return 1;
}

int read_short(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	short* Address = (short*)(DWORD)lua_tonumber(state, 1);
	short* Value = ReadMemory(Address, script.safe_read);
	if (Value != nullptr)
		lua_pushnumber(state, *Value);
	else
		lua_pushnil(state);

	return 1;
}

int read_word(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	WORD* Address = (WORD*)(DWORD)lua_tonumber(state, 1);
	WORD* Value = ReadMemory(Address, script.safe_read);
	if (Value != nullptr)
		lua_pushnumber(state, *Value);
	else
		lua_pushnil(state);

	return 1;
}

int read_int(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	int* Address = (int*)(DWORD)lua_tonumber(state, 1);
	int* Value = ReadMemory(Address, script.safe_read);
	if (Value != nullptr)
		lua_pushnumber(state, *Value);
	else
		lua_pushnil(state);

	return 1;
}

int read_dword(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	DWORD* Address = (DWORD*)(DWORD)lua_tonumber(state, 1);
	DWORD* Value = ReadMemory(Address, script.safe_read);
	if (Value != nullptr)
		lua_pushnumber(state, *Value);
	else
		lua_pushnil(state);

	return 1;
}

int read_float(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	float* Address = (float*)(DWORD)lua_tonumber(state, 1);
	float* Value = ReadMemory(Address, script.safe_read);
	if (Value != nullptr)
		lua_pushnumber(state, *Value);
	else
		lua_pushnil(state);

	return 1;
}

int read_double(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	double* Address = (double*)(DWORD)lua_tonumber(state, 1);
	double* Value = ReadMemory(Address, script.safe_read);
	if (Value != nullptr)
		lua_pushnumber(state, *Value);
	else
		lua_pushnil(state);

	return 1;
}

int read_vector3d(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	vect3f* Address = (vect3f*)(DWORD)lua_tonumber(state, 1);
	vect3f* Value = ReadMemory(Address, script.safe_read);
	if (Value != nullptr)
	{
		lua_pushnumber(state, Value->x);
		lua_pushnumber(state, Value->y);
		lua_pushnumber(state, Value->z);
	}
	else
	{
		lua_pushnil(state);
		lua_pushnil(state);
		lua_pushnil(state);
	}

	return 3;
}

int read_string(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 1);
	IsNumber(state, 1);

	char* Address = (char*)(DWORD)lua_tonumber(state, 1);
	char* Value = ReadMemory(Address, script.safe_read, 4096);
	if (Value != nullptr)
		lua_pushstring(state, Value);
	else
		lua_pushnil(state);

	return 1;
}

int safe_write(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 1);
	IsBoolean(state, 1);

	script.safe_write = (bool)lua_toboolean(state, 1);

	return 0;
}

int write_bit(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 3);
	IsNumber(state, 1);
	IsNumber(state, 2);
	IsNumber(state, 3);

	char* Address = (char*)(DWORD)lua_tonumber(state, 1);
	size_t Bit = (size_t)lua_tonumber(state, 2);
	bool Value = (bool)lua_tonumber(state, 3);

	if (script.safe_write)
	{
		DWORD OldProtect;
		if (VirtualProtect(Address, 1, PAGE_EXECUTE_READWRITE, &OldProtect))
		{
			if (!Value)
			{
				*Address &= (unsigned char)(~(1 << Bit));
			}
			else
			{
				*Address |= (unsigned char)(1 << Bit);
			}
			VirtualProtect(Address, 1, OldProtect, &OldProtect);
			lua_pushboolean(state, true);
		}
		else
		{
			lua_pushboolean(state, false);
		}
	}
	else
	{
		if (!Value)
		{
			*Address &= (unsigned char)(~(1 << Bit));
		}
		else
		{
			*Address |= (unsigned char)(1 << Bit);
		}

		lua_pushboolean(state, true);
	}

	return 1;
}

int write_char(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsNumber(state, 2);

	char* Address = (char*)(DWORD)lua_tonumber(state, 1);
	char Value = (char)lua_tonumber(state, 2);
	lua_pushboolean(state, WriteMemory(Address, Value, script.safe_write));

	return 1;
}

int write_byte(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsNumber(state, 2);

	BYTE* Address = (BYTE*)(DWORD)lua_tonumber(state, 1);
	BYTE Value = (BYTE)lua_tonumber(state, 2);
	lua_pushboolean(state, WriteMemory(Address, Value, script.safe_write));

	return 1;
}

int write_short(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsNumber(state, 2);

	short* Address = (short*)(DWORD)lua_tonumber(state, 1);
	short Value = (short)lua_tonumber(state, 2);
	lua_pushboolean(state, WriteMemory(Address, Value, script.safe_write));

	return 1;
}

int write_word(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsNumber(state, 2);

	WORD* Address = (WORD*)(DWORD)lua_tonumber(state, 1);
	WORD Value = (WORD)lua_tonumber(state, 2);
	lua_pushboolean(state, WriteMemory(Address, Value, script.safe_write));

	return 1;
}

int write_int(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsNumber(state, 2);

	int* Address = (int*)(DWORD)lua_tonumber(state, 1);
	int Value = (int)lua_tonumber(state, 2);
	lua_pushboolean(state, WriteMemory(Address, Value, script.safe_write));

	return 1;
}

int write_dword(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsNumber(state, 2);

	DWORD* Address = (DWORD*)(DWORD)lua_tonumber(state, 1);
	DWORD Value = (DWORD)lua_tonumber(state, 2);
	lua_pushboolean(state, WriteMemory(Address, Value, script.safe_write));

	return 1;
}

int write_float(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsNumber(state, 2);

	float* Address = (float*)(DWORD)lua_tonumber(state, 1);
	float Value = (float)lua_tonumber(state, 2);
	lua_pushboolean(state, WriteMemory(Address, Value, script.safe_write));

	return 1;
}

int write_double(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsNumber(state, 2);

	double* Address = (double*)(DWORD)lua_tonumber(state, 1);
	double Value = lua_tonumber(state, 2);
	lua_pushboolean(state, WriteMemory(Address, Value, script.safe_write));

	return 1;
}

int write_vector3d(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 4);
	IsNumber(state, 1);
	IsNumber(state, 2);
	IsNumber(state, 3);
	IsNumber(state, 4);

	vect3f* Address = (vect3f*)(DWORD)lua_tonumber(state, 1);
	vect3f Value;
	Value.x = (float)lua_tonumber(state, 2);
	Value.y = (float)lua_tonumber(state, 3);
	Value.z = (float)lua_tonumber(state, 4);
	lua_pushboolean(state, WriteMemory(Address, Value, script.safe_write));

	return 1;
}

int write_string(lua_State* state)
{
	Lua_Script& script = GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsString(state, 2);

	char* Address = (char*)(DWORD)lua_tonumber(state, 1);
	const char* Value = lua_tostring(state, 2);

	if (script.safe_write)
	{
		size_t len = strlen(Value) + 1;
		DWORD OldProtect;
		if (VirtualProtect(Address, len, PAGE_EXECUTE_READWRITE, &OldProtect))
		{
			strcpy(Address, Value);
			VirtualProtect(Address, len, OldProtect, &OldProtect);
			lua_pushboolean(state, true);
		}
		else
		{
			lua_pushboolean(state, false);
		}
	}
	else
	{
		strcpy(Address, Value);
		lua_pushboolean(state, true);
	}

	return 1;
}

template <class T, class U>
static void message(lua_State* state, std::function<U(size_t, const char*)> transform = [] (size_t i, const char* s) { return s; })
{
	GetScript(state);

	int argc = FunctionTop(state, 2);
	IsNumber(state, 1);
	IsString(state, 2);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	if (PlayerPresent(PlayerIndex) && !stfu[PlayerIndex])
	{
		if (argc < 6)
		{
			NetworkEventPlayer<T>(PlayerIndex, transform(PlayerIndex, lua_tostring(state, 2)), 0, 0, 255, 255);
		}
		else
		{
			IsNumber(state, 3);
			IsNumber(state, 4);
			IsNumber(state, 5);

			uint8_t red = (uint8_t)lua_tointeger(state, 3);
			uint8_t green = (uint8_t)lua_tointeger(state, 4);
			uint8_t blue = (uint8_t)lua_tointeger(state, 5);
			uint8_t alpha;

			if (argc > 5)
			{
				IsNumber(state, 6);
				alpha = (uint8_t)lua_tointeger(state, 6);
			}
			else
			{
				alpha = 255;
			}
			NetworkEventPlayer<T>(PlayerIndex, transform(PlayerIndex, lua_tostring(state, 2)), red, green, blue, alpha);
		}
	}
}

int chat_message(lua_State* state)
{
	message<EventChatText, std::wstring>(state, [] (size_t i, const char* s)
	{
		std::wstring msg = FormatWArgs(L"%hs", s);
		WStrReplace(i, msg);
		return msg;
	});

	return 0;
}

int rcon_message(lua_State* state)
{
	message<EventRconText, const char*>(state);

	return 0;
}

int hud_message(lua_State* state)
{
	message<EventHUDText, const char*>(state);

	return 0;
}

int play_sound(lua_State* state)
{
	GetScript(state);

	FunctionTop(state, 2);
	IsNumber(state, 1);
	IsNumber(state, 2);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	if (PlayerPresent(PlayerIndex) && !stfu[PlayerIndex])
	{
		NetworkEventPlayer<Event_Sound>(PlayerIndex, (uint8_t)lua_tonumber(state, 2));
	}

	return 0;
}

int hud_color(lua_State* state)
{
	GetScript(state);

	int argc = FunctionTop(state, 5);
	IsNumber(state, 1);
	IsNumber(state, 2);

	int MachineIndex = lua_tointeger(state, 1) - 1;
	int PlayerIndex = NumberToIndex(MachineIndex);

	size_t index = lua_tointeger(state, 2);

	if (index == 0)
	{
		NetworkEventPlayer<NetworkEventBase>(PlayerIndex, NE_HUD_Color, NE_HUD_Color_Reset);
	}
	else if (index < 5 && PlayerPresent(PlayerIndex))
	{
		IsNumber(state, 3);
		IsNumber(state, 4);
		IsNumber(state, 5);

		uint8_t red = (uint8_t)lua_tointeger(state, 3);
		uint8_t green = (uint8_t)lua_tointeger(state, 4);
		uint8_t blue = (uint8_t)lua_tointeger(state, 5);
		uint8_t alpha;

		if (argc > 5)
		{
			IsNumber(state, 6);
			alpha = (uint8_t)lua_tointeger(state, 6);
		}
		else
		{
			alpha = 255;
		}

		NetworkEventPlayer<HUD_Color>(PlayerIndex, static_cast<NE_HUD_ColorType>(index), red, green, blue, alpha);
	}

	return 0;
}

#define LUA_REGISTER(name) lua_register(state, #name, name);

void RegisterFunctions(lua_State* state)
{
	LUA_REGISTER(timer)
	LUA_REGISTER(register_callback)
	LUA_REGISTER(unregister_callback)
	LUA_REGISTER(execute_command)
	LUA_REGISTER(execute_command_sequence)
	LUA_REGISTER(say)
	LUA_REGISTER(say_all)
	LUA_REGISTER(rprint)
	LUA_REGISTER(cprint)
	LUA_REGISTER(rand)
	LUA_REGISTER(add_var)
	LUA_REGISTER(del_var)
	LUA_REGISTER(get_var)
	LUA_REGISTER(set_var)
	LUA_REGISTER(to_real_index)
	LUA_REGISTER(to_player_index)
	LUA_REGISTER(player_present)
	LUA_REGISTER(player_alive)
	LUA_REGISTER(get_player)
	LUA_REGISTER(get_dynamic_player)
	LUA_REGISTER(get_object_memory)
	LUA_REGISTER(lookup_tag)
	LUA_REGISTER(spawn_object)
	LUA_REGISTER(spawn_object_location)
	LUA_REGISTER(spawn_projectile)
	LUA_REGISTER(destroy_object)
	LUA_REGISTER(sync_ammo)
	LUA_REGISTER(powerup_interact)
	LUA_REGISTER(assign_weapon)
	LUA_REGISTER(drop_weapon)
	LUA_REGISTER(enter_vehicle)
	LUA_REGISTER(exit_vehicle)
	LUA_REGISTER(kill)
	LUA_REGISTER(camo)
	LUA_REGISTER(intersect)
	LUA_REGISTER(sig_scan)
	LUA_REGISTER(system_status)
	LUA_REGISTER(safe_read)
	LUA_REGISTER(read_bit)
	LUA_REGISTER(read_char)
	LUA_REGISTER(read_byte)
	LUA_REGISTER(read_short)
	LUA_REGISTER(read_word)
	LUA_REGISTER(read_int)
	LUA_REGISTER(read_dword)
	LUA_REGISTER(read_float)
	LUA_REGISTER(read_double)
	LUA_REGISTER(read_vector3d)
	LUA_REGISTER(read_string)
	LUA_REGISTER(safe_write)
	LUA_REGISTER(write_bit)
	LUA_REGISTER(write_char)
	LUA_REGISTER(write_byte)
	LUA_REGISTER(write_short)
	LUA_REGISTER(write_word)
	LUA_REGISTER(write_int)
	LUA_REGISTER(write_dword)
	LUA_REGISTER(write_float)
	LUA_REGISTER(write_double)
	LUA_REGISTER(write_vector3d)
	LUA_REGISTER(write_string)
	LUA_REGISTER(chat_message)
	LUA_REGISTER(rcon_message)
	LUA_REGISTER(hud_message)
	LUA_REGISTER(play_sound)
	LUA_REGISTER(hud_color)
}

}