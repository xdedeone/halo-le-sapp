#pragma once

#include "Admins.h"
#include "bans.h"
#include "Event.h"
#include "EventScan.h"
#include "CommandFactory.h"
#include "CustomCommand.h"
#include "GenericCommand.h"
#include "MiscellaneousFunctions.h"
#include "PlayerCommand.h"
#include "PlayerJoin-Leave.h"
#include "ThreadCycleFix.h"

#pragma warning(disable: 4996) // 'GetVersionExA': was declared deprecated

class CommandUsage final : public GenericCommand
{
	std::wstring command_name;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			command_name = std::move(args[1]);
			return true;
		}
		return false;
	}

public:
	CommandUsage() : GenericCommand(L"usage", L"<command name>", -1) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (!GlobalCommandFactory.PrintUsage(command_name, env))
		{
			env_Print(FormatWArgs(L"Command \"%ls\" not found! Use \"help\" to get help for default Halo commands.", command_name.c_str()).c_str());
		}
		return true;
	}
};

class CommandSet final : public GenericCommand
{
	std::wstring command_name;
	std::wstring new_name;
	int new_level;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			command_name = std::move(args[1]);
			if (ToNumber(args[2], new_level))
			{
				new_name.clear();
				if (new_level >= -1 && new_level <= 4)
					return true;
			}
			else
			{
				new_name = std::move(args[2]);
				return true;
			}
		}
		return false;
	}

public:
	CommandSet() : GenericCommand(L"setcmd", L"<command name> <new name>\n<command name> <new level(-1-4)>") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if(!new_name.empty())
		{
			if(GlobalCommandFactory.Exists(new_name))
			{
				env_Print(FormatWArgs(L"Error: The name \"%ls\" is already taken by another command!", new_name.c_str()).c_str());
				return true;
			}
		}

		bool changed;
		if(!new_name.empty())
			changed = GlobalCommandFactory.ChangeName(command_name, new_name);
		else
			changed = GlobalCommandFactory.ChangeLevel(command_name, new_level);

		if(changed)
		{
			if(!new_name.empty())
			{
				env_Print(FormatWArgs(L"Command \"%ls\" has been changed to \"%ls\".", command_name.c_str(), new_name.c_str()).c_str());
			}
			else
			{
				env_Print(FormatWArgs(L"Command \"%ls\" has been changed to level %d.", command_name.c_str(), new_level).c_str());
			}
		}
		else
		{
			env_Print(FormatWArgs(L"Command \"%ls\" not found.", command_name.c_str()).c_str());
		}

		return true;
	}
};

class CommandList final : public GenericCommand
{
	CommandType command_type;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 1)
		{
			command_type = CommandType::BASE;
			return true;
		}
		else if (args.argc == 2)
		{
			if (!_wcsicmp(args[1].c_str(), L"generic"))
				command_type = CommandType::GENERIC;
			else if (!_wcsicmp(args[1].c_str(), L"player"))
				command_type = CommandType::PLAYER;
			else if (!_wcsicmp(args[1].c_str(), L"custom"))
				command_type = CommandType::CUSTOM;
			else
				return false;
			return true;
		}
		return false;
	}

public:
	CommandList() : GenericCommand(L"list", L"[generic/player/custom]", -1) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		GlobalCommandFactory.ListCommands(command_type, env);
		return true;
	}
};

class CommandLogin final : public GenericCommand
{
	std::string password;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			password = WToStr(args[1]);
			return true;
		}
		return false;
	}

