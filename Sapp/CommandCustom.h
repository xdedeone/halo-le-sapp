#pragma once

#include "CustomCommand.h"
#include "CustomCommandFactory.h"
#include "GenericCommand.h"

class CommandCustomAdd final : public GenericCommand
{
	std::wstring command_name;
	std::wstring command_sequence;
	std::vector <std::wstring> command_args;
	int command_level;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc > 2)
		{
			command_args.clear();
			command_name = std::move(args[1]);
			size_t param = 2;
			while (args.argc > param + 1 && args[param][0] == L'#')
			{
				command_args.emplace_back(std::move(args[param]));
				param++;
			}
			command_sequence = std::move(args[param]);
			if (args.argc == param + 1)
			{
				command_level = 4;
				return true;
			}
			else if (args.argc == param + 2)
			{
				if (ToNumber(args[param + 1], command_level) && command_level >= -1 && command_level <= 4)
					return true;
			}	
		}
		return false;
	}

public:
	CommandCustomAdd() : GenericCommand(L"cmd_add", L"<command name> [#arguments]... <command sequence> [level(=4, -1-4)]") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if(GlobalCommandFactory.Exists(command_name))
		{
			env_Print(FormatWArgs(L"Error: The name \"%ls\" is already taken by another command!", command_name.c_str()).c_str());
			return true;
		}
		
		GlobalCommandFactory.AddCommand(std::make_unique<CustomCommand>(std::wstring(command_name), command_level, command_sequence, command_args));
		SaveCustomCommand(command_name, command_sequence, command_args, command_level);
		env_Print(FormatWArgs(L"Successfully added custom command \"%ls\" with level %d.", command_name.c_str(), command_level).c_str());

		return true;
	}
};

class CommandCustomDel final : public GenericCommand
{
	std::wstring command_name;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			command_name = std::move(args[1]);
			return true;
		}
		return false;
	}

public:
	CommandCustomDel() : GenericCommand(L"cmd_del", L"<command name>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (GlobalCommandFactory.RemoveCommand(command_name))
		{
			env_Print(FormatWArgs(L"Successfully removed custom command \"%ls\".", command_name.c_str()).c_str());
		}
		else
		{
			env_Print(FormatWArgs(L"Can't remove \"%ls\", custom command not found!", command_name.c_str()).c_str());
		}
		return true;
	}
};