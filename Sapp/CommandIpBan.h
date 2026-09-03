#pragma once

#include "bans.h"
#include "GenericCommand.h"
#include "MiscellaneousFunctions.h"
#include "OnlineAdmins.h"
#include "PlayerCommand.h"

class CommandIPBan final : public PlayerCommand
{
	size_t ban_length;
	std::wstring reason;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			ban_length = 0;
			reason = L"-";
			return true;
		}
		else if (args.argc == 3)
		{
			if (!ToNumber(args[2], ban_length))
			{
				ban_length = 0;
				reason = std::move(args[2]);
			}
			else
			{
				reason = L"-";
			}
			return true;
		}
		else if (args.argc == 4)
		{
			if (ToNumber(args[2], ban_length))
				reason = std::move(args[3]);
			else if (ToNumber(args[3], ban_length))
				reason = std::move(args[2]);
			else
				return false;
			return true;
		}
		return false;
	}

public:
	CommandIPBan() : PlayerCommand(L"ipban", L"[reason]\n<minutes> <reason>\n<reason> <minutes>", 3) {}

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
				env_Print("Error: You can't IP-ban a player with higher or equal level than you!");
				return true;
			}
		}
		else if(GlobalPrefs->AdminBan == 1)
		{
			int player_level = GlobalOnlineAdminList.GetAdminLevel(PlayerIndex);
			if(player_level > env.PrivilegeLevel)
			{
				env_Print("Error: You can't IP-ban a player with higher level than you!");
				return true;
			}
		}

		std::wstring wstr = FormatWArgs(L"%ls was IP-banned by %ls. Reason: %ls", PlayerTable[PlayerIndex].name, env.sPlayerIndex == -1 ? L"the server" : GlobalPrefs->HideAdmin ? L"the admin" : PlayerTable[env.sPlayerIndex].name, reason.c_str());
		if (env.type != Enviroment::CHAT)
		{
			env_Print(wstr.c_str());
		}

		sv_say(wstr.c_str());
		WriteLog(L"IPBAN", std::move(wstr));

		NetInfoStruct* info = **Server->MachineInfo[IndexToNumber(PlayerIndex)].IPAddress;
		if (info != NULL)
		{
			GlobalIPBanlist.LoadIPBans();
			GlobalIPBanlist.BanIP(info->IP, 0xFFFFFFFF, 32, ban_length != 0 ? time(0) + ban_length * 60 : 0, PlayerTable[PlayerIndex].name, std::wstring(reason));
			GlobalIPBanlist.SaveIPBans();
		}

		if (!GlobalPrefs->FullIPBan)
			sv_kick(PlayerIndex);

		return true;
	}
};

class CommandIPRangeBan final : public GenericCommand
{
	std::wstring name;
	std::string ip_range;
	std::wstring reason;
	size_t ban_length;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			name = std::move(args[1]);
			ip_range = WToStr(args[2]);
			reason = L"-";
			ban_length = 0;
			return true;
		}
		else if (args.argc == 4)
		{
			name = std::move(args[1]);
			ip_range = WToStr(args[2]);
			reason = std::move(args[3]);
			ban_length = 0;
			return true;
		}
		else if (args.argc == 5)
		{
			name = std::move(args[1]);
			ip_range = WToStr(args[2]);
			reason = std::move(args[3]);
			if (ToNumber(args[4], ban_length))
				return true;
		}
		return false;
	}

public:
	CommandIPRangeBan() : GenericCommand(L"iprangeban", L"<name> <ip/mask> [reason] [minutes]") {}
	
	bool Execute(const CommandEnviroment& env) override	
	{
		size_t ip;
		size_t mask;
		size_t mask_range;
		if(ParseIP(ip_range, ip, mask, mask_range))
		{
			std::wstring wstr = FormatWArgs(L"IP range %ls has been banned. Reason: %ls", ip_range.c_str(), reason.c_str());
			env_Print(wstr.c_str());
			WriteLog(L"IPBAN", std::move(wstr));
			GlobalIPBanlist.LoadIPBans();
			GlobalIPBanlist.BanIP(ip, mask, mask_range, ban_length != 0 ? time(0) + ban_length * 60 : 0, std::move(name), std::move(reason));
			GlobalIPBanlist.SaveIPBans();
		}
		else
		{
			env_Print(FormatWArgs(L"Failed to parse IP range '%hs'.", ip_range.c_str()).c_str());
		}

		return true;
	}
};

class CommandIPUnBan final : public GenericCommand
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
	CommandIPUnBan() : GenericCommand(L"ipunban", L"<ip ban index>", 3) {}

	bool Execute(const CommandEnviroment& env) override
	{
		GlobalIPBanlist.LoadIPBans();
		unsigned long unbanned = GlobalIPBanlist.UnbanIP(index);
		if(unbanned > 0)
		{
			std::wstring wstr = FormatWArgs(L"IP %hs has been unbanned.", inet_ntoa(*(in_addr*)(&unbanned)));
			env_Print(wstr.c_str());			
			WriteLog(L"IPUNBAN", std::move(wstr));
			GlobalIPBanlist.SaveIPBans();
		}
		else
		{
			env_Print(FormatArgs("IP ban does not exist with ID '%u'.", index).c_str());
		}

		return true;
	}
};

class CommandIPBans final : public GenericCommand
{
public:
	CommandIPBans() : GenericCommand(L"ipbans", L"", 3) {}

	bool Execute(const CommandEnviroment& env) override
	{
		GlobalIPBanlist.Refresh();
		env.StreamPrint(GlobalIPBanlist.StreamOut());
		return true;
	}
};

class CommandRefreshIPbans final : public GenericCommand
{
public:
	CommandRefreshIPbans() : GenericCommand(L"refresh_ipbans", L"", 3) {}

	bool Execute(const CommandEnviroment& env) override
	{
		GlobalIPBanlist.Refresh();
		GlobalIPBanlist.LoadIPBans();
		GlobalIPBanlist.SaveIPBans();
		return true;
	}
};
