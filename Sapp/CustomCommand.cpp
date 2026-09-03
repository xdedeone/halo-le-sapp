#include "CustomCommand.h"

#include "CommandExecuter.h"
#include "TextUtilities.h"

void CustomCommand::ReplaceArgs(std::wstring& Sequence, ArgContainer& ArgCont) const
{
	for(size_t i = 0; i < command_args.size(); i++)
	{
		size_t name_pos = 0;
		while((name_pos = Sequence.find(command_args[i], name_pos)) != std::wstring::npos)
		{
			Sequence.replace(name_pos, command_args[i].length(), ArgCont[i+1]);
			name_pos++;
		}
	}
}

bool CustomCommand::ParseArgs(ArgContainer& args)
{
	return command_args.size() == args.argc - 1;
}

void CustomCommand::PrintUsage(const CommandEnviroment& env, bool print_level) const
{
	std::wstring usage_str = (print_level ? (level < 0 ? L"(" : L"( ") + ToWStr(level) + L") " + name : name);
	for (size_t i = 0; i < command_args.size(); i++)
	{
		usage_str += L" <arg" + ToWStr(static_cast<int>(i + 1)) + L">";
	}
	env.Print(usage_str.c_str());
}

bool CustomCommand::ExecuteCommand(ArgContainer& args, const CommandEnviroment& env)
{
	if (ParseArgs(args))
	{
		std::wstring FinalSequence = command_sequence;

		ReplaceArgs(FinalSequence, args);

		CommandEnviroment script_env(env.type, env.sPlayerIndex, 4, true);
		ExecuteSequence(FinalSequence, script_env);

		return true;
	}
	else
	{
		env_Print(FormatWArgs(L"This command takes %u argument(s). Usage:", command_args.size()).c_str());
		PrintUsage(env);
		return false;
	}
}