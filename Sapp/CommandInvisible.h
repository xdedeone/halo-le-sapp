#pragma once

#include "PlayerCommand.h"

class CommandInvisible final : public PlayerCommand
{
	int duration;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc < 3)
		{
			duration = 0;
			return true;
		}
		else if (args.argc == 3)
		{
			if (ToNumber(args[2], duration))
				return true;
		}
		return false;
	}

public:
	CommandInvisible() : PlayerCommand(L"camo", L"[duration(=0)]", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{	
		MakeInvisible(PlayerIndex, duration * 30); // 30 ticks / second (the speed of the halo thread)

		env_Print(FormatWArgs(L"%ls is now invisible.", PlayerTable[PlayerIndex].name).c_str());
		return true;
	}
};