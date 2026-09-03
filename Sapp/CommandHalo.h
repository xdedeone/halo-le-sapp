#pragma once

#include "EventScan.h"
#include "GenericCommand.h"
#include "MapCheckAndExecute.h"
#include "MapLoader.h"
#include "NetworkEvents.h"

class CommandTell final : public GenericCommand
{
	std::wstring message;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc > 1)
		{
			message.clear();
			for (size_t i = 1; i < args.argc; i++)
				message += args[i] + L" ";
			return true;
		}
		return false;
	}

public:
	CommandTell() : GenericCommand(L"tell", L"<message>...", 2, ServerOnly::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		bool admin = (env.type != Enviroment::SCRIPT);

		sv_say(message.c_str(), admin);

		return true;
	}
};

class CommandMap final : public GenericCommand
{
	std::string map;
	std::string gametype;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			map = WToStr(args[1]);
			gametype = WToStr(args[2]);
			return true;
		}
		return false;
	}

public:
	CommandMap() : GenericCommand(L"map", L"<map> <gametype>", 3) {}

	bool Execute(const CommandEnviroment& env) override
	{	
		if(!RunMap(map.c_str(), gametype.c_str()))
		{
			if (env.type != Enviroment::CONSOLE)
			{
				env_Print(L"Error: Invalid map or gametype!");
			}
		}
		else if(GlobalPrefs->MapVote && !MapVoteCanceled)
		{
			MapVoteCanceled = true;
			sv_say(L"Map Voting has been cancelled!");
		}
		
		return true;
	}
};

class CommandMapList final : public GenericCommand
{
public:
	CommandMapList() : GenericCommand(L"maplist", L"", 3) {}

	bool Execute(const CommandEnviroment& env) override
	{
		for(size_t i = 0; i < *MapTableSize - 2; i += 3)
		{
			env_Print(FormatArgs("%24hs %24hs %24hs", SappMapTable[i].name, SappMapTable[i + 1].name, SappMapTable[i + 2].name).c_str());
		}
		if (*MapTableSize % 3 == 2)
		{
			env_Print(FormatArgs("%24hs %24hs", SappMapTable[*MapTableSize - 2].name, SappMapTable[*MapTableSize - 1].name).c_str());
		}
		else if (*MapTableSize % 3 == 1)
		{
			env_Print(FormatArgs("%24hs", SappMapTable[*MapTableSize - 1].name).c_str());
		}

		return true;
	}
};

class CommandReloadGametypes final : public GenericCommand
{
public:
	CommandReloadGametypes() : GenericCommand(L"reload_gametypes", L"") {}

	bool Execute(const CommandEnviroment& env) override
	{
		ReloadGametypes();

		env_Print("Gametypes were reloaded.");

		return true;
	}
};

class CommandMotd final : public GenericCommand
{
	std::string message;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 1)
		{
			message.clear();
			return true;
		}
		else if (args.argc > 1)
		{
			message = WToStr(args[1]);
			return true;
		}
		return false;
	}

public:
	CommandMotd() : GenericCommand(L"motd", L"[message]") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (!message.empty())
		{
			strcpy_s(Motd, 256, message.c_str());

			BroadcastMotd(message.c_str());
		}

		env_Print(FormatArgs("Motd: %hs", Motd).c_str());

		return true;
	}
};

class CommandScoreLimit final : public GenericCommand
{
	std::wstring scorelimit;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 1)
		{
			scorelimit.clear();
			return true;
		}
		else if (args.argc == 2)
		{
			scorelimit = std::move(args[1]);
			return true;
		}
		return false;
	}

public:
	CommandScoreLimit() : GenericCommand(L"scorelimit", L"[score]", 4, ServerOnly::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (!scorelimit.empty())
		{
			ProcessOperator(scorelimit, *ScoreLimit);
			if (CurrentGameType == GameType::CTF)
			{
				ProcessOperator(scorelimit, CTFScores->Scorelimit);
			}
			else if (CurrentGameType == GameType::Oddball)
			{
				if (*ScoreType != 2)
				{
					int tmp = OddballScores->Scorelimit / 1800;
					ProcessOperator(scorelimit, tmp);
					OddballScores->Scorelimit = tmp * 1800;
				}
				else
				{
					ProcessOperator(scorelimit, OddballScores->Scorelimit);
				}
			}
		}

		env_Print(FormatArgs("Score Limit: %d", *ScoreLimit).c_str());	

		return true;
	}
};

class CommandTimelimit final : public GenericCommand
{
	std::wstring timelimit;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 1)
		{
			timelimit.clear();
			return true;
		}
		else if (args.argc == 2)
		{
			timelimit = std::move(args[1]);
			return true;
		}
		return false;
	}

public:
	CommandTimelimit() : GenericCommand(L"timelimit", L"[minutes]", 4, ServerOnly::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		int TimeLimit = (*Timelimit) / 1800;

		if (!timelimit.empty())
		{
			ProcessOperator(timelimit, TimeLimit);
			*Timelimit = TimeLimit * 1800;
			NetworkEvent<DataSync<int>>(NE_DataSync_TimeLeft, GetTimeLeft());
		}

		if (TimeLimit > 0)
		{
			env_Print(FormatArgs("Time Limit: %d minutes", TimeLimit).c_str());
		}
		else
		{
			env_Print(FormatArgs("Time Limit: infinite").c_str());
		}

		return true;
	}
};

class CommandHillTimer final : public GenericCommand
{
	std::wstring hilltimer;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 1)
		{
			hilltimer.clear();
			return true;
		}
		else if (args.argc == 2)
		{
			hilltimer = std::move(args[1]);
			return true;
		}
		return false;
	}

public:
	CommandHillTimer() : GenericCommand(L"hill_timer", L"[seconds]", 4, ServerOnly::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		size_t HillTimer = (GlobalPrefs->HillTimer) / 30;

		if (!hilltimer.empty())
		{
			ProcessOperator(hilltimer, HillTimer);
			GlobalPrefs->HillTimer = HillTimer * 30;
		}

		env_Print(FormatArgs("Hill Timer: %u seconds", HillTimer).c_str());

		return true;
	}
};