public:
	CommandLogin() : GenericCommand(L"login", L"<password>", -1, ServerOnly::no, ClientOnly::yes, Naughty::no, Unalterable::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		int current_level = GlobalOnlineAdminList.GetAdminLevel(env.sPlayerIndex);
		if(current_level < 0)
		{
			int admin_level = -1;
			if (!GlobalPrefs->SappRcon && RconPassword[0] != 0 && !strcmp(password.c_str(), RconPassword))
				admin_level = 4;
			else
			{
				unsigned char pwmd5[16];
				MD5_Hash(password.c_str(), password.length(), pwmd5);

				std::string password_hash = ConvertToMD5Hash(pwmd5);

				admin_level = GlobalAdminList.IsAdmin(PlayerTable[env.sPlayerIndex].name, password_hash);
			}

			if (admin_level > -1)
			{
				env_Print(FormatWArgs(L"You have successfully logged in, %ls!", PlayerTable[env.sPlayerIndex].name).c_str());
				Login(env.sPlayerIndex, admin_level);
			}
			else
			{
				env_Print("Incorrect User or Password!");
				WriteLog(L"LOGIN", FormatWArgs(L"Player %ls failed to log in.", PlayerTable[env.sPlayerIndex].name));

				if (++LoginFails[env.sPlayerIndex] == 3)
				{
					env_Print("Warning! One more failed login access and you will be banned!");
				}
				else if (LoginFails[env.sPlayerIndex] == 4)
				{
					WriteLog(L"IPBAN", FormatWArgs(L"Player %ls was IP banned for more than 3 failed login attempts.", PlayerTable[env.sPlayerIndex].name));

					NetInfoStruct* info = **Server->MachineInfo[IndexToNumber(env.sPlayerIndex)].IPAddress;
					if (info != NULL)
					{
						GlobalIPBanlist.BanIP(info->IP, 0xFFFFFFFF, 32, time(0) + 3600, PlayerTable[env.sPlayerIndex].name, L"FAILED LOGIN ATTEMPTS");
					}
					if (!GlobalPrefs->FullIPBan)
						sv_kick(env.sPlayerIndex);
				}
			}
		}
		else
		{
			env_Print(FormatWArgs(L"You are already logged in, %ls!", PlayerTable[env.sPlayerIndex].name).c_str());
		}

		return true;
	}
};

class CommandAFK final : public GenericCommand
{
public:
	CommandAFK() : GenericCommand(L"afk", L"", 0, ServerOnly::no, ClientOnly::yes, Naughty::yes) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if (!AFK[env.sPlayerIndex])
		{
			env_Print("Now you won't respawn after you die until you type this command again.");
			AFK[env.sPlayerIndex] = true;
		}
		else
		{
			if(PlayerAlive(env.sPlayerIndex))
			{
				env_Print("Now you will respawn after you die.");
			}
			else if (PlayerTable[env.sPlayerIndex].RespawnTimer >= 30)
			{
				env_Print(FormatArgs("You will respawn in %u seconds.", PlayerTable[env.sPlayerIndex].RespawnTimer / 30 + 1).c_str());
			}
			else
			{
				env_Print("You will respawn in a moment.");
			}

			AFK[env.sPlayerIndex] = false;
		}

		return true;
	}
};

class CommandSTFU final : public GenericCommand
{
public:
	CommandSTFU() : GenericCommand(L"stfu", L"", -1, ServerOnly::no, ClientOnly::yes, Naughty::no, Unalterable::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		stfu[env.sPlayerIndex] = true;
		env_Print("The server has been stfu'd for you!");
		return true;
	}
};

class CommandUnSTFU final : public GenericCommand
{
public:
	CommandUnSTFU() : GenericCommand(L"unstfu", L"", -1, ServerOnly::no, ClientOnly::yes, Naughty::no, Unalterable::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		stfu[env.sPlayerIndex] = false;
		env_Print("The server has been unstfu'd for you!");
		return true;
	}
};

class CommandStats final : public GenericCommand
{
public:
	CommandStats() : GenericCommand(L"stats", L"", -1, ServerOnly::no, ClientOnly::yes) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		size_t diff = static_cast<size_t>(time(0) - PlayerStats[env.sPlayerIndex].Joined);
			
		size_t hours = diff / 3600;
		diff -= hours * 3600;
		size_t minutes = diff / 60;
		diff -= minutes * 60;
		size_t seconds = diff;

		env_Print(FormatArgs("You have been playing here for %u hours %u minutes and %u seconds.", hours, minutes, seconds).c_str());

		long k = PlayerStats[env.sPlayerIndex].Kills + PlayerTable[env.sPlayerIndex].kills;
		long a = PlayerStats[env.sPlayerIndex].Assists + PlayerTable[env.sPlayerIndex].assists;
		WORD d = PlayerStats[env.sPlayerIndex].Deaths + PlayerTable[env.sPlayerIndex].deaths;

		env_Print(FormatArgs("Kills: %d  Assists: %d  Deaths: %d", k, a, d).c_str());

		if(d == 0)
		{
			env_Print("Kill\\Death Ratio is infinitive");
		}
		else
		{
			env_Print(FormatArgs("Kill\\Death Ratio: %4.2f", (static_cast<float>(k)) / (static_cast<float>(d))).c_str());
		}

		return true;	
	}
};

