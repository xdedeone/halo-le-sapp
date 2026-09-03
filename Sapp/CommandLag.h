#pragma once

#include "PlayerCommand.h"

class CommandLag final : public PlayerCommand
{
public:
	CommandLag() : PlayerCommand(L"lag", L"", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

		Player->SuspendUnit = 1;

		env_Print(FormatWArgs(L"%ls is lagged.", PlayerTable[PlayerIndex].name).c_str());
		return true;
	}
};


class CommandUnlag final : public PlayerCommand
{
public:
	CommandUnlag() : PlayerCommand(L"unlag", L"", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

		Player->SuspendUnit = 0;

		env_Print(FormatWArgs(L"%ls is unlagged.", PlayerTable[PlayerIndex].name).c_str());
		return true;
	}
};