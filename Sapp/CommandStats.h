#pragma once

#include "GenericCommand.h"
#include "PlayerCommand.h"

class CommandKDR final : public PlayerCommand
{
public:
	CommandKDR() : PlayerCommand(L"kdr", L"", 0) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if(PlayerTable[PlayerIndex].deaths == 0)
		{
			env_Print(FormatWArgs(L"%ls's Kill\\Death Ratio is infinitive", PlayerTable[PlayerIndex].name).c_str());
		}
		else
		{
			env_Print(FormatWArgs(L"%ls's Kill\\Death Ratio is %4.2f", PlayerTable[PlayerIndex].name, (PlayerTable[PlayerIndex].kills*1.0f)/(PlayerTable[PlayerIndex].deaths*1.0f)).c_str());
		}
			
		return true;
	}
};

class CommandKills final : public PlayerCommand
{
	bool setStats;
	std::wstring statsToSet;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc < 3)
		{
			setStats = false;
			return true;
		}
		else if (args.argc == 3)
		{
			setStats = true;
			statsToSet = std::move(args[2]);
			return true;
		}
		return false;
	}

public:	
	CommandKills() : PlayerCommand(L"kills", L"[kills]", 4, Naughty::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if(setStats)
			ProcessOperator(statsToSet, PlayerTable[PlayerIndex].kills);

		env_Print(FormatWArgs(L"%ls's kills: %d", PlayerTable[PlayerIndex].name, PlayerTable[PlayerIndex].kills).c_str());

		return true;
	}
};

class CommandAssists final : public PlayerCommand
{
	bool setStats;
	std::wstring statsToSet;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc < 3)
		{
			setStats = false;
			return true;
		}
		else if (args.argc == 3)
		{
			setStats = true;
			statsToSet = std::move(args[2]);
			return true;
		}
		return false;
	}

public:
	CommandAssists() : PlayerCommand(L"assists", L"[assists]", 4, Naughty::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if(setStats)
			ProcessOperator(statsToSet, PlayerTable[PlayerIndex].assists);

		env_Print(FormatWArgs(L"%ls's assists: %d", PlayerTable[PlayerIndex].name, PlayerTable[PlayerIndex].assists).c_str());

		return true;
	}
};

class CommandDeaths final : public PlayerCommand
{
	bool setStats;
	std::wstring statsToSet;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc < 3)
		{
			setStats = false;
			return true;
		}
		else if (args.argc == 3)
		{
			setStats = true;
			statsToSet = std::move(args[2]);
			return true;
		}
		return false;
	}

public:
	CommandDeaths() : PlayerCommand(L"deaths", L"[deaths]", 4, Naughty::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if(setStats)
			ProcessOperator(statsToSet, PlayerTable[PlayerIndex].deaths);

		env_Print(FormatWArgs(L"%ls's deaths: %d", PlayerTable[PlayerIndex].name, PlayerTable[PlayerIndex].deaths).c_str());

		return true;
	}
};

class CommandHealth final : public PlayerCommand
{
	bool setStats;
	std::wstring statsToSet;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc < 3)
		{
			setStats = false;
			return true;
		}
		else if (args.argc == 3)
		{
			setStats = true;
			statsToSet = std::move(args[2]);
			return true;
		}
		return false;
	}

public:	
	CommandHealth() : PlayerCommand(L"hp", L"[health]", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

		if(setStats)
			ProcessOperator(statsToSet, Player->Health);

		env_Print(FormatWArgs(L"%ls's health: %4.2f", PlayerTable[PlayerIndex].name, Player->Health).c_str());

		return true;
	}
};

class CommandShield final : public PlayerCommand
{
	bool setStats;
	std::wstring statsToSet;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc < 3)
		{
			setStats = false;
			return true;
		}
		else if (args.argc == 3)
		{
			setStats = true;
			statsToSet = std::move(args[2]);
			return true;
		}
		return false;
	}

public:
	CommandShield() : PlayerCommand(L"sh", L"[shield]", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

		if(setStats)
			ProcessOperator(statsToSet, Player->Shield);

		env_Print(FormatWArgs(L"%ls's shield: %4.2f", PlayerTable[PlayerIndex].name, Player->Shield).c_str());

		return true;
	}
};

