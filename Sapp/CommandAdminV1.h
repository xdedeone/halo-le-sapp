#pragma once

#include "Addresses.h"
#include "GenericCommand.h"
#include "HaloUtilities.h"
#include "OnlineAdmins.h"
#include "PlayerCommand.h"
#include "Prefs.h"
#include "TextUtilities.h"
#include "UserList.h"

class CommandAdminV1Add final : public PlayerCommand
{
	int admin_level;
	std::vector <std::string> ip_ranges;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc >= 3)
		{
			ip_ranges.clear();

			if (ToNumber(args[2], admin_level) && admin_level >= 0 && admin_level <= 4)
			{
				for (size_t i = 3; i < args.argc; i++)
				{
					ip_ranges.emplace_back(WToStr(args[i]));
				}
				return true;
			}
		}
		return false;
	}

public:
	CommandAdminV1Add() : PlayerCommand(L"adminadd", L"<level(0-4)> [ip-ranges(<ip>/<mask>)]...", 0) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		size_t admin_id;

		if(GlobalPrefs->AdminAddSameLevel == 0)
		{
			if(env.PrivilegeLevel < 4)
			{
				env_Print("Error: You don't have the right to add another admin!");
				return true;
			}
		}
		else if(GlobalPrefs->AdminAddSameLevel == 1)
		{
			if(env.PrivilegeLevel <= admin_level)
			{
				env_Print("Error: You don't have the right to add another admin with higher or equal your level than yours!");
				return true;
			}
		}
		else
		{
			if(env.PrivilegeLevel < admin_level)
			{
				env_Print("Error: You don't have the right to add another admin with higher lever than yours!");
				return true;
			}
		}

		GlobalUserList.LoadUsersFile();

		if(GlobalUserList.AddUser(PlayerIndex, admin_level, admin_id))
		{
			env_Print(FormatWArgs(L"Successfully added player %ls [ %hs ] to the admin list.", PlayerTable[PlayerIndex].name, GetPlayerHash(IndexToNumber(PlayerIndex)).c_str()).c_str());
			Login(PlayerIndex, admin_level);
		}
		else 
		{
			env_Print(FormatWArgs(L"Failed to add player %ls to the admin list, probably he/she is already an admin.", PlayerTable[PlayerIndex].name).c_str());
			return true;
		}

		for (size_t i = 0; i < ip_ranges.size(); i++)
		{
			if (GlobalUserList.UserAddIP(admin_id, ip_ranges[i]))
			{
				env_Print(FormatWArgs(L"Added '%hs' to IP ranges of admin %ls.", ip_ranges[i].c_str(), PlayerTable[PlayerIndex].name).c_str());
			}
			else
			{
				env_Print(FormatWArgs(L"Invalid IP range '%hs', can not add it for admin %ls.", ip_ranges[i].c_str(), PlayerTable[PlayerIndex].name).c_str());
			}
		}

		GlobalUserList.SaveUsersFile();

		return true;
	}
};


class CommandAdminV1Del final : public GenericCommand
{
	size_t admin_id;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			if (ToNumber(args[1], admin_id))
				return true;
		}
		return false;
	}

public:
	CommandAdminV1Del() : GenericCommand(L"admindel", L"<admin id>", 0) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		int AdminLvL = GlobalUserList.UserGetLevel(admin_id);

		if(GlobalPrefs->AdminDelSameLevel == 0)
		{
			if(env.PrivilegeLevel < 4)
			{
				env_Print("Error: You don't have the right to delete another admin!");
				return true;
			}
		}
		else if(GlobalPrefs->AdminDelSameLevel == 1)
		{
			if(env.PrivilegeLevel <= AdminLvL)
			{
				env_Print("Error: You don't have the right to delete another admin with higher or equal your level than yours!");
				return true;
			}
		}
		else
		{
			if(env.PrivilegeLevel < AdminLvL)
			{
				env_Print("Error: You don't have the right to delete another admin with higher lever than yours!");
				return true;
			}
		}

		GlobalUserList.LoadUsersFile();

		if (GlobalUserList.RemoveUser(admin_id))
		{
			env_Print(FormatArgs("Admin with ID '%u' has been successfully removed.", admin_id).c_str());
			GlobalUserList.SaveUsersFile();
		}
		else
		{
			env_Print(FormatArgs("Admin does not exist with ID '%u'.", admin_id).c_str());
		}

		return true;
	}
};


class CommandAdminV1Level final : public GenericCommand
{
	size_t admin_id;
	int admin_level;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			if (!ToNumber(args[1], admin_id))
				return false;

			if (ToNumber(args[2], admin_level) && admin_level >= 0 && admin_level <= 4)
				return true;
		}
		return false;
	}

public:
	CommandAdminV1Level() : GenericCommand(L"adminlevel", L"<admin id> <new admin level(0-4)>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		GlobalUserList.LoadUsersFile();

		if (GlobalUserList.UserChangeLevel(admin_id, admin_level))
		{
			env_Print(FormatArgs("Successfully changed the admin's privilege level to %d.", admin_level).c_str());
			GlobalUserList.SaveUsersFile();
		}
		else
		{
			env_Print(FormatArgs("Admin does not exist with ID '%u'.", admin_id).c_str());
		}

		return true;
	}
};

class CommandAdminV1s final : public GenericCommand
{
public:
	CommandAdminV1s() : GenericCommand(L"admins") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		env.StreamPrint(GlobalUserList.StreamOut());
		return true;
	}
};