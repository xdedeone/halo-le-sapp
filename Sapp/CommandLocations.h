#pragma once

#include "Addresses.h"
#include "GenericCommand.h"
#include "Locations.h"

class CommandLocAdd final : public GenericCommand
{
	std::wstring loc_name;
	vect3f pos;
	bool from_player;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			from_player = true;
			loc_name = std::move(args[1]);
			return true;
		}
		else if (args.argc == 5)
		{
			from_player = false;
			loc_name = std::move(args[1]);
			if (!ToNumber(args[2], pos.x))
				return false;
			if (!ToNumber(args[3], pos.y))
				return false;
			if (ToNumber(args[4], pos.z))
				return true;
		}
		return false;
	}

public:
	CommandLocAdd() : GenericCommand(L"loc_add", L"<location name> [x] [y] [z]") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (GlobalLocations.Find(loc_name) != -1)
		{
			env_Print(FormatWArgs(L"Location %ls already exists for %hs.", loc_name.c_str(), CurrentMap.c_str()).c_str());
			return false;
		}

		if (from_player)
		{
			if (!PlayerAliveOnly(env.sPlayerIndex))
			{
				env_Print("You have no location; you are dead.");
				return false;
			}

			GetPlayerPosition(env.sPlayerIndex, pos);
		}

		GlobalLocations.Add(loc_name, pos);
		env_Print(FormatWArgs(L"Location %ls has been added to %hs.", loc_name.c_str(), CurrentMap.c_str()).c_str());

		return true;
	}
};

class CommandLocDel final : public GenericCommand
{
	std::wstring loc_name;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			loc_name = std::move(args[1]);
			return true;
		}
		return false;
	}

public:
	CommandLocDel() : GenericCommand(L"loc_del", L"<location name>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		size_t i = GlobalLocations.Find(loc_name);
		if (i != -1)
		{
			GlobalLocations.Del(i);
			env_Print(FormatWArgs(L"Removed %ls from %hs.", loc_name.c_str(), CurrentMap.c_str()).c_str());
		}
		else
		{
			env_Print(FormatWArgs(L"Location %ls does not exists for %hs.", loc_name.c_str(), CurrentMap.c_str()).c_str());
		}

		return true;
	}
};

class CommandLocList final : public GenericCommand
{
public:
	CommandLocList() : GenericCommand(L"loc_list") {}

	bool Execute(const CommandEnviroment& env) override
	{
		env.StreamPrint(GlobalLocations.StreamOut());
		return true;
	}
};

class CommandLocListAll final : public GenericCommand
{
public:
	CommandLocListAll() : GenericCommand(L"loc_listall") {}

	bool Execute(const CommandEnviroment& env) override
	{
		env.StreamPrint(GlobalLocations.StreamOutAll());
		return true;
	}
};