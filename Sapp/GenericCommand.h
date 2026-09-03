#pragma once

#include "BaseCommand.h"

enum class ServerOnly : bool { no, yes };
enum class ClientOnly : bool { no, yes };

class GenericCommand : public BaseCommand
{
	const ServerOnly server_only;
	const ClientOnly client_only;

	virtual bool Execute(const CommandEnviroment& env) = 0;

public:
	GenericCommand(std::wstring&& name, std::wstring&& usage = L"", int level = 4, ServerOnly server_only = ServerOnly::no, ClientOnly client_only = ClientOnly::no, Naughty naughty = Naughty::no, Unalterable unalterable = Unalterable::no)
		: BaseCommand(std::move(name), std::move(usage), level, CommandType::GENERIC, naughty, unalterable), server_only(server_only), client_only(client_only) {}
	
	virtual ~GenericCommand() {}

	void PrintUsage(const CommandEnviroment& env, bool print_level = false) const override final;

	bool ExecuteCommand(ArgContainer& args, const CommandEnviroment& env) override final;
};

class PrefBoolCommand : public GenericCommand
{
	bool ParseArgs(ArgContainer& args) override;

protected:
	std::wstring value;

public:
	PrefBoolCommand(std::wstring&& name) : GenericCommand(std::move(name), L"[boolean]") {}
};

class PrefStringCommand : public GenericCommand
{
	bool ParseArgs(ArgContainer& args) override;

protected:
	std::wstring value;
	bool set;

public:
	PrefStringCommand(std::wstring&& name, std::wstring&& usage) : GenericCommand(std::move(name), std::move(usage)) {}
};

class PrefNumberCommand : public GenericCommand
{
	const int min_val, max_val;

	bool ParseArgs(ArgContainer& args) override;

protected:
	int value;
	bool set;

public:
	PrefNumberCommand(std::wstring&& name, std::wstring&& usage, int min_val = 0, int max_val = 2147483647) : GenericCommand(std::move(name), std::move(usage)), min_val(min_val), max_val(max_val) {}
};