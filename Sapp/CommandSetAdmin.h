#pragma once

#include "OnlineAdmins.h"
#include "PlayerCommand.h"
#include "TextUtilities.h"

class CommandSetAdmin final : public PlayerCommand
{
	int new_level;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			if (ToNumber(args[2], new_level) && new_level >= -1 && new_level <= 4)
				return true;
		}
		return false;
	}

public:
	CommandSetAdmin() : PlayerCommand(L"setadmin", L"<level(-1-4)>") {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if (new_level > -1)
		{
			Login(PlayerIndex, new_level);
		}
		else
		{
			GlobalOnlineAdminList.SetAdminLevel(PlayerIndex, -1);
			NetworkEventPlayer<DataSync<bool>>(PlayerIndex, NE_DataSync_SJ, GlobalOnlineAdminList.GetAdminLevel(PlayerIndex) >= GlobalPrefs->SJLevel);
		}
		env_Print(FormatWArgs(L"%ls's level is now %d.", PlayerTable[PlayerIndex].name, new_level).c_str());

		return true;
	}
};