class CommandSv_Stats final : public GenericCommand
{
public:
	CommandSv_Stats() : GenericCommand(L"sv_stats", L"", -1) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		env_Print(FormatArgs("Server Queries: %u    Join Attempts: %u", stats::ServerQueries, stats::JoinAttempts).c_str());
		env_Print(FormatArgs("Executed Commands: %u    Executed Events: %u", stats::Commands, stats::NumberOfExecutedEvents).c_str());
		env_Print(FormatArgs("Games Played: %u    Flags Captured: %u", stats::Games, stats::Captures).c_str());
		env_Print(FormatArgs("Total Kills: %u    Total Betrays: %u    Total Suicides: %u", stats::Kills, stats::Betrays, stats::Suicides).c_str());
		env_Print(FormatArgs("Chat Messages: %u", stats::Chat).c_str());
		return true;
	}
};

class CommandCPU final : public GenericCommand
{
public:
	CommandCPU() : GenericCommand(L"cpu") {}

	bool Execute(const CommandEnviroment& env) override
	{
		char BrandString[48];

		ProccessorBrandString(BrandString);

		env_Print(FormatArgs("Processor: %hs", BrandString).c_str());

		OSVERSIONINFO OsVersion;
		OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		GetVersionEx(&OsVersion);
		env_Print(FormatArgs("Major: %u Minor: %u Build: %u PlatformId: %u",
			OsVersion.dwMajorVersion, OsVersion.dwMinorVersion,
			OsVersion.dwBuildNumber, OsVersion.dwPlatformId).c_str());
		env_Print(FormatArgs("OS info: %hs", OsVersion.szCSDVersion).c_str());

		env_Print(FormatArgs("CPU Load (Current / Total): %u%% / %u%% Memory Load: %u%%", GlobalSystemMonitor.GetCurrentProcessCPULoad(), GlobalSystemMonitor.GetTotalCPULoad(), GlobalSystemMonitor.GetMemoryLoad()).c_str());

		return true;
	}
};

class CommandFiles final : public GenericCommand
{
public:
	CommandFiles() : GenericCommand(L"files") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		char currentPath[FILENAME_MAX] = {};
		GetCurrentDirectory(FILENAME_MAX, currentPath);
		env_Print(FormatArgs("users, UCS-2 LE: %hs\\sapp\\users.txt", currentPath).c_str());
		env_Print(FormatArgs("admins, UCS-2 LE: %hs\\sapp\\admins.txt", currentPath).c_str());
		env_Print(FormatArgs("ipbans, UCS-2 LE %hs\\sapp\\ipbans.txt", currentPath).c_str());
		env_Print(FormatArgs("locations, UCS-2 LE: %hs\\sapp\\location.txt", currentPath).c_str());
		env_Print(FormatArgs("areas, UCS-2 LE: %hs\\sapp\\areas.txt", currentPath).c_str());
		env_Print(FormatArgs("init (sapp's), ANSI: %hs\\sapp\\init.txt", GametypesFolder).c_str());
		env_Print(FormatArgs("mapcycle (sapp's), ANSI: %hs\\sapp\\mapcycle.txt", GametypesFolder).c_str());
		env_Print(FormatArgs("mapvotes, UCS-2 LE: %hs\\sapp\\mapvotes.txt", GametypesFolder).c_str());
		env_Print(FormatArgs("custom commands, UCS-2 LE: %hs\\sapp\\commands.txt", GametypesFolder).c_str());
		env_Print(FormatArgs("events, UCS-2 LE: %hs\\sapp\\events.txt", GametypesFolder).c_str());
		env_Print(FormatArgs("alias, UCS-2 LE: %hs\\sapp\\alias.txt", GametypesFolder).c_str());
		env_Print(FormatArgs("log, UCS-2 LE: %hs\\sapp\\%ls.txt", GametypesFolder, log_name.c_str()).c_str());
		return true;
	}
};

class CommandUptime final : public GenericCommand
{
public:
	CommandUptime() : GenericCommand(L"uptime", L"", 0) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		size_t diff = static_cast<size_t>(time(0) - stats::load_time);
			
		size_t days = diff / 86400;
		diff -= days * 86400;
		size_t hours = diff / 3600;
		diff -= hours * 3600;
		size_t minutes = diff / 60;
		diff -= minutes * 60;
		size_t seconds = diff;

		env_Print(FormatArgs("SAPP Uptime: %u days %u hours %u minutes %u seconds", days, hours, minutes, seconds).c_str());

		ULONGLONG SystemTime = GetTickCount64() / 1000;

		days = static_cast<size_t>(SystemTime / 86400);
		SystemTime -= days * 86400;
		hours = static_cast<size_t>(SystemTime / 3600);
		SystemTime -= hours * 3600;
		minutes = static_cast<size_t>(SystemTime / 60);
		SystemTime -= minutes * 60;
		seconds = static_cast<size_t>(SystemTime);

		env_Print(FormatArgs("System uptime: %u days %u hours %u minutes %u seconds", days, hours, minutes, seconds).c_str());
		return true;
	}
};

