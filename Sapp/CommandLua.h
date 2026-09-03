#pragma once

#include "GenericCommand.h"
#include "LuaHandler.h"

class CommandLuaLoad final : public GenericCommand
{
	std::wstring script_name;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			script_name = std::move(args[1]);
			return true;
		}
		return false;
	}

public:
	CommandLuaLoad() : GenericCommand(L"lua_load", L"<script name>", 4, ServerOnly::no, ClientOnly::no, Naughty::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (GlobalPrefs->Lua && !GlobalPrefs->ScrimMode)
		{
			if (lua::LoadScript(script_name))
			{
				env_Print(FormatWArgs(L"Successfully loaded %ls.lua!", script_name.c_str()).c_str());
			}
			else
			{
				env_Print(FormatWArgs(L"Failed to load %ls.lua!", script_name.c_str()).c_str());
			}
			return true;
		}
		else
		{
			env_Print(L"Lua scripting is disabled!");
		}

		return false;
	}
};

class CommandLuaUnload final : public GenericCommand
{
	std::wstring script_name;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			script_name = std::move(args[1]);
			return true;
		}
		return false;
	}

public:
	CommandLuaUnload() : GenericCommand(L"lua_unload", L"<script name>", 4, ServerOnly::no, ClientOnly::no, Naughty::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (GlobalPrefs->Lua && !GlobalPrefs->ScrimMode)
		{
			if (lua::UnloadScript(script_name))
			{
				env_Print(FormatWArgs(L"Successfully unloaded %ls.lua!", script_name.c_str()).c_str());
			}
			else
			{
				env_Print(FormatWArgs(L"Failed to unload %ls.lua!", script_name.c_str()).c_str());
			}
			return true;
		}
		else
		{
			env_Print(L"Lua scripting is disabled!");
		}

		return false;
	}
};

class CommandLuaList final : public GenericCommand
{
public:
	CommandLuaList() : GenericCommand(L"lua_list") {}

	bool Execute(const CommandEnviroment& env) override
	{
		env.StreamPrint(lua::StreamOut());
		return true;
	}
};

class CommandLuaCall final : public GenericCommand
{
	ArgContainer* arg_container;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc >= 3)
		{
			arg_container = &args;
			return true;
		}
		return false;
	}

public:
	CommandLuaCall() : GenericCommand(L"lua_call", L"<script name> <function name> [arguments]...", 4, ServerOnly::no, ClientOnly::no, Naughty::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (GlobalPrefs->Lua && !GlobalPrefs->ScrimMode)
		{
			return lua::LuaCallCustomFunc(*arg_container);
		}
		else
		{
			env_Print(L"Lua scripting is disabled!");
		}

		return false;
	}
};

class CommandLuaAPIV final : public GenericCommand
{
public:
	CommandLuaAPIV() : GenericCommand(L"lua_api_v") {}

	bool Execute(const CommandEnviroment& env) override
	{
		env_Print(FormatArgs("Lua API Version: %hs", GlobalPrefs->LuaAPIVersion).c_str());
		return true;
	}
};