#pragma once

#include "GenericCommand.h"
#include "PlayerCommand.h"

class CommandTextBan final : public PlayerCommand
{
	size_t time;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			time = 0;
			return true;
		}
		else if (args.argc == 3)
		{
			if (ToNumber(args[2], time))
				return true;
		}
		return false;
	}

public:
	CommandTextBan() : PlayerCommand(L"textban", L"[minutes(=0)]", 2) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if(GlobalOnlineAdminList.GetAdminLevel(PlayerIndex) < 0)
		{
			GlobalTextBans.Ban(PlayerIndex, time);
			std::wstring wstr = FormatWArgs(L"%ls has been text-banned by %ls %ls.", PlayerTable[PlayerIndex].name, env.sPlayerIndex == -1 ? L"the server" : GlobalPrefs->HideAdmin ? L"the admin" : PlayerTable[env.sPlayerIndex].name, time == 0 ? L"permanently" : FormatWArgs(L"for %u minutes", time).c_str()); //lol ye ik xD
			if (env.type != Enviroment::CHAT)
			{
				env_Print(wstr.c_str());
			}
			sv_say(wstr.c_str());
			WriteLog(L"TEXTBAN", std::move(wstr));
		}
		else
		{
			env_Print("Error: You can't text-ban an admin!");
		}

		return true;
	}
};

class CommandTextUnBan final : public GenericCommand
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
	CommandTextUnBan() : GenericCommand(L"textunban", L"<text ban index>", 2) {}

	bool Execute(const CommandEnviroment& env) override
	{
		std::wstring unbanned = GlobalTextBans.Unban(index);
		if(unbanned.size())
		{
			std::wstring wstr = FormatWArgs(L"%ls has been text-unbanned.", unbanned.c_str());
			env_Print(wstr.c_str());
			WriteLog(L"TEXTUNBAN", std::move(wstr));
		}
		else
		{
			env_Print(FormatArgs("Text-ban does not exist with ID '%u'.", index).c_str());
		}

		return true;
	}
};

class CommandTextBans final : public GenericCommand
{
public:
	CommandTextBans() : GenericCommand(L"textbans", L"", 2) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		GlobalTextBans.Refresh();
		env.StreamPrint(GlobalTextBans.StreamOut());
		return true;
	}
};

class CommandMute final : public PlayerCommand
{
	size_t time;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			time = 0;
			return true;
		}
		else if (args.argc == 3)
		{
			if (ToNumber(args[2], time))
				return true;
		}
		return false;
	}

public:
	CommandMute() : PlayerCommand(L"mute", L"[minutes(=0)]", 2) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if(GlobalOnlineAdminList.GetAdminLevel(PlayerIndex) < 0)
		{
			GlobalMutedPlayers.Ban(PlayerIndex, time);
			std::wstring wstr = FormatWArgs(L"%ls has been muted by %ls %ls.", PlayerTable[PlayerIndex].name, env.sPlayerIndex == -1 ? L"the server" : GlobalPrefs->HideAdmin ? L"the admin" : PlayerTable[env.sPlayerIndex].name, time == 0 ? L"permanently" : FormatWArgs(L"for %u minutes", time).c_str()); // hehe ye xD
			if (env.type != Enviroment::CHAT)
			{
				env_Print(wstr.c_str());
			}
			sv_say(wstr.c_str());
			WriteLog(L"MUTE", std::move(wstr));
		}
		else
		{
			env_Print("Error: You can't mute an admin!");
		}

		return true;
	}
};

class CommandUnMute final : public GenericCommand
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
	CommandUnMute() : GenericCommand(L"unmute", L"<mute index>", 2) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		std::wstring unbanned = GlobalMutedPlayers.Unban(index);
		if(unbanned.size())
		{
			std::wstring wstr = FormatWArgs(L"%ls has been unmuted.", unbanned.c_str());
			env_Print(wstr.c_str());
			WriteLog(L"UNMUTE", std::move(wstr));
		}
		else
		{
			env_Print(FormatArgs("Mute does not exist with ID '%u'.", index).c_str());
		}

		return true;
	}
};

class CommandMutes final : public GenericCommand
{
public:
	CommandMutes() : GenericCommand(L"mutes", L"", 2) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		GlobalMutedPlayers.Refresh();
		env.StreamPrint(GlobalMutedPlayers.StreamOut());
		return true;
	}
};