class CommandInfo final : public GenericCommand
{
public:
	CommandInfo() : GenericCommand(L"info", L"", -1, ServerOnly::yes, ClientOnly::no, Naughty::no, Unalterable::yes) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		size_t i = 0;
		while(Server->ServerName[i] < 33 && i < 64)
			i++;

		std::string timeLeft;
		int TimeLeft = GetTimeLeft() / 30;
		if (TimeLeft > 0)
		{
			timeLeft = FormatArgs("%u minutes %u seconds", TimeLeft / 60, TimeLeft % 60);
		}
		else
		{
			timeLeft = "Infinite";
		}

		env_Print(FormatWArgs(L"SAPP Version %hs %hs", GlobalPrefs->AppVersion, GlobalPrefs->AppHaloType).c_str());
		env_Print(FormatWArgs(L"Server Name: %ls", Server->ServerName + i).c_str());
		env_Print(FormatWArgs(L"Map: %hs | GameType: %ls", CurrentMap.c_str(), CurrentModeW.c_str()).c_str());
		env_Print(FormatWArgs(L"Time Left: %hs | Players: %u/%u", timeLeft.c_str(), Server->PlayerNumber, Server->MaxPlayers).c_str());
		env_Print(FormatWArgs(L"Scrim Mode: %ls | Anticheat: %ls", GlobalPrefs->ScrimMode ? L"ON" : L"OFF", GlobalPrefs->AntiCheat ? L"ON" : L"OFF").c_str());
		return true;
	}
};

class CommandAbout final : public GenericCommand
{
public:
	CommandAbout() : GenericCommand(L"about", L"", -1, ServerOnly::no, ClientOnly::no, Naughty::no, Unalterable::yes) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		env_Print(FormatArgs("SAPP Version %hs %hs", GlobalPrefs->AppVersion, GlobalPrefs->AppHaloType).c_str());
		if (env.type == Enviroment::CHAT || env.type == Enviroment::RCON)
		{
			env_Print(L"Developed by sehé°° (2017), continuing the project of Termy (2011), modified by louk.");
		}
		else
		{
			env_Print("Developed by seh‚řř (2017), continuing the project of Termy (2011), modified by louk.");
		}
		env_Print("URL: halo.isimaginary.com");
		env_Print("Steam/Evolve: Szotyi2501");
		return true;
	}
};

class CommandLogNote final : public GenericCommand
{
	std::wstring message;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			message = std::move(args[1]);
			return true;
		}
		return false;
	}

public:
	CommandLogNote() : GenericCommand(L"log_note", L"<message>") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if(GlobalPrefs->Log)
		{
			WriteLog(L"NOTE", std::move(message));
			env_Print("A note was written to the log file.");
		}
		else
		{
			env_Print("Error: SAPP Logging is disabled!");
		}

		return true;
	}
};

class CommandText final : public GenericCommand
{
	std::string message;
	short color;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			message = WToStr(args[1]);
			color = GlobalPrefs->ConAttrib;
			return true;
		}
		else if (args.argc == 3)
		{
			message = WToStr(args[1]);
			if (ToNumber(args[2], color))
				return true;
		}
		return false;
	}

public:
	CommandText() : GenericCommand(L"text", L"<message> [color]") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		PrintConsole(color, FormatArgs("%hs", message.c_str()).c_str());
		return true;
	}
};

class CommandBeep final : public GenericCommand
{
	size_t hz;
	size_t ms;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 1)
		{
			hz = 1000;
			ms = 1000;
			return true;
		}
		else if (args.argc == 2)
		{
			if (!ToNumber(args[1], hz))
				return false;
			ms = 1000;
			return true;
		}
		else if (args.argc == 3)
		{
			if (!ToNumber(args[1], hz))
				return false;
			if (ToNumber(args[2], ms))
				return true;
		}
		return false;
	}

public:
	CommandBeep() : GenericCommand(L"beep", L"[hertz(=1000)] [milliseconds(=1000)]") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		Beep(hz, ms);
		return true;
	}
};

class CommandyEyE final : public GenericCommand
{
public:
	CommandyEyE() : GenericCommand(L"yeye") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		sv_say(L"I LUV U ALL @ IM TOO SEXY FOR THE GAME @@@@@@@@");
		return true;
	}
};

class CommandAyyLmao final : public GenericCommand
{
public:
	CommandAyyLmao() : GenericCommand(L"ayy lmao", L"", 4, ServerOnly::no, ClientOnly::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		PlayerTable[env.sPlayerIndex].speed = -1;
		env.Print(L"ayy lmao");
		return true;
	}
};
