#pragma once

#include "ObjectManagement.h"
#include "PlayerCommand.h"

class CommandAssignWeapon final : public PlayerCommand
{
	DWORD ObjectID;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc < 3)
		{
			ObjectID = LastWeaponSpawned;
			return true;
		}
		return false;
	}

public:
	CommandAssignWeapon() : PlayerCommand(L"wadd", L"", 4, Naughty::yes, AliveOnly::yes, NoMultipleExecution::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if (AssignPlayerWeapon(PlayerIndex, ObjectID))
		{
			env_Print(FormatWArgs(L"Successfully gave a weapon to %ls.", PlayerTable[PlayerIndex].name).c_str());
		}
		else
		{
			env_Print(FormatWArgs(L"Failed to give a weapon to %ls.", PlayerTable[PlayerIndex].name).c_str());
		}		

		return true;
	}
};