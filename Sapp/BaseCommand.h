#pragma once

#include <memory>
#include <string>

#include "ArgContainer.h"
#include "CommandEnviroment.h"

enum class Naughty : bool { no, yes };
enum class Unalterable : bool { no, yes };

enum class CommandType { BASE, GENERIC, PLAYER, CUSTOM };

class BaseCommand
{
	friend class CommandFactory;
protected:
	std::wstring name;
	const std::wstring usage;

	int level;

	const CommandType type;
	
	const Naughty naughty;
	const Unalterable unalterable;

	virtual bool ParseArgs(ArgContainer& args)
	{
		return args.argc == 1;
	}

public:
	BaseCommand(std::wstring&& name, std::wstring&& usage = L"", int level = 4, CommandType type = CommandType::BASE, Naughty naughty = Naughty::no, Unalterable unalterable = Unalterable::no)
		: name(std::move(name)), usage(std::move(usage)), level(level), type(type), naughty(naughty), unalterable(unalterable) {}

	virtual ~BaseCommand() {}

	virtual void PrintUsage(const CommandEnviroment& env, bool print_level = false) const = 0;

	virtual bool ExecuteCommand(ArgContainer& args, const CommandEnviroment& env) = 0;
};