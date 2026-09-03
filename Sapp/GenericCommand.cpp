#include "GenericCommand.h"

#include "HaloUtilities.h"
#include "TextUtilities.h"

void GenericCommand::PrintUsage(const CommandEnviroment& env, bool print_level) const
{	
	std::vector<std::wstring> items;
	if (!Split(items, usage, L'\n'))
	{
		std::wstring usage_str = (print_level ? (level < 0 ? L"(" : L"( ") + ToWStr(level) + L") " + name : name) + L" " + usage;
		env.Print(usage_str.c_str());
	}
	else
	{
		for (auto& it : items)
		{
			std::wstring usage_str = (print_level ? (level < 0 ? L"(" : L"( ") + ToWStr(level) + L") " + name : name) + L" " + it;
			env.Print(usage_str.c_str());
		}
	}	
}

bool GenericCommand::ExecuteCommand(ArgContainer& arg, const CommandEnviroment& env)
{
	if (server_only == ServerOnly::yes && !ServerRunning())
	{
		env_Print(FormatArgs("'%ls' is a server-only function!", name.c_str()).c_str());
		return false;
	}

	if (client_only == ClientOnly::yes && env.type != Enviroment::CHAT && env.type != Enviroment::RCON)
	{
		env_Print(FormatArgs("'%ls' is a client-only function!", name.c_str()).c_str());
		return false;
	}

	if (ParseArgs(arg))
	{
		return Execute(env);
	}
	else
	{
		env_Print(FormatWArgs(L"Incorrect argument(s) for '%ls' command. Usage:", name.c_str()).c_str());
		PrintUsage(env);
		return false;
	}
}

bool PrefBoolCommand::ParseArgs(ArgContainer& args)
{
	if (args.argc == 1)
	{
		value.clear();
		return true;
	}
	else if (args.argc == 2)
	{
		value = std::move(args[1]);
		return true;
	}
	return false;
}

bool PrefStringCommand::ParseArgs(ArgContainer& args)
{
	if (args.argc == 1)
	{
		set = false;
		return true;
	}
	else if (args.argc == 2)
	{
		set = true;
		value = std::move(args[1]);
		return true;
	}
	return false;
}

bool PrefNumberCommand::ParseArgs(ArgContainer& args)
{
	if (args.argc == 1)
	{
		set = false;
		return true;
	}
	else if (args.argc == 2)
	{
		set = true;
		if (!ToNumber(args[1], value) || ((value < min_val || value > max_val) && value != 0))
			return false;
		return true;
	}
	return false;
}