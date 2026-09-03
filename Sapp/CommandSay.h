#pragma once

#include "PlayerCommand.h"
#include "PlayerJoin-Leave.h"

class CommandSay final : public PlayerCommand
{
	std::wstring orig_message;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			orig_message = std::move(args[2]);
			return true;
		}
		return false;
	}

public:
	CommandSay() : PlayerCommand(L"say", L"<message>", 2) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if (stfu[PlayerIndex])
			return true;

		std::wstring message = orig_message;
		for(size_t i = 0; i < message.length(); i++)
		{
			if(message[i] == '\\')
			{
				switch (message[i+1])
				{
					case 'n':
					{
						message.replace(i, 2, PlayerTable[PlayerIndex].name);
						break;
					}
					case 't':
					{
						message.replace(i, 2, PlayerTable[PlayerIndex].Team == 0 ? L"red": L"blue");
						break;
					}
					case 'o':
					{
						message.replace(i, 2, PlayerTable[PlayerIndex].Team == 0 ? L"blue": L"red");
						break;
					}
					case 's':
					{
						message.replace(i, 2, GlobalPrefs->Me);
						break;
					}
					case 'y':
					{
						message.replace(i, 2, GlobalPrefs->Ye);
						break;
					}
				}
			}
		}

		bool admin = (env.type != Enviroment::SCRIPT);

		MsgSend(PlayerIndex, message.c_str(), admin);

		return true;
	}
};
