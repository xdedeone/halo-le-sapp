#pragma once

#include "ObjectManagement.h"
#include "PlayerCommand.h"

void __stdcall EventTeamChange(DWORD i);

class CommandSwitchTeam final : public PlayerCommand
{
	size_t switchTo; // 0 - red, 1 - blue, 2 - toggle

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc < 3)
		{
			switchTo = 2;
			return true;
		}
		else
		{
			if (!_wcsicmp(args[2].c_str(), L"red"))
			{
				switchTo = 0;
				return true;
			}
			else if (!_wcsicmp(args[2].c_str(), L"blue"))
			{
				switchTo = 1;
				return true;
			}
		}
		return false;
	}

public:
	CommandSwitchTeam() : PlayerCommand(L"st", L"[red/blue]", 2) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if (!*TeamGame)
		{
			env_Print("There is no team game in progress.");
			return false;
		}

		if(switchTo == 2)
		{
			if(PlayerTable[PlayerIndex].Team == 0)
			{
				Server->PlayerData[PlayerIndex].team = 1;
				PlayerTable[PlayerIndex].Team = 1;
			}
			else
			{
				Server->PlayerData[PlayerIndex].team = 0;
				PlayerTable[PlayerIndex].Team = 0;
			}
			EventTeamChange(PlayerIndex);
		}
		else if(switchTo == 0 || switchTo == 1)
		{
			if (switchTo == PlayerTable[PlayerIndex].Team)
			{
				env_Print(FormatWArgs(L"%ls is already in the %ls team.", PlayerTable[PlayerIndex].name, PlayerTable[PlayerIndex].Team == 0 ? L"red" : L"blue").c_str());
				return true;
			}
				
			Server->PlayerData[PlayerIndex].team = static_cast<char>(switchTo);
			PlayerTable[PlayerIndex].Team = switchTo;
			EventTeamChange(PlayerIndex);
		}

		env_Print(FormatWArgs(L"Switching %ls to the %ls team.", PlayerTable[PlayerIndex].name, PlayerTable[PlayerIndex].Team == 0 ? L"red" : L"blue").c_str());

		if (PlayerAliveOnly(PlayerIndex))
		{
			KillPlayer(PlayerIndex);
			PlayerTable[PlayerIndex].deaths--;
		}

		return true;
	}
};