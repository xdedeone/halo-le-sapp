#pragma once
#pragma warning(disable: 4800) // warning C4800: 'size_t' : forcing value to bool 'true' or 'false' (performance warning)

#include "PlayerCommand.h"
#include "ObjectInteractions.h"

class CommandBlockObject final : public PlayerCommand
{
	std::string object_name;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			object_name = WToStr(args[2]);
			return true;
		}
		return false;
	}

public:
	CommandBlockObject() : PlayerCommand(L"block_object", L"<object name with path>", 4, Naughty::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		auto it = DisabledObjects.find(object_name);
		if (it != DisabledObjects.end())
		{
			it->second.Players[PlayerIndex] = true;
			ObjectCache.clear();
		}
		else
		{
			ObjectUsage TB;
			TB.Players[PlayerIndex] = true;
			DisabledObjects[object_name] = TB;
			ObjectCache.clear();
		}

		env_Print(FormatWArgs(L"Usage of object %hs has been forbidden for %ls.", object_name.c_str(), PlayerTable[PlayerIndex].name).c_str());

		return true;
	}
};

class CommandUnblockObject final : public PlayerCommand
{
	std::string object_name;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			object_name = WToStr(args[2]);
			return true;
		}
		return false;
	}

public:
	CommandUnblockObject() : PlayerCommand(L"unblock_object", L"<object name with path>", 4, Naughty::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		auto it = DisabledObjects.find(object_name);
		if (it != DisabledObjects.end())
		{
			it->second.Players[PlayerIndex] = false;
			RemoveIfEmpty(it);
			ObjectCache.clear();
		}

		env_Print(FormatWArgs(L"Usage of object %hs has been allowed for %ls.", object_name.c_str(), PlayerTable[PlayerIndex].name).c_str());

		return true;
	}
};

class CommandBlockAllObjects final : public PlayerCommand
{
	bool block;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			size_t d;
			if (!ToNumber(args[2], d) || d > 1)
				return false;

			block = static_cast<bool>(d);
			return true;
		}
		return false;
	}

public:
	CommandBlockAllObjects() : PlayerCommand(L"block_all_objects", L"<block(0-1)>") {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		GlobalPrefs->DisableObjectsPlayer[PlayerIndex] = block;

		env_Print(FormatWArgs(L"Interaction with any object has been %hs for %ls.", block ? "disallowed" : "allowed", PlayerTable[PlayerIndex].name).c_str());

		return true;
	}
};

class CommandBlockAllVehicles final : public PlayerCommand
{
	bool block;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			size_t d;
			if (!ToNumber(args[2], d) || d > 1)
				return false;

			block = static_cast<bool>(d);
			return true;
		}
		return false;
	}

public:
	CommandBlockAllVehicles() : PlayerCommand(L"block_all_vehicles", L"<block(0-1)>") {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		GlobalPrefs->DisableVehiclesPlayer[PlayerIndex] = block;

		env_Print(FormatWArgs(L"Interaction with any object has been %hs for %ls.", block ? "disallowed" : "allowed", PlayerTable[PlayerIndex].name).c_str());

		return true;
	}
};