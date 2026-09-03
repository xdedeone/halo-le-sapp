#pragma once

#include "GenericCommand.h"
#include "Replace.h"

class CommandVarAdd final : public GenericCommand
{
	std::wstring var_name;
	CustomVariableFlags var_type;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			var_name = L"$" + std::move(args[1]);
			size_t var_type_num;
			if (ToNumber(args[2], var_type_num) && var_type_num < 6)
			{
				var_type = static_cast<CustomVariableFlags>(var_type_num);
				return true;
			}
		}
		return false;
	}

public:
	CommandVarAdd() : GenericCommand(L"var_add", L"<name> <type(0-5)>") {}

	bool Execute(const CommandEnviroment& env) override
	{	
		if (AddCustomVar(var_name, var_type))
		{
			env_Print(L"Successfully added the custom variable.");			
		}
		else
		{
			env_Print(L"Error: This variable name already exists.");
		}
		return true;
	}
};

class CommandVarDel final : public GenericCommand
{
	std::wstring var_name;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			var_name = L"$" + std::move(args[1]);
			return true;
		}
		return false;
	}

public:
	CommandVarDel() : GenericCommand(L"var_del", L"<name>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (DelCustomVar(var_name))
		{
			env_Print(L"Successfully deleted the custom variable.");			
		}
		else
		{
			env_Print(L"Error: Didn't find any custom variable with this name.");
		}
		return true;
	}
};

class CommandVarSet final : public GenericCommand
{
	std::wstring var_name;
	std::wstring var_value;
	size_t player_index;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			var_name = L"$" + std::move(args[1]);
			var_value = std::move(args[2]);
			player_index = 0;
			return true;
		}
		else if (args.argc == 4)
		{
			var_name = L"$" + std::move(args[1]);
			var_value = std::move(args[2]);
			if (ToNumber(args[3], player_index) && player_index <= 16)
				return true;
		}
		return false;
	}

public:
	CommandVarSet() : GenericCommand(L"var_set", L"<name> <value> [player index(=0, 0-16)]") {}

	bool Execute(const CommandEnviroment& env) override
	{
		size_t PlayerIndex = (size_t)NumberToIndex(player_index - 1);
		if (PlayerIndex > 15)
			PlayerIndex = env.sPlayerIndex;

		if (!SetCustomVar(var_name, var_value, PlayerIndex))
		{
			env_Print(L"Error: Didn't find any custom variable with this name.");
		}
		return true;
	}
};

class CommandVarConvert final : public GenericCommand
{
	std::wstring var_name;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			var_name = L"$" + std::move(args[1]);
			return true;
		}
		return false;
	}

public:
	CommandVarConvert() : GenericCommand(L"var_conv", L"<name>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (!ConvertCustomVar(var_name))
		{
			env_Print(L"Error: Didn't find any custom variable with this name or the variable is not convertible.");
		}
		return true;
	}
};

class CommandVarList final : public GenericCommand
{
	bool custom;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 1)
		{
			custom = false;
			return true;
		}
		else if (args.argc == 2)
		{
			if (!_wcsicmp(args[1].c_str(), L"custom"))
				custom = true;
			else
				return false;
			return true;
		}
		return false;
	}

public:
	CommandVarList() : GenericCommand(L"var_list", L"[custom]") {}

	bool Execute(const CommandEnviroment& env) override
	{
		env.StreamPrint(StreamOutVariables(custom));
		return true;
	}
};