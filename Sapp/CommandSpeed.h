#pragma once

#include "PlayerCommand.h"

class CommandSpeed final : public PlayerCommand
{
	bool setSpeed;
	std::wstring speedToSet;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc < 3)
		{
			setSpeed = false;
			return true;
		}
		else if (args.argc == 3)
		{
			setSpeed = true;
			speedToSet = std::move(args[2]);
			return true;
		}
		return false;
	}

public:
	CommandSpeed() : PlayerCommand(L"s", L"[speed]", 4, Naughty::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if(setSpeed)
			ProcessOperator(speedToSet, PlayerTable[PlayerIndex].speed);

		env_Print(FormatWArgs(L"%ls's speed: %4.2f", PlayerTable[PlayerIndex].name, PlayerTable[PlayerIndex].speed).c_str());
		return true;
	}
};