class CommandScore final : public PlayerCommand
{
	bool setStats;
	std::wstring statsToSet;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc < 3)
		{
			setStats = false;
			return true;
		}
		else if (args.argc == 3)
		{
			setStats = true;
			statsToSet = std::move(args[2]);
			return true;
		}
		return false;
	}

	void SetPlayerStats(size_t PlayerIndex)
	{
		switch (CurrentGameType)
		{
		case GameType::CTF:
			ProcessOperator(statsToSet, PlayerTable[PlayerIndex].ctf_score);
			break;
		case GameType::Slayer:
		{
			int tmp = SlayerScores->PlayerScores[PlayerIndex];
			ProcessOperator(statsToSet, tmp);
			SlayerScores->PlayerScores[PlayerIndex] = tmp;
			if (!*TeamGame)
				SlayerScores->TeamScores[PlayerIndex] = tmp;
			break;
		}
		case GameType::Oddball:
		{
			if (*ScoreType != 2)
			{
				int tmp = OddballScores->PlayerScores[PlayerIndex] / 30;
				ProcessOperator(statsToSet, tmp);
				tmp *= 30;
				OddballScores->PlayerScores[PlayerIndex] = tmp;
				if (!*TeamGame)
					OddballScores->TeamScores[PlayerIndex] = tmp;
			}
			else
			{
				int tmp = OddballScores->PlayerScores[PlayerIndex];
				ProcessOperator(statsToSet, tmp);
				OddballScores->PlayerScores[PlayerIndex] = tmp;
				if (!*TeamGame)
					OddballScores->TeamScores[PlayerIndex] = tmp;
			}		
			break;
		}
		case GameType::King:
		{
			int tmp = PlayerTable[PlayerIndex].king_time / 30;
			ProcessOperator(statsToSet, tmp);
			tmp *= 30;
			PlayerTable[PlayerIndex].king_time = tmp;
			if (!*TeamGame)
				KingScores->TeamScores[PlayerIndex] = tmp;
			break;
		}
		case GameType::Race:
		{
			int tmp = PlayerTable[PlayerIndex].race_score;
			ProcessOperator(statsToSet, tmp);
			PlayerTable[PlayerIndex].race_score = tmp;
			if (!*TeamGame)
				RaceScores->TeamScores[PlayerIndex] = tmp;
			break;
		}
		default:
			break;
		}
	}

public:
	CommandScore() : PlayerCommand(L"score", L"[score]", 4, Naughty::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if (setStats)
		{
			SetPlayerStats(PlayerIndex);
		}			

		env_Print(FormatWArgs(L"%ls's score: %d", PlayerTable[PlayerIndex].name, GetPlayerScore(PlayerIndex)).c_str());

		return true;
	}
};

class CommandTeamScore final : public GenericCommand
{
	size_t team;
	bool setStats;
	std::wstring statsToSet;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc <= 2)
		{
			setStats = false;
			return true;
		}
		else if (args.argc == 3)
		{
			setStats = true;
			if (ToNumber(args[1], team))
			{
				if (team > 2)
					return false;
				statsToSet = std::move(args[2]);
				return true;
			}
			else
			{
				if (!_wcsicmp(args[1].c_str(), L"red"))
					team = 0;
				else if (!_wcsicmp(args[1].c_str(), L"blue"))
					team = 1;
				else if (!_wcsicmp(args[1].c_str(), L"both"))
					team = 2;
				else
					return false;
				statsToSet = std::move(args[2]);
				return true;
			}
		}
		return false;
	}

	void SetTeamScore(size_t TeamIndex)
	{
		switch (CurrentGameType)
		{
		case GameType::CTF:
			TeamIndex == 0 ? ProcessOperator(statsToSet, CTFScores->Reds) : ProcessOperator(statsToSet, CTFScores->Blues);
			break;
		case GameType::Slayer:
			ProcessOperator(statsToSet, SlayerScores->TeamScores[TeamIndex]);
			break;
		case GameType::Oddball:
		{
			if (*ScoreType != 2)
			{
				int tmp = OddballScores->TeamScores[TeamIndex] / 30;
				ProcessOperator(statsToSet, tmp);
				OddballScores->TeamScores[TeamIndex] = tmp * 30;
			}
			else
			{
				ProcessOperator(statsToSet, OddballScores->TeamScores[TeamIndex]);
			}
			break;
		}
		case GameType::King:
		{
			int tmp = KingScores->TeamScores[TeamIndex] / 30;
			ProcessOperator(statsToSet, tmp);
			KingScores->TeamScores[TeamIndex] = tmp * 30;
			break;
		}
		case GameType::Race:
			ProcessOperator(statsToSet, RaceScores->TeamScores[TeamIndex]);
			break;
		default:
			break;
		}
	}

public:
	CommandTeamScore() : GenericCommand(L"team_score", L"[team(0-2)] [score]\n[red/blue/both] [score]", 4, ServerOnly::yes, ClientOnly::no, Naughty::yes) {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if(!*TeamGame)
		{
			env_Print("There is no team game in progress.");
			return true;
		}

		if(setStats)
		{
			if (team == 0 || team == 2)
				SetTeamScore(0);
			if (team == 1 || team == 2)
				SetTeamScore(1);
		}

		env_Print(FormatArgs("Red's score: %d, Blue's score: %d", GetTeamScore(0), GetTeamScore(1)).c_str());
		return true;
	}
};