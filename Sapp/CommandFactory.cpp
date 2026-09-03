#include "CommandFactory.h"

#include "CommandAdminV1.h"
#include "CommandAdminV2.h"
#include "CommandAreas.h"
#include "CommandBalanceTeams.h"
#include "CommandBan.h"
#include "CommandBlockObject.h"
#include "CommandDisableObject.h"
#include "CommandEvent.h"
#include "CommandHalo.h"
#include "CommandIpBan.h"
#include "CommandLists.h"
#include "CommandLoadMap.h"
#include "CommandLocations.h"
#include "CommandLua.h"
#include "CommandMapcycle.h"
#include "CommandMapVote.h"
#include "CommandMisc.h"
#include "CommandNetworkEvents.h"
#include "CommandPrefs.h"
#include "CommandQuery.h"
#include "CommandReload.h"
#include "CommandReport.h"
#include "CommandCustom.h"
#include "CommandSpawnObject.h"
#include "CommandStats.h"
#include "CommandTeamUp.h"
#include "CommandTextban.h"
#include "CommandVar.h"
#include "CommandVehicleDelete.h"

#include "CommandAlias.h"
#include "CommandAmmo.h"
#include "CommandAssignWeapon.h"
#include "CommandBan.h"
#include "CommandBoost.h"
#include "CommandChangeColor.h"
#include "CommandDropWeapon.h"
#include "CommandGod.h"
#include "CommandInvisible.h"
#include "CommandIp.h"
#include "CommandIpBan.h"
#include "CommandKick.h"
#include "CommandKill.h"
#include "CommandLag.h"
#include "CommandPlayerDescription.h"
#include "CommandRconPrint.h"
#include "CommandSay.h"
#include "CommandSetAdmin.h"
#include "CommandSpeed.h"
#include "CommandStats.h"
#include "CommandSwitchTeam.h"
#include "CommandTeleport.h"
#include "CommandTextban.h"
#include "CommandVehicleDelete.h"
#include "CommandVehicleEnter.h"
#include "CommandVehicleExit.h"
#include "CommandWeaponDelete.h"

void CommandFactory::AddCommand(std::unique_ptr<BaseCommand>&& command)
{
	all_command.emplace(command->name, std::move(command));
}

bool CommandFactory::RemoveCommand(const std::wstring& name)
{
	auto& it = all_command.find(name);
	if (it != all_command.end())
	{
		if (it->second->type == CommandType::CUSTOM)
		{
			all_command.erase(it);
			return true;
		}
	}
	return false;
}

