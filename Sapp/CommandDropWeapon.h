#pragma once

#include "ObjectManagement.h"
#include "PlayerCommand.h"

class CommandDropWeapon final : public PlayerCommand
{
public:
	CommandDropWeapon() : PlayerCommand(L"wdrop", L"", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		env_Print(FormatWArgs(L"Dropping weapon from %ls's hand.", PlayerTable[PlayerIndex].name).c_str());
		DropWeapon(PlayerIndex);
		return true;
	}
};