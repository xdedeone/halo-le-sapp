#pragma once

#include "AliasThread.h"
#include "PlayerCommand.h"

class CommandAlias final : public PlayerCommand
{
public:
	CommandAlias() : PlayerCommand(L"alias") {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		Lock_Object guard(GetAliasLock());
		auto& alias = GetAliasNoLock(GetPlayerHash(IndexToNumber(PlayerIndex)));

		if(alias.empty())
		{
			env_Print("There are no aliases for this player yet.");
		}
		else
		{
			std::wstring line = L"Names used by this player: ";	

			for (auto& it = alias.begin(); it != alias.end(); ++it)
			{
				line += *it;
				if(line.length() > 128)
				{
					env_Print(line.c_str());
					line.clear();
				}
				else
				{
					line += L", ";
				}
			}

			if (line.size())
			{
				line.erase(line.begin() + line.size() - 2, line.end());
				env_Print(line.c_str());
			}
		}
		return true;
	}
};