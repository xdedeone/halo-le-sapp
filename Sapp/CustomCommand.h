#pragma once

#include "BaseCommand.h"

class CustomCommand final : public BaseCommand
{
	const std::wstring command_sequence;
	const std::vector <std::wstring> command_args;

	bool ParseArgs(ArgContainer& args) override;

	void ReplaceArgs(std::wstring& Sequence, ArgContainer& ArgCont) const;

public:
	CustomCommand(std::wstring&& name, int level, const std::wstring& cmd, const std::vector <std::wstring>& args)
		: BaseCommand(std::move(name), L"", level, CommandType::CUSTOM, Naughty::no, Unalterable::no), command_sequence(cmd), command_args(args) {}

	void PrintUsage(const CommandEnviroment& env, bool print_level = false) const override;

	bool ExecuteCommand(ArgContainer& args, const CommandEnviroment& env) override;
};