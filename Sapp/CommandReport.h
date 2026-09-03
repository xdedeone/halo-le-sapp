#pragma once

#include "GenericCommand.h"
#include "NetworkClient.h"

class CommandReport final : public GenericCommand
{
	std::string message;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 1)
		{
			return true;
		}
		else if (args.argc == 2)
		{
			message = WToStr(args[1]);
			return true;
		}
		return false;
	}

public:
	CommandReport() : GenericCommand(L"report", L"[message]", -1, ServerOnly::no, ClientOnly::yes, Naughty::no, Unalterable::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (AntiCheat)
		{
			if (LastKiller[env.sPlayerIndex] == 0)
			{
				env_Print("No one killed you yet!");
			}
			else
			{
				size_t MachineIndex = IndexToNumber(env.sPlayerIndex);
				for (size_t i = 0; i < AnticheatPlayers.size(); i++)
				{
					if (AnticheatPlayers[i].MachineID == MachineIndex)
					{
						AntcheatReport Report = { AnticheatPlayers[i].SessionKey, LastKiller[env.sPlayerIndex] };
						LastKiller[env.sPlayerIndex] = 0;
						if (!message.empty())
						{
							memcpy(Report.Message, message.c_str(), message.size() > 64 ? 64 : message.size());
						}
						GlobalSAPPServer.SendBigEvent(event_report, (char*)&Report, sizeof(AntcheatReport));
						env_Print("The report was sent!");
						break;
					}
				}
			}
			return true;
		}
		else
		{
			env_Print("This command can be used only with Anticheat ON!");
			return false;
		}		
	}
};