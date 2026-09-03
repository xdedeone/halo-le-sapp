#include "PlayerCommand.h"

#include "Addresses.h"
#include "HaloUtilities.h"
#include "MiscellaneousFunctions.h"
#include "OnlineAdmins.h"
#include "TextUtilities.h"
#include "Prefs.h"

const std::map <std::wstring, PlayerExpr, WStringICmp> PlayerCommand::player_argument_cache =
{ { L"me", PlayerExpr::ME }, { L"rt", PlayerExpr::RT }, { L"bt", PlayerExpr::BT }, { L"pl", PlayerExpr::PL }, { L"admin", PlayerExpr::ADMIN }, { L"rand", PlayerExpr::RAND }, { L"randred", PlayerExpr::RANDRED }, { L"randblue", PlayerExpr::RANDBLUE } };

void PlayerCommand::PrintUsage(const CommandEnviroment& env, bool print_level) const
{
	std::vector<std::wstring> items;
	if (!Split(items, usage, L'\n'))
	{
		std::wstring usage_str = (print_level ? (level < 0 ? L"(" : L"( ") + ToWStr(level) + L") " + name : name) + L" <player expression> " + usage;
		env.Print(usage_str.c_str());
	}
	else
	{
		for (auto& it : items)
		{
			std::wstring usage_str = (print_level ? (level < 0 ? L"(" : L"( ") + ToWStr(level) + L") " + name : name) + L" <player expression> " + it;
			env.Print(usage_str.c_str());
		}
	}
}

bool PlayerCommand::ExecuteCommand(ArgContainer& arg, const CommandEnviroment& env)
{
	if (!ServerRunning())
	{
		env_Print(FormatArgs("'%ls' is a server-only function!", arg[0].c_str()).c_str());
		return false;
	}

	if(ParseArgs(arg))
	{
		std::vector <size_t> playerIndexes;
		size_t playerIndex;
		if (arg[1].empty())
		{
			if (PlayerPresent(env.sPlayerIndex))
			{
				playerIndexes.emplace_back(env.sPlayerIndex);
			}
		}
		else if (ToNumber(arg[1], playerIndex) && playerIndex != 0 && playerIndex <= 16)
		{
			int index = ToPlayerIndex(playerIndex);
			if(index != -1)
			{
				playerIndexes.emplace_back(index);
			}
			else
			{
				env_Print(FormatWArgs(L"Player #%ls is not present.", arg[1].c_str()).c_str());
				return true;
			}
		}
		else
		{
			auto& it = player_argument_cache.find(arg[1]);
			if(it != player_argument_cache.end())
			{
				switch (it->second)
				{
				case PlayerExpr::ME:
					{
						if(PlayerPresent(env.sPlayerIndex))
						{
							playerIndexes.emplace_back(env.sPlayerIndex);
						}
						break;
					}
				case PlayerExpr::RT:
					{
						for(size_t k = 0; k < PlayerHeader->SlotsTaken; k++)
						{
							if(PlayerPresent(k))
							{
								if(PlayerTable[k].Team == 0)
								{
									playerIndexes.emplace_back(k);
								}
							}
						}
						break;
					}
				case PlayerExpr::BT:
					{
						for(size_t k = 0; k < PlayerHeader->SlotsTaken; k++)
						{
							if(PlayerPresent(k))
							{
								if(PlayerTable[k].Team == 1)
								{
									playerIndexes.emplace_back(k);
								}
							}
						}
						break;
					}
				case PlayerExpr::PL:
					{
						for(size_t k = 0; k < PlayerHeader->SlotsTaken; k++)
						{
							if(GlobalOnlineAdminList.GetAdminLevel(k) < 0)
							{
								playerIndexes.emplace_back(k);
							}
						}
						break;
					}
				case PlayerExpr::ADMIN:
					{
						for(size_t k = 0; k < PlayerHeader->SlotsTaken; k++)
						{
							if(GlobalOnlineAdminList.GetAdminLevel(k) > 0)
							{
								playerIndexes.emplace_back(k);
							}
						}
						break;
					}
				case PlayerExpr::RAND:
					{
						int PlayerNumber = 0;
						size_t TakenSlotIndexes[16];

						GetPlayers(PlayerNumber, TakenSlotIndexes);

						if(PlayerNumber != 0)
						{
							size_t n = GetRand(0, PlayerNumber);
							playerIndexes.emplace_back(TakenSlotIndexes[n]);
						}
						break;
					}
				case PlayerExpr::RANDRED:
					{
						int PlayerNumber = 0;
						size_t TakenSlotIndexes[16];

						GetReds(PlayerNumber, TakenSlotIndexes);

						if(PlayerNumber != 0)
						{
							size_t n = GetRand(0, PlayerNumber);
							playerIndexes.emplace_back(TakenSlotIndexes[n]);
						}
						break;
					}
				case PlayerExpr::RANDBLUE:
					{
						int PlayerNumber = 0;
						size_t TakenSlotIndexes[16];

						GetBlues(PlayerNumber, TakenSlotIndexes);

						if(PlayerNumber != 0)
						{
							size_t n = GetRand(0, PlayerNumber);
							playerIndexes.emplace_back(TakenSlotIndexes[n]);
						}
						break;
					}
				}
			}
			else
			{
				for(size_t k = 0; k < PlayerHeader->SlotsTaken; k++)
				{
					if(PlayerPresent(k))
					{
						if(wildicmp(arg[1].c_str(), PlayerTable[k].name))
						{
							playerIndexes.emplace_back(k);
						}
					}			
				}
			}
		}

		if(playerIndexes.empty())
		{
			env_Print(FormatWArgs(L"No player matches '%ls' expression.", arg[1].c_str()).c_str());
		}
		else
		{
			size_t executed = 0;
			for(size_t k = 0; k < playerIndexes.size(); k++)
			{
				if (no_multiple_execution == NoMultipleExecution::yes && executed > 0)
					break;

				size_t PlayerIndex = playerIndexes[k];
				if (alive_only == AliveOnly::yes && !PlayerAliveOnly(PlayerIndex))
				{
					env_Print(FormatWArgs(L"%ls is dead.", PlayerTable[PlayerIndex].name).c_str());
				}
				else
				{
					if (ExecuteOnPlayer(PlayerIndex, env))
						executed++;
				}
			}
		}
		return true;
	}
	else
	{
		env_Print(FormatWArgs(L"Incorrect argument(s) for '%ls' command. Usage:", name.c_str()).c_str());
		PrintUsage(env);
		return false;
	}
}