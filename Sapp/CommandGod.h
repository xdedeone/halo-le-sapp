#pragma once

#include "PlayerCommand.h"

class CommandGod final : public PlayerCommand
{
public:
	CommandGod() : PlayerCommand(L"god", L"", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

		Player->IsInvincible |= 8;

		env_Print(FormatWArgs(L"%ls is now God.", PlayerTable[PlayerIndex].name).c_str());
		return true;
	}
};

class CommandUnGod final : public PlayerCommand
{
public:	
	CommandUnGod() : PlayerCommand(L"ungod", L"", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

		Player->IsInvincible &= ~8;

		env_Print(FormatWArgs(L"%ls is now normal Player.", PlayerTable[PlayerIndex].name).c_str());	
		return true;
	}
};