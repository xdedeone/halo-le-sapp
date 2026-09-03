#pragma once

#include "PlayerCommand.h"

class CommandVehicleExit final : public PlayerCommand
{
public:
	CommandVehicleExit() : PlayerCommand(L"vexit", L"", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if (!PlayerInVehicle(PlayerIndex))
		{
			env_Print(FormatWArgs(L"%ls isn't in a vehicle.", PlayerTable[PlayerIndex].name).c_str());
			return true;
		}

		EjectVehicle(PlayerIndex);

		env_Print(FormatWArgs(L"Ejecting %ls from the vehicle.", PlayerTable[PlayerIndex].name).c_str());

		return true;
	}
};