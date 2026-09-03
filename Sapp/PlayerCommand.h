#pragma once

#include <map>

#include "BaseCommand.h"
#include "StringCompare.h"

enum class AliveOnly : bool { no, yes };
enum class NoMultipleExecution : bool { no, yes };
enum class PlayerExpr { ME, RT, BT, PL, ADMIN, RAND, RANDRED, RANDBLUE };

class PlayerCommand: public BaseCommand
{
	const AliveOnly alive_only;
	const NoMultipleExecution no_multiple_execution;  // true if command can't be executed on more than 1 player at once

	static const std::map <std::wstring, PlayerExpr, WStringICmp> player_argument_cache;

	virtual bool ParseArgs(ArgContainer& args) override
	{
		return args.argc <= 2;
	}

	virtual bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) = 0;

public:
	PlayerCommand(std::wstring&& name, std::wstring&& usage = L"", int level = 4, Naughty naughty = Naughty::no, AliveOnly alive_only = AliveOnly::no, NoMultipleExecution no_multiple_execution = NoMultipleExecution::no)
		: BaseCommand(std::move(name), std::move(usage), level, CommandType::PLAYER, naughty, Unalterable::no), alive_only(alive_only), no_multiple_execution(no_multiple_execution) {}

	virtual ~PlayerCommand() {}

	void PrintUsage(const CommandEnviroment& env, bool print_level = false) const override final;

	bool ExecuteCommand(ArgContainer& args, const CommandEnviroment& env) override final;
};