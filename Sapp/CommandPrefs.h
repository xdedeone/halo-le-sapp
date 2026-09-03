#pragma once

#include "AimbotScan.h"
#include "AliasThread.h"
#include "ConsolePatch.h"
#include "EventScan.h"
#include "GenericCommand.h"
#include "mtv.h"
#include "NetworkEvents.h"
#include "NetworkHook.h"
#include "ObjectRespawn.h"
#include "ObjectSyncCleanup.h"
#include "RemoteConsole.h"
#include "ThreadCycleFix.h"

class CommandCMDStart1 final : public PrefStringCommand
{
public:
	CommandCMDStart1() : PrefStringCommand(L"cmdstart1", L"[character]") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if (set && value.size() > 0)
		{
			GlobalPrefs->CmdStart1 = value[0];
		}
			
		env_Print(FormatWArgs(L"cmdstart 1: %c", GlobalPrefs->CmdStart1).c_str());
		return true;
	}
};

class CommandCMDStart2 final : public PrefStringCommand
{
public:
	CommandCMDStart2() : PrefStringCommand(L"cmdstart2", L"[character]") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (set && value.size() > 0)
			GlobalPrefs->CmdStart2 = value[0];
		env_Print(FormatWArgs(L"cmdstart 2: %c", GlobalPrefs->CmdStart2).c_str());
		return true;
	}
};

class CommandLua final : public PrefBoolCommand
{
public:
	CommandLua() : PrefBoolCommand(L"lua") {}

	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->Lua);
		env_Print(FormatArgs("Lua: %hs", GlobalPrefs->Lua ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandMsgPrefix final : public PrefStringCommand
{
public:
	CommandMsgPrefix() : PrefStringCommand(L"msg_prefix", L"[prefix]") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
			GlobalPrefs->msg_prefix = value;
		env_Print(FormatWArgs(L"Message Prefix: %ls", GlobalPrefs->msg_prefix.c_str()).c_str());
		return true;
	}
};

class CommandAdminPrefix final : public PrefStringCommand
{
public:
	CommandAdminPrefix() : PrefStringCommand(L"admin_prefix", L"[prefix]") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
			GlobalPrefs->admin_prefix = value;
		env_Print(FormatWArgs(L"Admin Message Prefix: %ls", GlobalPrefs->admin_prefix.c_str()).c_str());
		return true;
	}
};

class CommandSayPrefix final : public PrefBoolCommand
{
public:
	CommandSayPrefix() : PrefBoolCommand(L"say_prefix") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->SPre);
		env_Print(FormatArgs("Say Prefix: %hs", GlobalPrefs->SPre ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandSAPPMapcycle final : public PrefBoolCommand
{
public:
	CommandSAPPMapcycle() : PrefBoolCommand(L"sapp_mapcycle") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		bool changed = ProcessBoolValue(value, GlobalPrefs->SappMapcycle);
		env_Print(FormatArgs("SAPP Mapcycle: %hs", GlobalPrefs->SappMapcycle ? "enabled" : "disabled").c_str());
		if (changed)
		{
			if (GlobalPrefs->SappMapcycle)
			{
				GlobalPrefs->Flags |= 4;
			}
			else
			{
				GlobalPrefs->Flags &= ~4;
			}
			if (GlobalPrefs->SappMapcycle && GlobalPrefs->MapVote)
			{
				GlobalPrefs->MapVote = false;
				env_Print("Map Voting is now disabled.");
			}
			FormatQueryString();
		}
		return true;
	}
};

class CommandMapVote final : public PrefBoolCommand
{
public:
	CommandMapVote() : PrefBoolCommand(L"mapvote") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		bool changed = ProcessBoolValue(value, GlobalPrefs->MapVote);
		env_Print(FormatArgs("Map Voting: %hs", GlobalPrefs->MapVote ? "enabled" : "disabled").c_str());
		if (changed && GlobalPrefs->MapVote && GlobalPrefs->SappMapcycle)
		{
			GlobalPrefs->SappMapcycle = false;
			GlobalPrefs->Flags &= ~4;
			env_Print("SAPP Mapcycle is now disabled.");
			FormatQueryString();
		}
		return true;
	}
};