void CommandFactory::LoadCommands()
{
	// Generic commands
	AddCommand(std::make_unique<CommandVarAdd>());
	AddCommand(std::make_unique<CommandVarDel>());
	AddCommand(std::make_unique<CommandVarSet>());
	AddCommand(std::make_unique<CommandVarConvert>());
	AddCommand(std::make_unique<CommandVarList>());
	AddCommand(std::make_unique<CommandLuaLoad>());
	AddCommand(std::make_unique<CommandLuaUnload>());
	AddCommand(std::make_unique<CommandLuaList>());
	AddCommand(std::make_unique<CommandLuaCall>());
	AddCommand(std::make_unique<CommandLuaAPIV>());
	AddCommand(std::make_unique<CommandPL>());
	AddCommand(std::make_unique<CommandStats>());
	AddCommand(std::make_unique<CommandSv_Stats>());
	AddCommand(std::make_unique<CommandAbout>());
	AddCommand(std::make_unique<CommandInfo>());
	AddCommand(std::make_unique<CommandUptime>());
	AddCommand(std::make_unique<CommandCPU>());
	AddCommand(std::make_unique<CommandAFK>());
	AddCommand(std::make_unique<CommandSTFU>());
	AddCommand(std::make_unique<CommandUnSTFU>());
	AddCommand(std::make_unique<CommandAdminV1Add>());
	AddCommand(std::make_unique<CommandAdminV1Del>());
	AddCommand(std::make_unique<CommandAdminV1Level>());
	AddCommand(std::make_unique<CommandAdminV1s>());
	AddCommand(std::make_unique<CommandAdminV2Add>());
	AddCommand(std::make_unique<CommandAdminV2AddManually>());
	AddCommand(std::make_unique<CommandAdminV2ChangePw>());
	AddCommand(std::make_unique<CommandAdminV2ChangeLevel>());
	AddCommand(std::make_unique<CommandAdminV2Del>());
	AddCommand(std::make_unique<CommandAdminV2List>());
	AddCommand(std::make_unique<CommandChangePassword>());
	AddCommand(std::make_unique<CommandAdminAddSameLevel>());
	AddCommand(std::make_unique<CommandAdminDelSameLevel>());
	AddCommand(std::make_unique<CommandEvents>());
	AddCommand(std::make_unique<CommandEventDel>());
	AddCommand(std::make_unique<CommandCustomEvent>());
	AddCommand(std::make_unique<CommandCustomAdd>());
	AddCommand(std::make_unique<CommandCustomDel>());
	AddCommand(std::make_unique<CommandAFKs>());
	AddCommand(std::make_unique<CommandSkips>());
	AddCommand(std::make_unique<CommandSpawnObject>());
	AddCommand(std::make_unique<CommandVehicleDeleteAll>());
	AddCommand(std::make_unique<CommandSAPPMapcycle>());
	AddCommand(std::make_unique<CommandMapcycle>());
	AddCommand(std::make_unique<CommandMapcycleAdd>());
	AddCommand(std::make_unique<CommandMapcycleDel>());
	AddCommand(std::make_unique<CommandMapcycleBegin>());
	AddCommand(std::make_unique<CommandMapNext>());
	AddCommand(std::make_unique<CommandMapPrev>());
	AddCommand(std::make_unique<CommandMapSpec>());
	AddCommand(std::make_unique<CommandWhatMapNext>());
	AddCommand(std::make_unique<CommandMapVote>());
	AddCommand(std::make_unique<CommandMapVotes>());
	AddCommand(std::make_unique<CommandMapVoteAdd>());
	AddCommand(std::make_unique<CommandMapVoteDel>());
	AddCommand(std::make_unique<CommandMapvoteBegin>());
	AddCommand(std::make_unique<CommandLoadMap>());
	AddCommand(std::make_unique<CommandDownloadMap>());
	AddCommand(std::make_unique<CommandQueryMap>());
	AddCommand(std::make_unique<CommandReport>());
	AddCommand(std::make_unique<CommandLogin>());
	AddCommand(std::make_unique<CommandCMDStart1>());
	AddCommand(std::make_unique<CommandCMDStart2>());
	AddCommand(std::make_unique<CommandSayPrefix>());
	AddCommand(std::make_unique<CommandMsgPrefix>());
	AddCommand(std::make_unique<CommandAdminPrefix>());
	AddCommand(std::make_unique<CommandLua>());
	AddCommand(std::make_unique<CommandAntiSpam>());
	AddCommand(std::make_unique<CommandAntiCaps>());
	AddCommand(std::make_unique<CommandBlockTC>());
	AddCommand(std::make_unique<CommandHideAdmin>());
	AddCommand(std::make_unique<CommandFullIPBan>());
	AddCommand(std::make_unique<CommandAntiHaloFP>());
	AddCommand(std::make_unique<CommandAntiGlitch>());
	AddCommand(std::make_unique<CommandAntiLagSpawn>());
	AddCommand(std::make_unique<CommandDebugStrings>());
	AddCommand(std::make_unique<CommandSaveScores>());
	AddCommand(std::make_unique<CommandSaveRespawnTime>());
	AddCommand(std::make_unique<CommandZombies>());
	AddCommand(std::make_unique<CommandSJLevel>());
	AddCommand(std::make_unique<CommandScrimMode>());
	AddCommand(std::make_unique<CommandDisableBacktap>());
	AddCommand(std::make_unique<CommandNoLead>());
	AddCommand(std::make_unique<CommandLead>());
	AddCommand(std::make_unique<CommandCustomLead>());
	AddCommand(std::make_unique<CommandMaxVotes>());
	AddCommand(std::make_unique<CommandMaxIdle>());
	AddCommand(std::make_unique<CommandPacketLimit>());
	AddCommand(std::make_unique<CommandAutoUpdate>());
	AddCommand(std::make_unique<CommandNetworkThread>());
	AddCommand(std::make_unique<CommandCollectAliases>());
	AddCommand(std::make_unique<CommandLog>());
	AddCommand(std::make_unique<CommandLogRotation>());
	AddCommand(std::make_unique<CommandLogName>());
	AddCommand(std::make_unique<CommandChatConsoleEcho>());
	AddCommand(std::make_unique<CommandAFKKick>());
	AddCommand(std::make_unique<CommandPingKick>());
	AddCommand(std::make_unique<CommandMapSkip>());
	AddCommand(std::make_unique<CommandSpawnProtection>());
	AddCommand(std::make_unique<CommandAntiCamp>());
	AddCommand(std::make_unique<CommandAntiWarp>());
	AddCommand(std::make_unique<CommandAdminBan>());
	AddCommand(std::make_unique<CommandAimbotBan>());
	AddCommand(std::make_unique<CommandAimbotScores>());
	AddCommand(std::make_unique<CommandBalanceTeams>());
	AddCommand(std::make_unique<CommandTeamUp>());
	AddCommand(std::make_unique<CommandTell>());
	AddCommand(std::make_unique<CommandSappRcon>());
	AddCommand(std::make_unique<CommandMap>());
	AddCommand(std::make_unique<CommandMapList>());
#ifndef HPC
	AddCommand(std::make_unique<CommandMotd>());
#endif
	AddCommand(std::make_unique<CommandReloadGametypes>());
	AddCommand(std::make_unique<CommandTeamScore>());
	AddCommand(std::make_unique<CommandScoreLimit>());
	AddCommand(std::make_unique<CommandTimelimit>());
	AddCommand(std::make_unique<CommandHillTimer>());
	AddCommand(std::make_unique<CommandDisableAllObjects>());
	AddCommand(std::make_unique<CommandDisableAllVehicles>());
	AddCommand(std::make_unique<CommandDisableObject>());
	AddCommand(std::make_unique<CommandEnableObject>());
	AddCommand(std::make_unique<CommandDisabledObjects>());
	AddCommand(std::make_unique<CommandMultiTeamVehicles>());
	AddCommand(std::make_unique<CommandDisableTimerOffsets>());
	AddCommand(std::make_unique<CommandAntiCheat>());
	AddCommand(std::make_unique<CommandRemoteConsole>());
	AddCommand(std::make_unique<CommandRemoteConsolePort>());
	AddCommand(std::make_unique<CommandRemoteConsoleList>());
	AddCommand(std::make_unique<CommandV>());
	AddCommand(std::make_unique<CommandDNS>());
	AddCommand(std::make_unique<CommandFiles>());
	AddCommand(std::make_unique<CommandText>());
	AddCommand(std::make_unique<CommandLogNote>());
	AddCommand(std::make_unique<CommandSAPPConsole>());
	AddCommand(std::make_unique<CommandConsoleInput>());
	AddCommand(std::make_unique<CommandCustomSleep>());
#ifndef HPC
	AddCommand(std::make_unique<CommandUnlockConsoleLog>());
#endif
	AddCommand(std::make_unique<CommandSetConsoleTextAttribute>());
	AddCommand(std::make_unique<CommandObjectsyncCleanup>());
	AddCommand(std::make_unique<CommandQueryAdd>());
	AddCommand(std::make_unique<CommandQueryDel>());
	AddCommand(std::make_unique<CommandQueryList>());
	AddCommand(std::make_unique<CommandBeep>());
	AddCommand(std::make_unique<CommandReload>());
	AddCommand(std::make_unique<CommandyEyE>()); // =)
	AddCommand(std::make_unique<CommandAyyLmao>()); // ayy lmao
	AddCommand(std::make_unique<CommandGameSpeed>());
	AddCommand(std::make_unique<CommandGravity>());
	AddCommand(std::make_unique<CommandUsage>());
	AddCommand(std::make_unique<CommandSet>());
	AddCommand(std::make_unique<CommandList>());

	// Connected to Player Commands
	AddCommand(std::make_unique<CommandLocAdd>());
	AddCommand(std::make_unique<CommandLocDel>());
	AddCommand(std::make_unique<CommandLocList>());
	AddCommand(std::make_unique<CommandLocListAll>());
	AddCommand(std::make_unique<CommandAreaAddSphere>());
	AddCommand(std::make_unique<CommandAreaAddCuboid>());
	AddCommand(std::make_unique<CommandAreaDel>());
	AddCommand(std::make_unique<CommandAreaList>());
	AddCommand(std::make_unique<CommandAreaListAll>());
	AddCommand(std::make_unique<CommandBans>());
	AddCommand(std::make_unique<CommandUnBan>());
	AddCommand(std::make_unique<CommandTextBans>());
	AddCommand(std::make_unique<CommandTextUnBan>());
	AddCommand(std::make_unique<CommandUnMute>());
	AddCommand(std::make_unique<CommandMutes>());
	AddCommand(std::make_unique<CommandIPRangeBan>());
	AddCommand(std::make_unique<CommandIPBans>());
	AddCommand(std::make_unique<CommandIPUnBan>());
	AddCommand(std::make_unique<CommandRefreshIPbans>());
	AddCommand(std::make_unique<CommandBlockObject>());
	AddCommand(std::make_unique<CommandUnblockObject>());

	// Player commands
	AddCommand(std::make_unique<CommandSay>());
	AddCommand(std::make_unique<CommandRconPrint>());
	AddCommand(std::make_unique<CommandAlias>());
	AddCommand(std::make_unique<CommandSetAdmin>());
	AddCommand(std::make_unique<CommandSpeed>());
	AddCommand(std::make_unique<CommandTeleport>());
	AddCommand(std::make_unique<CommandMove>());
	AddCommand(std::make_unique<CommandTeleportToPlayer>());
	AddCommand(std::make_unique<CommandBoost>());
	AddCommand(std::make_unique<CommandVehicleExit>());
	AddCommand(std::make_unique<CommandVehicleEnter>());
	AddCommand(std::make_unique<CommandVehicleDelete>());
	AddCommand(std::make_unique<CommandAssignWeapon>());
	AddCommand(std::make_unique<CommandWeaponDelete>());
	AddCommand(std::make_unique<CommandDropWeapon>());
	AddCommand(std::make_unique<CommandSwitchTeam>());
	AddCommand(std::make_unique<CommandKill>());
	AddCommand(std::make_unique<CommandKick>());
	AddCommand(std::make_unique<CommandBan>());
	AddCommand(std::make_unique<CommandNades>());
	AddCommand(std::make_unique<CommandAmmo>());
	AddCommand(std::make_unique<CommandMag>());
	AddCommand(std::make_unique<CommandBattery>());
	AddCommand(std::make_unique<CommandInvisible>());
	AddCommand(std::make_unique<CommandScore>());
	AddCommand(std::make_unique<CommandKills>());
	AddCommand(std::make_unique<CommandAssists>());
	AddCommand(std::make_unique<CommandDeaths>());
	AddCommand(std::make_unique<CommandHealth>());
	AddCommand(std::make_unique<CommandShield>());
	AddCommand(std::make_unique<CommandKDR>());
	AddCommand(std::make_unique<CommandGod>());
	AddCommand(std::make_unique<CommandUnGod>());
	AddCommand(std::make_unique<CommandLag>());
	AddCommand(std::make_unique<CommandUnlag>());
	AddCommand(std::make_unique<CommandChangeColor>());
	AddCommand(std::make_unique<CommandPlayerDescription>());
	AddCommand(std::make_unique<CommandCoord>());
	AddCommand(std::make_unique<CommandInf>());
	AddCommand(std::make_unique<CommandIp>());
	AddCommand(std::make_unique<CommandTextBan>());
	AddCommand(std::make_unique<CommandMute>());
	AddCommand(std::make_unique<CommandIPBan>());
	AddCommand(std::make_unique<CommandBlockAllObjects>());
	AddCommand(std::make_unique<CommandBlockAllVehicles>());
}

