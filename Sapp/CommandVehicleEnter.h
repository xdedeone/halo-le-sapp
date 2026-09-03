#pragma once

#include "ObjectManagement.h"
#include "PlayerCommand.h"

class CommandVehicleEnter final : public PlayerCommand
{
	size_t seat;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc < 3)
		{
			seat = 0;
			return true;
		}
		else if (args.argc == 3)
		{
			if (ToNumber(args[2], seat))
				return true;
		}
		return false;
	}

public:
	CommandVehicleEnter() : PlayerCommand(L"venter", L"[seat(=0)]", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if (!SpawnedVehicles.empty())
		{
			env_Print(FormatWArgs(L"Placing %ls inside the last spawned vehicle.", PlayerTable[PlayerIndex].name).c_str());
			EnterVehicle(PlayerIndex, SpawnedVehicles.back().ObjectID, seat);
		}
		else
		{
			env_Print(FormatWArgs(L"Can't place %ls inside a vehicle, no vehicles have been manually spawned.", PlayerTable[PlayerIndex].name).c_str());
		}

		return true;
	}
};