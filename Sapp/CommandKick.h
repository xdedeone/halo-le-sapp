#pragma once

#include "OnlineAdmins.h"
#include "PlayerCommand.h"

class CommandKick final : public PlayerCommand
{
	std::wstring reason;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			reason = L"-";
			return true;
		}
		else if (args.argc == 3)
		{
			reason = std::move(args[2]);
			return true;
		}
		return false;
	}

public:
	CommandKick() : PlayerCommand(L"k", L"[reason]", 2) {}
	
	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override	
	{
		if(GlobalPrefs->AdminBan == 2)
		{
			int player_level = GlobalOnlineAdminList.GetAdminLevel(PlayerIndex);
			if(player_level >= env.PrivilegeLevel)
			{
				env_Print("Error: You can't kick a player with higher or equal level than you!");
				return true;
			}
		}
		else if(GlobalPrefs->AdminBan == 1)
		{
			int player_level = GlobalOnlineAdminList.GetAdminLevel(PlayerIndex);
			if(player_level > env.PrivilegeLevel)
			{
				env_Print("Error: You can't kick a player with higher level than you!");
				return true;
			}
		}

		std::wstring wstr = FormatWArgs(L"%ls was kicked by %ls. Reason: %ls", PlayerTable[PlayerIndex].name, env.sPlayerIndex == -1 ? L"the server" : GlobalPrefs->HideAdmin ? L"the admin" : PlayerTable[env.sPlayerIndex].name, reason.c_str());
		if (env.type != Enviroment::CHAT)
		{
			env_Print(wstr.c_str());
		}

		sv_say(wstr.c_str());
		WriteLog(L"KICK", std::move(wstr));

		sv_kick(PlayerIndex);

		return true;
	}
};