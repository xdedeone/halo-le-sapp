#pragma once

#include <functional>
#include <map>

#include "BaseCommand.h"
#include "StringCompare.h"

class CommandFactory final
{
	std::map <std::reference_wrapper<const std::wstring>, std::unique_ptr<BaseCommand>, WStringICmp> all_command;

public:
	int ExecuteCommand(ArgContainer& args, const CommandEnviroment& env, bool block = false);
	void LoadCommands();
	void ClearCommands();
	bool Exists(const std::wstring& name);
	void AddCommand(std::unique_ptr<BaseCommand>&& command);
	bool RemoveCommand(const std::wstring& name);
	bool ChangeName(const std::wstring& name, const std::wstring& new_name);
	bool ChangeLevel(const std::wstring& name, int new_level);
	bool PrintUsage(const std::wstring& name, const CommandEnviroment& env);
	void ListCommands(CommandType type, const CommandEnviroment& env);
	const decltype(all_command)& GetStorage() { return all_command; }
};

extern CommandFactory GlobalCommandFactory;