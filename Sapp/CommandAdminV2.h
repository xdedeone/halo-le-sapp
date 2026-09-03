#pragma once

#include "Addresses.h"
#include "Admins.h"
#include "GenericCommand.h"
#include "MiscellaneousFunctions.h"
#include "OnlineAdmins.h"
#include "TextUtilities.h"

class CommandAdminV2Add final : public PlayerCommand
{
	std::string password;
	int admin_level;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 4)
		{
			password = WToStr(args[2]);
			if (ToNumber(args[3], admin_level) && admin_level >= 0 && admin_level <= 4)
				return true;
		}
		return false;
	}

public:
	CommandAdminV2Add() : PlayerCommand(L"admin_add", L"<password> <admin level(0-4)>") {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		unsigned char pwmd5[16];
		MD5_Hash(password.c_str(), password.length(), pwmd5);

		std::string new_password_hash = ConvertToMD5Hash(pwmd5);

		GlobalAdminList.LoadAdmins();

		if (GlobalAdminList.AddAdmin(PlayerTable[PlayerIndex].name, move(new_password_hash), admin_level))
		{
			env_Print(FormatWArgs(L"Successfully added player %ls to admin list.", PlayerTable[PlayerIndex].name).c_str());
			GlobalAdminList.SaveAdmins();
			Login(PlayerIndex, admin_level);
		}
		else
		{
			env_Print(FormatWArgs(L"Failed to add player %ls to the admin list, probably he/she is already admin.", PlayerTable[PlayerIndex].name).c_str());
		}

		return true;
	}
};

class CommandAdminV2AddManually final : public GenericCommand
{
	std::wstring username;
	std::string password;
	int admin_level;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 4)
		{
			username = std::move(args[1]);
			password = WToStr(args[2]);
			if (ToNumber(args[3], admin_level) && admin_level >= 0 && admin_level <= 4)
				return true;
		}
		return false;
	}

public:
	CommandAdminV2AddManually() : GenericCommand(L"admin_add_manually", L"<username(1-11)> <password> <admin level(0-4)>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		unsigned char pwmd5[16];
		MD5_Hash(password.c_str(), password.length(), pwmd5);

		std::string password_hash = ConvertToMD5Hash(pwmd5);

		GlobalAdminList.LoadAdmins();

		if (GlobalAdminList.AddAdmin(std::wstring(username), move(password_hash), admin_level))
		{
			env_Print(FormatWArgs(L"Successfully added player %ls to admin list.", username.c_str()).c_str());
			GlobalAdminList.SaveAdmins();
		}
		else
		{
			env_Print(FormatWArgs(L"Failed to add player %ls to the admin list, probably he/she is already an admin.", username.c_str()).c_str());
		}

		return true;
	}
};

class CommandAdminV2ChangePw final : public GenericCommand
{
	size_t admin_id;
	std::string password;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			if (!ToNumber(args[1], admin_id))
				return false;

			password = WToStr(args[2]);
			return true;
		}
		return false;
	}

public:
	CommandAdminV2ChangePw() : GenericCommand(L"admin_change_pw", L"<admin id> <new password>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		unsigned char pwmd5[16];
		MD5_Hash(password.c_str(), password.length(), pwmd5);

		std::string password_hash = ConvertToMD5Hash(pwmd5);

		GlobalAdminList.LoadAdmins();

		if (GlobalAdminList.ChangePassword(admin_id, password_hash))
		{
			env_Print("Successfully changed the admin's password.");
			GlobalAdminList.SaveAdmins();
		}
		else
		{
			env_Print(FormatArgs("Admin does not exist with ID '%u'.", admin_id).c_str());
		}

		return true;
	}
};

class CommandAdminV2ChangeLevel final : public GenericCommand
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
	CommandAdminV2ChangeLevel() : GenericCommand(L"admin_change_level", L"<admin id> <admin level(0-4)>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		GlobalAdminList.LoadAdmins();

		if (GlobalAdminList.ChangeLevel(admin_id, admin_level))
		{
			env_Print(FormatArgs("Successfully changed the admin's privilege level to %d.", admin_level).c_str());
			GlobalAdminList.SaveAdmins();
		}
		else
		{
			env_Print(FormatArgs("Admin does not exist with ID '%u'.", admin_id).c_str());
		}

		return true;
	}
};

class CommandAdminV2Del final : public GenericCommand
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
	CommandAdminV2Del() : GenericCommand(L"admin_del", L"<admin id>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		GlobalAdminList.LoadAdmins();

		if (GlobalAdminList.RemoveAdmin(admin_id))
		{
			env_Print(FormatArgs("Successfully removed admin, ID: %u.", admin_id).c_str());
			GlobalAdminList.SaveAdmins();			
		}
		else
		{
			env_Print(FormatArgs("Admin does not exist with ID '%u'.", admin_id).c_str());
		}
		return true;
	}
};

class CommandAdminV2List final : public GenericCommand
{
public:
	CommandAdminV2List() : GenericCommand(L"admin_list") {}

	bool Execute(const CommandEnviroment& env) override
	{
		env.StreamPrint(GlobalAdminList.StreamOut());
		return true;
	}
};

class CommandChangePassword final : public GenericCommand
{
	std::string old_password;
	std::string new_password;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			old_password = WToStr(args[1]);
			new_password = WToStr(args[2]);
			return true;
		}
		return false;
	}

public:
	CommandChangePassword() : GenericCommand(L"change_password", L"<old password> <new password>", 0, ServerOnly::no, ClientOnly::yes, Naughty::no, Unalterable::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		unsigned char pwmd5old[16], pwmd5new[16];
		MD5_Hash(old_password.c_str(), old_password.length(), pwmd5old);
		MD5_Hash(new_password.c_str(), new_password.length(), pwmd5new);

		std::string old_password_hash = ConvertToMD5Hash(pwmd5old), new_password_hash = ConvertToMD5Hash(pwmd5new);

		if(GlobalAdminList.IsAdmin(PlayerTable[env.sPlayerIndex].name, old_password_hash) == -1)
		{
			env_Print("Error! You are not an admin or the old password is incorrect!");
			return true;
		}

		GlobalAdminList.LoadAdmins();

		if(GlobalAdminList.ChangePassword(GlobalAdminList.GetAdminID(PlayerTable[env.sPlayerIndex].name), new_password_hash))
		{
			env_Print("Successfully changed your password.");
			GlobalAdminList.SaveAdmins();
		}

		return true;
	}	
};