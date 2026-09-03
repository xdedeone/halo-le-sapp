#pragma once

#include <fstream>

#include "CommandExecuter.h"
#include "GenericCommand.h"
#include "LogThread.h"
#include "OnlineAdmins.h"
#include "PlayerCommand.h"
#include "Prefs.h"

class CommandBan final : public PlayerCommand
{
	std::wstring reason;
	std::string time;

public:
	CommandBan() : PlayerCommand(L"b", L"[reason] [time]", 3) {}
	
	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			reason = L"-";
			time.clear();
			return true;
		}
		else if (args.argc == 3)
		{
			reason = std::move(args[2]);
			time.clear();
			return true;
		}
		else if(args.argc == 4)
		{
			reason = std::move(args[2]);
			time = WToStr(args[3]);
			if (time.size() > 32)
				time.resize(32);
			return true;
		}
		return false;
	}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override	
	{
		if(PlayerIndex == env.sPlayerIndex)
		{
			env_Print("Error: You can't ban yourself, you moron!");
			return true;
		}
		if(GlobalPrefs->AdminBan == 2)
		{
			int player_level = GlobalOnlineAdminList.GetAdminLevel(PlayerIndex);
			if(player_level >= env.PrivilegeLevel)
			{
				env_Print("Error: You can't ban a player with higher or equal level than you!");
				return true;
			}
		}
		else if(GlobalPrefs->AdminBan == 1)
		{
			int player_level = GlobalOnlineAdminList.GetAdminLevel(PlayerIndex);
			if(player_level > env.PrivilegeLevel)
			{
				env_Print("Error: You can't ban a player with higher level than you!");
				return true;
			}
		}

		std::wstring wstr = FormatWArgs(L"%ls was banned by %ls. Reason: %ls", PlayerTable[PlayerIndex].name, env.sPlayerIndex == -1 ? L"the server" : GlobalPrefs->HideAdmin ? L"the admin" : PlayerTable[env.sPlayerIndex].name, reason.c_str());
		if (env.type != Enviroment::CHAT)
		{
			env_Print(wstr.c_str());
		}
		if (env.PrivilegeLevel < 5)
		{
			sv_say(wstr.c_str());
			WriteLog(L"BAN", move(wstr));
		}
		sv_ban(PlayerIndex, time.c_str());

		return true;
	}
};

class CommandUnBan final : public GenericCommand
{
	size_t index;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			if (ToNumber(args[1], index))
				return true;
		}
		return false;
	}

public:
	CommandUnBan() : GenericCommand(L"ub", L"<ban index>", 3) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		ExecuteHaloCommand(-1, Enviroment::CONSOLE, FormatArgs("sv_unban %u", index).c_str());
		env_Print(FormatArgs("Ban ID: %u has been unbanned.", index).c_str());
		return true;
	}
};

class CommandBans final : public GenericCommand
{
public:
	CommandBans() : GenericCommand(L"bans", L"", 3) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		std::ifstream BansFile(Banlist);
		if(!BansFile.is_open())
		{
			env_Print("Bans file not found.");
			return true;
		}

		std::vector <std::string> names;
		std::string line;

		getline(BansFile, line);

		while(getline(BansFile, line))
		{
			size_t n;
			if ((n = line.find_first_not_of(" \t")) != std::string::npos)
			{
				if(line[n] == ';')
					continue;
			}
			
			size_t comma = line.find_first_of(',');
			names.emplace_back(line.substr(0, comma));
		}
		BansFile.close();

		if (names.empty())
		{
			env_Print("There are no CD-Key bans.");
		}
		else
		{			
			int n = (int)names.size();

			for (int i = 0; i < n - 3; i += 4)
			{
				env_Print(FormatArgs("[%u  %hs]  [%u  %hs]  [%u  %hs]  [%u  %hs]", i, names[i].c_str(), i + 1, names[i + 1].c_str(), i + 2, names[i + 2].c_str(), i + 3, names[i + 3].c_str()).c_str());
			}

			if (n % 4 == 3)
			{
				env_Print(FormatArgs("[%u  %hs]  [%u  %hs]  [%u  %hs]", n - 3, names[n - 3].c_str(), n - 2, names[n - 2].c_str(), n - 1, names[n - 1].c_str()).c_str());
			}
			else if (n % 4 == 2)
			{
				env_Print(FormatArgs("[%u  %hs]  [%u  %hs]", n - 2, names[n - 2].c_str(), n - 1, names[n - 1].c_str()).c_str());
			}
			else if (n % 4 == 1)
			{
				env_Print(FormatArgs("[%u  %hs]", n - 1, names[n - 1].c_str()).c_str());
			}
		}

		return true;
	}
};