#pragma once

#include "NetworkHook.h"
#include "PlayerCommand.h"

class CommandIp final : public PlayerCommand 
{
public:
	CommandIp() : PlayerCommand(L"ip", L"", 3, Naughty::no, AliveOnly::no, NoMultipleExecution::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		std::string ip = GetPlayerIP(IndexToNumber(PlayerIndex));

		env_Print(FormatArgs("IP: %hs", ip.c_str()).c_str());
		
		for (size_t i = 0; i < 32; i++)
		{
			if (ip[i] == ':')
			{
				ip[i] = 0;
				break;
			}
		}				
		
		char IP_DNS[1028];
		if (Getnameinfo(ip.c_str(), IP_DNS))
		{
			env_Print(FormatArgs("Rev DNS: %hs", IP_DNS).c_str());
		}

		return true;
	}
};