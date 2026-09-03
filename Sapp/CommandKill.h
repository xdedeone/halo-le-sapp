#pragma once

#include "ObjectManagement.h"
#include "PlayerCommand.h"

class CommandKill final : public PlayerCommand
{
public:
	CommandKill() : PlayerCommand(L"kill", L"", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{							
		KillPlayer(PlayerIndex);
		env_Print(FormatWArgs(L"Killed %ls.", PlayerTable[PlayerIndex].name).c_str());
		return true;
	}
};