void CommandFactory::ClearCommands()
{
	all_command.clear();
}

int CommandFactory::ExecuteCommand(ArgContainer& args, const CommandEnviroment& env, bool block)
{
	auto& it = all_command.find(args[0]);
	if (it != all_command.end())
	{
		auto& command = it->second;

		if (block && command->unalterable == Unalterable::no)
			return 1;

		if (command->level > env.PrivilegeLevel)
		{
			env.Print("You do not have the rights to execute this command!");
			return -1;
		}
		else
		{
			if (GlobalPrefs->ScrimMode && command->naughty == Naughty::yes)
			{
				env.Print("Scrim Mode is ON! You can't execute this command now!");
				return 0;
			}
			else
			{
				return command->ExecuteCommand(args, env);
			}
		}
	}
	else
	{
		return -2;
	}
}

bool CommandFactory::Exists(const std::wstring& name)
{
	auto& it = all_command.find(name);
	return it != all_command.end();
}

bool CommandFactory::ChangeName(const std::wstring& name, const std::wstring& new_name)
{
	auto& it = all_command.find(name);
	if (it != all_command.end() && it->second->unalterable == Unalterable::no)
	{
		auto tmp = std::move(it->second);
		all_command.erase(it);
		tmp->name = new_name;
		AddCommand(std::move(tmp));
		return true;
	}
	return false;
}

bool CommandFactory::ChangeLevel(const std::wstring& name, int new_level)
{
	auto& it = all_command.find(name);
	if (it != all_command.end() && it->second->unalterable == Unalterable::no)
	{
		it->second->level = new_level;
		return true;
	}
	return false;
}

bool CommandFactory::PrintUsage(const std::wstring& name, const CommandEnviroment& env)
{
	auto& it = all_command.find(name);
	if (it != all_command.end())
	{
		it->second->PrintUsage(env);
		return true;
	}
	return false;
}

void CommandFactory::ListCommands(CommandType type, const CommandEnviroment& env)
{
	size_t n = 0;

	for (auto& it : all_command)
	{
		if (type == CommandType::BASE || type == it.second->type)
		{
			if (it.second->level <= env.PrivilegeLevel)
			{
				it.second->PrintUsage(env, true);
				n++;
			}
		}
	}

	if (n == 0)
		env.Print("There are no commands available for your level.");
}

CommandFactory GlobalCommandFactory;