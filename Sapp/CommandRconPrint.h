#pragma once

#include "Haloutilities.h"
#include "TextUtilities.h"
#include "PlayerCommand.h"
#include "PlayerJoin-Leave.h"

class CommandRconPrint final : public PlayerCommand
{
	std::string message;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			message = WToStr(args[2]);
			return true;
		}
		return false;
	}

public:
	CommandRconPrint() : PlayerCommand(L"rprint", L"<message>") {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if (stfu[PlayerIndex])
			return true;

		PrintRcon(PlayerIndex, message.c_str());

		return true;
	}
};