class CommandBlockTC final : public PrefBoolCommand
{
public:
	CommandBlockTC() : PrefBoolCommand(L"block_tc") {}

	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->BlockTC);
		env_Print(FormatArgs("Blocking Team Change: %hs", GlobalPrefs->BlockTC ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandHideAdmin final : public PrefBoolCommand
{
public:
	CommandHideAdmin() : PrefBoolCommand(L"hide_admin") {}

	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->HideAdmin);
		env_Print(FormatArgs("Hiding the name of the admin who banned: %hs", GlobalPrefs->HideAdmin ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandSaveScores final : public PrefBoolCommand
{
public:
	CommandSaveScores() : PrefBoolCommand(L"save_scores") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->SaveScores);
		env_Print(FormatArgs("Saving Scores: %hs", GlobalPrefs->SaveScores ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandSaveRespawnTime final : public PrefBoolCommand
{
public:
	CommandSaveRespawnTime() : PrefBoolCommand(L"save_respawn_time") {}

	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->SaveRespawnTime);
		env_Print(FormatArgs("Saving Respawn Time: %hs", GlobalPrefs->SaveRespawnTime ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandZombies final : public PrefNumberCommand
{
public:
	CommandZombies() : PrefNumberCommand(L"zombies", L"[value(0-2)]", 0, 2) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
		{
			if (value != GlobalPrefs->Zombies)
			{
				if (value > 0)
				{
					GlobalPrefs->Flags |= 64;
				}
				else
				{
					GlobalPrefs->Flags &= ~64;
				}

				GlobalPrefs->Zombies = static_cast<byte>(value);

				NetworkEvent<DataSync<byte>>(NE_DataSync_Zombies, GlobalPrefs->Zombies);
				FormatQueryString();
			}
		}

		switch (GlobalPrefs->Zombies)
		{
		case 0:
		{
			env_Print("Zombies mode is disabled.");
			break;
		}
		case 1:
		{
			env_Print("Zombie team: red");
			break;
		}
		case 2:
		{
			env_Print("Zombie team: blue");
			break;
		}
		}

		return true;
	}
};

class CommandSJLevel final : public PrefNumberCommand
{
public:
	CommandSJLevel() : PrefNumberCommand(L"sj_level", L"[level(-1-5)]", -1, 5) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (GlobalPrefs->AntiCheat)
		{
			env_Print("Anticheat overrides this function and Sight-Jacking is disabled.");
		}
		else if (set)
		{
			if (value != GlobalPrefs->SJLevel)
			{
				if (GlobalPrefs->ScrimMode || value != -1)
				{
					GlobalPrefs->Flags |= 2;
				}
				else
				{
					GlobalPrefs->Flags &= ~2;
				}

				GlobalPrefs->SJLevel = value;
				UpdateSJLevel();
				FormatQueryString();
			}
		}

		env_Print(FormatArgs("Minimum level required for SightJacking: %d", GlobalPrefs->SJLevel).c_str());
		return true;
	}
};

class CommandScrimMode final : public PrefBoolCommand
{
public:
	CommandScrimMode() : PrefBoolCommand(L"scrim_mode") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		bool changed = ProcessBoolValue(value, GlobalPrefs->ScrimMode);
		env_Print(FormatArgs("Scrim Mode: %hs", GlobalPrefs->ScrimMode ? "enabled" : "disabled").c_str());
		if (changed)
		{
			if (GlobalPrefs->ScrimMode || GlobalPrefs->SJLevel != -1)
			{
				GlobalPrefs->Flags |= 2;
			}
			else
			{
				GlobalPrefs->Flags &= ~2;
			}
			FormatQueryString();
			ResetPlayers();
			sv_say(FormatWArgs(L"The Admin has %ls the Scrim Mode!", GlobalPrefs->ScrimMode ? L"ENABLED" : L"DISABLED").c_str());
		}
		return true;
	}
};

class CommandDisableBacktap final : public PrefBoolCommand
{
public:
	CommandDisableBacktap() : PrefBoolCommand(L"disable_backtap") {}

	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->DisableBacktap);
		env_Print(FormatArgs("Disabling Backtaps: %hs", GlobalPrefs->DisableBacktap ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandSappRcon final : public PrefBoolCommand
{
public:
	CommandSappRcon() : PrefBoolCommand(L"sapp_rcon") {}

	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->SappRcon);
		env_Print(FormatArgs("SAPP Rcon: %hs", GlobalPrefs->SappRcon ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandAntiCaps final : public PrefBoolCommand
{
public:
	CommandAntiCaps() : PrefBoolCommand(L"anticaps") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->AntiCaps);
		env_Print(FormatArgs("Anti CapsLock: %hs", GlobalPrefs->AntiCaps ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandFullIPBan final : public PrefBoolCommand
{
public:
	CommandFullIPBan() : PrefBoolCommand(L"full_ipban") {}

	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->FullIPBan);
		env_Print(FormatArgs("Full IPBan: %hs", GlobalPrefs->FullIPBan ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandAntiHaloFP final : public PrefBoolCommand
{
public:
	CommandAntiHaloFP() : PrefBoolCommand(L"antihalofp") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->AntiHaloFP);
		env_Print(FormatArgs("Anti Halofp: %hs", GlobalPrefs->AntiHaloFP ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandAntiGlitch final : public PrefBoolCommand
{
public:
	CommandAntiGlitch() : PrefBoolCommand(L"antiglitch") {}

	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->AntiGlitch);
		env_Print(FormatArgs("Anti Glitch: %hs", GlobalPrefs->AntiGlitch ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandAntiLagSpawn final : public PrefBoolCommand
{
public:
	CommandAntiLagSpawn() : PrefBoolCommand(L"antilagspawn") {}

	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->AntiLagSpawn);
		env_Print(FormatArgs("Anti Lag-Sapwn: %hs", GlobalPrefs->AntiLagSpawn ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandDebugStrings final : public PrefBoolCommand
{
public:
	CommandDebugStrings() : PrefBoolCommand(L"debug_strings") {}

	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->DebugStrings);
		env_Print(FormatArgs("Debug Strings: %hs", GlobalPrefs->DebugStrings ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandMaxVotes final : public PrefNumberCommand
{
public:
	CommandMaxVotes() : PrefNumberCommand(L"max_votes", L"[votes to display(1+)]", 1) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
		{
			GlobalPrefs->MaxVotes = value;
			if(value > 8)
			{
				env_Print("Warning! The amount of displayed mapvotes is more than 8!");
				env_Print("Note that default Halo-chat only has 8 rows!");
			}
		}
		env_Print(FormatArgs("Maximum MapVotes Displayed: %u", GlobalPrefs->MaxVotes).c_str());
		return true;
	}
};

class CommandMaxIdle final : public PrefNumberCommand
{
public:
	CommandMaxIdle() : PrefNumberCommand(L"max_idle", L"[seconds]") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
		{
			GlobalPrefs->MaxIdle = value * 30;
		}
		if(!GlobalPrefs->MaxIdle)
		{
			env_Print("Mapcycle will be never restarted if the server is idle.");
		}
		else
		{
			env_Print(FormatArgs("Idle time after mapcycle gets restarted: %u seconds", GlobalPrefs->MaxIdle / 30).c_str());
		}
		return true;
	}
};

class CommandPacketLimit final : public PrefNumberCommand
{
public:
	CommandPacketLimit() : PrefNumberCommand(L"packet_limit", L"[value(0 or 250+)]", 250) {}
	
	bool Execute(const CommandEnviroment& env) override
	{		
		if (set)
		{
			GlobalPrefs->PacketLimit = value;
		}
		if(GlobalPrefs->PacketLimit == 0)
		{
			env_Print("DoS Protection is inactive.");
		}
		else
		{
			env_Print(FormatArgs("DoS Protection is Active! Maximum allowed packet number per second: %u", GlobalPrefs->PacketLimit).c_str());
		}
		return true;
	}
};

class CommandAntiSpam final : public PrefNumberCommand
{
public:
	CommandAntiSpam() : PrefNumberCommand(L"antispam", L"[value(0-2)]", 0, 2) {}
	
	bool Execute(const CommandEnviroment& env) override
	{		
		if (set)
		{
			GlobalPrefs->AntiSpam = static_cast<byte>(value);
		}
		if(GlobalPrefs->AntiSpam == 0)
		{
			env_Print("Antispam is disabled.");
		}
		else if(GlobalPrefs->AntiSpam == 1)
		{
			env_Print("Antispam is enabled, chat-ban type: CD-Key Hash");
		}
		else
		{
			env_Print("Antispam is enabled, chat-ban type: IP");
		}
		return true;
	}
};

class CommandAdminBan final : public PrefNumberCommand
{
public:
	CommandAdminBan() : PrefNumberCommand(L"adminban", L"[value(0-2)]", 0, 2) {}
	
	bool Execute(const CommandEnviroment& env) override
	{		
		if (set)
		{			
			GlobalPrefs->AdminBan = value;
		}
		if(GlobalPrefs->AdminBan == 0)
		{
			env_Print("Admins can kick/ban another admin.");
		}
		else if(GlobalPrefs->AdminBan == 1)
		{
			env_Print("Admins can't kick/ban another admin with higher level.");
		}
		else
		{
			env_Print("Admins can't kick/ban another admin with higher or equal level.");
		}
		return true;
	}
};

class CommandAdminAddSameLevel final : public PrefNumberCommand
{
public:
	CommandAdminAddSameLevel() : PrefNumberCommand(L"adminadd_samelevel", L"[value(0-2)]", 0, 2) {}

	bool Execute(const CommandEnviroment& env) override
	{		
		if (set)
		{
			GlobalPrefs->AdminAddSameLevel = value;
		}
		if(GlobalPrefs->AdminAddSameLevel == 0)
		{
			env_Print("Admins can't add another admins.");
		}
		else if(GlobalPrefs->AdminAddSameLevel == 1)
		{
			env_Print("Admins can add another admins with lower level.");
		}
		else
		{
			env_Print("Admins can add another admins with lower or equal level.");
		}
		return true;
	}
};

class CommandAdminDelSameLevel final : public PrefNumberCommand
{
public:
	CommandAdminDelSameLevel() : PrefNumberCommand(L"admindel_samelevel", L"[value(0-2)]", 0, 2) {}

	bool Execute(const CommandEnviroment& env) override
	{		
		if (set)
		{
			GlobalPrefs->AdminDelSameLevel = value;
		}
		if(GlobalPrefs->AdminDelSameLevel == 0)
		{
			env_Print("Admins can't delete another admins.");
		}
		else if(GlobalPrefs->AdminDelSameLevel == 1)
		{
			env_Print("Admins can delete another admins with lower level.");
		}
		else
		{
			env_Print("Admins can delete another admins with lower or equal level.");
		}
		return true;
	}
};

class CommandAutoUpdate final : public PrefBoolCommand
{
public:
	CommandAutoUpdate() : PrefBoolCommand(L"auto_update") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->AutoUpdate);
		env_Print(FormatArgs("Auto Update: %hs", GlobalPrefs->AutoUpdate ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandNetworkThread final : public PrefBoolCommand
{
public:
	CommandNetworkThread() : PrefBoolCommand(L"network_thread") {}

	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->NetworkThread);
		env_Print(FormatArgs("Network Thread: %hs", GlobalPrefs->NetworkThread ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandCollectAliases final : public GenericCommand
{
	std::wstring value1;
	std::wstring value2;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 1)
		{
			value1.clear();
			value2.clear();
			return true;
		}
		else if (args.argc == 2)
		{
			value1 = std::move(args[1]);
			value2 = L"1";
			return true;
		}
		else if (args.argc == 3)
		{
			value1 = std::move(args[1]);
			value2 = std::move(args[2]);
			return true;
		}
		return false;
	}

public:
	CommandCollectAliases() : GenericCommand(L"collect_aliases", L"[boolean] [only valid(=1, boolean)]") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		bool changed = ProcessBoolValue(value1, GlobalPrefs->Alias);
		ProcessBoolValue(value2, GlobalPrefs->OnlyValidAlias);
		env_Print(FormatArgs("Collecting Aliases: %hs, only for valid hashes: %hs", GlobalPrefs->Alias ? "enabled" : "disabled", GlobalPrefs->OnlyValidAlias ? "yes" : "no").c_str());
		if(changed)
		{
			if(GlobalPrefs->Alias)
			{
				if(StartAliasThread())
				{
					env_Print("Alias thread has been started!");
				}
			}
			else
			{
				if(StopAliasThread())
				{
					env_Print("Alias thread has been stopped!");
				}
			}
		}		
		return true;
	}
};

class CommandLog final : public PrefBoolCommand
{
public:
	CommandLog() : PrefBoolCommand(L"log") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		bool changed = ProcessBoolValue(value, GlobalPrefs->Log);
		env_Print(FormatArgs("SAPP Logging: %hs", GlobalPrefs->Log ? "enabled" : "disabled").c_str());
		if(changed)
		{
			if(GlobalPrefs->Log)
			{
				if(StartLogThread())
				{
					env_Print("Log thread has been started!");
				}
			}
			else
			{
				if(StopLogThread())
				{
					env_Print("Log thread has been stopped!");
				}
			}
		}
		return true;
	}
};

class CommandLogRotation final : public PrefNumberCommand
{
public:
	CommandLogRotation() : PrefNumberCommand(L"log_rotation", L"[kb(0 or 1024+)]", 1024) {}
	
	bool Execute(const CommandEnviroment& env) override
	{		
		if (set)
		{
			GlobalPrefs->LogRotation = value * 1024;
		}
		if(GlobalPrefs->LogRotation == 0)
		{
			env_Print("SAPP log rotation is disabled.");
		}
		else
		{
			env_Print(FormatArgs("SAPP log rotation value: %u kb", GlobalPrefs->LogRotation / 1024).c_str());
		}
		return true;
	}
};

class CommandLogName final : public PrefStringCommand
{
public:
	CommandLogName() : PrefStringCommand(L"log_name", L"[file name]") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
		{
			log_file = FormatWArgs(L"%hs\\sapp\\%ls.log", GametypesFolder, value.c_str());
			log_name = FormatWArgs(L"%ls", value.c_str());
		}
		env_Print(FormatWArgs(L"Log File: %ls", log_file.c_str()).c_str());
		return true;
	}
};

class CommandChatConsoleEcho final : public PrefBoolCommand
{
public:
	CommandChatConsoleEcho() : PrefBoolCommand(L"chat_console_echo") {}

	bool Execute(const CommandEnviroment& env) override
	{
		ProcessBoolValue(value, GlobalPrefs->ConChatEcho);
		env_Print(FormatArgs("Chat Console Echo: %hs", GlobalPrefs->ConChatEcho ? "enabled" : "disabled").c_str());
		return true;
	}
};

class CommandAFKKick final : public PrefNumberCommand
{
public:
	CommandAFKKick() : PrefNumberCommand(L"afk_kick", L"[seconds(0 or 60-86400)]", 60, 86400) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
		{
			GlobalPrefs->AFKKick = value;
		}
		if (GlobalPrefs->AFKKick == 0)
		{
			env_Print("AFK kicking is disabled.");
		}
		else
		{
			env_Print(FormatArgs("AFK kick: %u s", GlobalPrefs->AFKKick).c_str());
		}
		return true;
	}
};

class CommandPingKick final : public PrefNumberCommand
{
public:
	CommandPingKick() : PrefNumberCommand(L"ping_kick", L"[ms(0 or 150-999)]", 150, 999) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
		{
			GlobalPrefs->PingKick = value;
		}
		if (GlobalPrefs->PingKick == 0)
		{
			env_Print("Ping kicking is disabled.");
		}
		else
		{
			env_Print(FormatArgs("Ping kick: %u ms", GlobalPrefs->PingKick).c_str());
		}
		return true;
	}
};

class CommandMapSkip final : public PrefNumberCommand
{
public:
	CommandMapSkip() : PrefNumberCommand(L"map_skip", L"[value(%, 0-100)]", 0, 100) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
		{
			GlobalPrefs->MapSkip = value;
			GlobalPrefs->OriginalMapSkip = value;
		}
		if (GlobalPrefs->OriginalMapSkip == 0)
		{
			env_Print("Map skipping is disabled.");
		}
		else
		{
			env_Print(FormatArgs("Votes to skip the map: %u%%", GlobalPrefs->OriginalMapSkip).c_str());
		}
		return true;
	}
};

class CommandSpawnProtection final : public PrefNumberCommand
{
public:
	CommandSpawnProtection() : PrefNumberCommand(L"spawn_protection", L"[seconds(0-10)]", 0, 10) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
		{
			GlobalPrefs->SpawnProtect = value * 30;
		}
		if (GlobalPrefs->SpawnProtect == 0)
		{
			env_Print("Spawn protection is disabled.");
		}
		else
		{
			env_Print(FormatArgs("Spawn protection: %u sec", GlobalPrefs->SpawnProtect / 30).c_str());
		}
		return true;
	}
};

class CommandAntiCamp final : public GenericCommand
{
	size_t time;
	float distance;
	bool set;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 1)
		{
			set = false;
			return true;
		}
		else if (args.argc == 2)
		{
			set = true;
			if (!ToNumber(args[1], time) || (time < 10 && time != 0))
				return false;
			distance = 5;
			return true;
		}
		else if (args.argc == 3)
		{
			set = true;
			if (!ToNumber(args[1], time) || (time < 10 && time != 0))
				return false;
			if (ToNumber(args[2], distance))
				return true;
		}
		return false;
	}

public:
	CommandAntiCamp() : GenericCommand(L"anticamp", L"[seconds(0 or 10+)] [world units(=5)]") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if(set)
		{
			GlobalPrefs->CampTime = time;
			GlobalPrefs->CampDist = distance;
		}
		if (GlobalPrefs->CampTime == 0)
		{
			env_Print("Anti Camp is disabled.");
		}
		else
		{
			env_Print(FormatArgs("Anti Camp is enabled, Time: %u s, Distance: %.2f u", GlobalPrefs->CampTime, GlobalPrefs->CampDist).c_str());
		}
		return true;
	}
};

class CommandAntiWarp final : public PrefNumberCommand
{
public:
	CommandAntiWarp() : PrefNumberCommand(L"antiwarp", L"[warps(0 or 3+)]", 3) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if(set)
		{
			GlobalPrefs->MaxWarps = value;
		}
		if (GlobalPrefs->MaxWarps == 0)
		{
			env_Print("Anti Warp is disabled.");
		}
		else
		{
			env_Print(FormatArgs("Anti Warp is enabled, Warps to raise event: %u", GlobalPrefs->MaxWarps).c_str());
		}
		return true;
	}
};

class CommandAimbotBan final : public GenericCommand
{
	size_t score;
	size_t type;
	size_t length;
	bool set;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 1)
		{
			set = false;
			return true;
		}
		else if (args.argc == 2)
		{
			set = true;
			if (!ToNumber(args[1], score) || ((score < 5000 || score > 20000) && score != 0))
				return false;
			type = 1;
			length = 1440;
			return true;
		}
		else if (args.argc == 3)
		{
			set = true;
			if (!ToNumber(args[1], score) || ((score < 5000 || score > 20000) && score != 0))
				return false;
			if (!ToNumber(args[2], type) || type > 4)
				return false;
			length = 1440;
			return true;
		}
		else if (args.argc == 4)
		{
			set = true;
			if (!ToNumber(args[1], score) || ((score < 5000 || score > 20000) && score != 0))
				return false;
			if (!ToNumber(args[2], type) || type > 4)
				return false;
			if (ToNumber(args[3], length))
				return true;
		}
		return false;
	}

public:
	CommandAimbotBan() : GenericCommand(L"aimbot_ban", L"[score(0 or 5000-20000)] [ban type(=1, 0-4)] [ban length in minutes(=1440)]") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if (GlobalPrefs->AntiCheat)
		{
			env_Print("Anticheat overrides this function and it's always disabled.");
		}
		else
		{
			if (set)
			{
				if (!GlobalPrefs->AimbotScanScoreLimit && score)
				{				
					LoadAimbotScan();
				}
				GlobalPrefs->AimbotScanScoreLimit = score;
				GlobalPrefs->AimbotScanBanType = type;
				GlobalPrefs->AimbotScanBanLength = length;
			}
			if (!GlobalPrefs->AimbotScanScoreLimit)
			{
				env_Print("Aimbot banning is disabled.");
			}
			else
			{
				env_Print(FormatArgs("Scores to ban aimbotter: %u", GlobalPrefs->AimbotScanScoreLimit).c_str());
				const char* ban_type;
				switch (GlobalPrefs->AimbotScanBanType)
				{
				case 0:
					ban_type = "CD-Key Ban";
					break;
				case 1:
					ban_type = "IP Ban";
					break;
				case 2:
					ban_type = "CD-Key Ban & IP Ban";
					break;
				case 3:
					ban_type = "Kick Only";
					break;
				default:
					ban_type = "Nothing";
				}
				env_Print(FormatArgs("Ban Type: %hs, Length: %hs", ban_type, GlobalPrefs->AimbotScanBanLength == 0 ? "Infinitive" : FormatArgs("%u minutes", GlobalPrefs->AimbotScanBanLength).c_str()).c_str()); // mdr
			}
		}
		return true;
	}
};

class CommandMultiTeamVehicles final : public PrefBoolCommand
{
public:
	CommandMultiTeamVehicles() : PrefBoolCommand(L"mtv") {}

	bool Execute(const CommandEnviroment& env) override
	{
		bool changed = ProcessBoolValue(value, GlobalPrefs->MTV);
		env_Print(FormatArgs("Multi Team Vehicles: %hs", GlobalPrefs->MTV ? "enabled" : "disabled").c_str());
		if (changed)
		{		
			if (GlobalPrefs->MTV)
			{
				GlobalPrefs->Flags |= 32;
				InstallMTV();
			}
			else
			{
				GlobalPrefs->Flags &= ~32;
				UninstallMTV();
			}
			NetworkEvent<DataSync<bool>>(NE_DataSync_MTV, GlobalPrefs->MTV);
			FormatQueryString();
		}
		return true;
	}
};

class CommandDisableTimerOffsets final : public PrefBoolCommand
{
public:
	CommandDisableTimerOffsets() : PrefBoolCommand(L"disable_timer_offsets") {}

	bool Execute(const CommandEnviroment& env) override
	{
		bool changed = ProcessBoolValue(value, GlobalPrefs->DisableTimerOffsets);
		env_Print(FormatArgs("Disabling Timer Offsets: %hs", GlobalPrefs->DisableTimerOffsets ? "enabled" : "disabled").c_str());
		if (changed)
		{
			if (GlobalPrefs->DisableTimerOffsets)
				InstallObjectRespawnPatch();
			else
				UninstallObjectRespawnPPatch();
		}
		return true;
	}
};

class CommandSAPPConsole final : public PrefBoolCommand
{
public:
	CommandSAPPConsole() : PrefBoolCommand(L"sapp_console") {}

	bool Execute(const CommandEnviroment& env) override
	{
		bool changed = ProcessBoolValue(value, GlobalPrefs->SappConsole);
		env_Print(FormatArgs("SAPP Console: %hs", GlobalPrefs->SappConsole ? "enabled" : "disabled").c_str());
		if (changed)
		{
			if (GlobalPrefs->SappConsole)
				InstallConsoleSpamPatch();
			else
				UninstallConsoleSpamPatch();
		}
		return true;
	}
};

class CommandConsoleInput final : public PrefBoolCommand
{
public:
	CommandConsoleInput() : PrefBoolCommand(L"console_input") {}

	bool Execute(const CommandEnviroment& env) override
	{
		bool changed = ProcessBoolValue(value, GlobalPrefs->ConsoleInput);
		env_Print(FormatArgs("Console Input: %hs", GlobalPrefs->ConsoleInput ? "enabled" : "disabled").c_str());
		if (changed)
		{
			if (GlobalPrefs->ConsoleInput)
				UninstallConsoleInputPatch();
			else
				InstallConsoleInputPatch();
		}
		return true;
	}
};

class CommandCustomSleep final : public PrefNumberCommand
{
public:
	CommandCustomSleep() : PrefNumberCommand(L"custom_sleep", L"[ms(0-33)]", 0, 33) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if(set)
		{		
			if (value == 0)
				value = 33;
			GlobalPrefs->CustomSleep = value;
			GlobalPrefs->OriginalCustomSleep = value;
			SetHaloTimer();
		}
		env_Print(FormatArgs("Halo now sleeps %u ms every cycle.", GlobalPrefs->OriginalCustomSleep).c_str());
		return true;
	}
};

class CommandUnlockConsoleLog final : public PrefBoolCommand
{
public:
	CommandUnlockConsoleLog() : PrefBoolCommand(L"unlock_console_log") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (UnlockConsole != nullptr)
		{
			bool yes = false;
			ProcessBoolValue(value, yes);
			if(yes)
			{
				env_Print("Unlocked");
				UnlockConsole[0] = UnlockConsole[1] = 0x0101010101010101;
			}
			else
			{
				env_Print("Locked");
				UnlockConsole[0] = UnlockConsole[1] = 0;
			}
		}
		return true;
	}
};

class CommandSetConsoleTextAttribute final : public PrefNumberCommand
{
public:
	CommandSetConsoleTextAttribute() : PrefNumberCommand(L"set_ccolor", L"[color]") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
		{
			WORD attrib = static_cast<WORD>(value);
			SetConsoleTextAttribute(hConsole, attrib);
			GlobalPrefs->ConAttrib = attrib;
			env_Print("The new color has been set.");
		}
		return true;
	}
};

class CommandObjectsyncCleanup final : public PrefBoolCommand
{
public:
	CommandObjectsyncCleanup() : PrefBoolCommand(L"object_sync_cleanup") {}

	bool Execute(const CommandEnviroment& env) override
	{
		bool changed = ProcessBoolValue(value, GlobalPrefs->ObjectSyncCleanup);
		env_Print(FormatArgs("Object Sync Cleanup: %hs", GlobalPrefs->ObjectSyncCleanup ? "enabled" : "disabled").c_str());
		if (changed)
		{
			if (GlobalPrefs->ObjectSyncCleanup)
			{
				ActivateObjectSyncCleanupHandler();
			}
			else
			{
				DeactivateObjectSyncCleanupHandler();
			}
		}
		return true;
	}
};

class CommandAntiCheat final : public PrefStringCommand
{
public:
	CommandAntiCheat() : PrefStringCommand(L"anticheat", L"[boolean]") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
		{
			bool ac = true;
			ProcessBoolValue(value, ac);
			if (ac)
			{
				if (!Initalised)
				{
					GlobalPrefs->AntiCheat = true;
					GlobalPrefs->SJLevel = 5;
					GlobalPrefs->Flags |= 18;
					GlobalPrefs->AimbotScanScoreLimit = 0;
					env_Print("Anticheat enabled!");
				}
				else
				{
					env_Print("You only can enable this feature from the init.txt!");
				}
			}
			else if (Initalised)
			{
				env_Print("This feature can't be disabled once it's enabled!");
			}
		}
		else
		{
			env_Print(FormatArgs("Anticheat: %hs", GlobalPrefs->AntiCheat ? "enabled" : "disabled").c_str());
		}					
		return true;
	}
};

class CommandRemoteConsole final : public PrefBoolCommand
{
public:
	CommandRemoteConsole() : PrefBoolCommand(L"remote_console") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (ProcessBoolValue(value, GlobalPrefs->RemoteConsole))
		{
			if (GlobalPrefs->RemoteConsole)
			{
				if (GlobalRemoteConsole.StartRemoteConsole())
				{
					env_Print("Remote Console has been started!");
				}
				else
				{
					GlobalPrefs->RemoteConsole = false;
					env_Print("Failed to start Remote Console!");
				}
			}
			else
			{
				if (GlobalRemoteConsole.StopRemoteConsole())
				{
					env_Print("Remote Console has been stopped!");
				}
				else
				{
					env_Print("Failed to stop Remote Console!");
				}
			}
		}
		else
		{
			if (GlobalPrefs->RemoteConsole)
			{
				env_Print("Remote Console is enabled!");
			}
			else
			{
				env_Print("Remote Console is disabled!");
			}
		}
		return true;
	}
};

class CommandRemoteConsolePort final : public PrefNumberCommand
{
public:
	CommandRemoteConsolePort() : PrefNumberCommand(L"remote_console_port", L"[port(1-65535)]", 1, 65535) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
			RemotePort = static_cast<unsigned short>(value);
		env_Print(FormatArgs("Remote Console port: %hu", RemotePort).c_str());
		return true;
	}
};

class CommandRemoteConsoleList final : public GenericCommand
{
public:
	CommandRemoteConsoleList() : GenericCommand(L"remote_console_list") {}

	bool Execute(const CommandEnviroment& env) override
	{
		env.StreamPrint(GlobalRemoteConsole.StreamOutRemoteClients());
		return true;
	}
};

class CommandV final : public PrefStringCommand
{
public:
	CommandV() : PrefStringCommand(L"v", L"[version string]") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
		{
			if (value.length() < 16)
			{
				std::string v = WToStr(value);
				DWORD oldProtect;
				VirtualProtect((LPVOID)HaloVersionStr, 16, PAGE_READWRITE, &oldProtect);
				memcpy(HaloVersionStr, v.c_str(), v.length() + 1);
				VirtualProtect((LPVOID)HaloVersionStr, 16, oldProtect, &oldProtect);
			}
			else
			{
				env_Print("Error: Halo Version String can't be longer than 15 characters!");
			}
		}
		env_Print(FormatArgs("Halo Version: %hs", HaloVersionStr).c_str());
		return true;
	}
};

class CommandDNS final : public PrefStringCommand
{
public:
	CommandDNS() : PrefStringCommand(L"dns", L"[dns string]") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
		{
			if (value.length() < 24)
			{
				std::string dns = WToStr(value);
				DWORD oldProtect;
				VirtualProtect((LPVOID)HaloDNSStr, 24, PAGE_READWRITE, &oldProtect);
				memcpy(HaloDNSStr, dns.c_str(), dns.length() + 1);
				VirtualProtect((LPVOID)HaloDNSStr, 24, oldProtect, &oldProtect);
				SetDNS(dns.c_str());
			}
			else
			{
				env_Print("Error: The Master Server DNS can't be longer than 23 characters!");
			}
		}
		env_Print(FormatArgs("Master Server DNS: %hs", HaloDNSStr).c_str());
		return true;
	}
};