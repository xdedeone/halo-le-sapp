#include "CommandExecuter.h"

#include "Addresses.h"
#include "ArgContainer.h"
#include "CommandFactory.h"
#include "Prefs.h"
#include "Replace.h"
#include "TextUtilities.h"
#include "Threads.h"

int CommandExecuting = -1; // -4: Remote Console, -3: Console, -2: Script, -1: Nothing, 0-15: Chat (PlayerIndex)

bool ExecuteHaloCommand(size_t PlayerIndex, Enviroment type, const char* command)
{
	switch (type)
	{
	case Enviroment::CONSOLE:
		CommandExecuting = -3;
		break;
	case Enviroment::CHAT:
		CommandExecuting = PlayerIndex;
		break;
	case Enviroment::SCRIPT:
		CommandExecuting = -2;
		break;
	case Enviroment::INIT:
		CommandExecuting = -3;
		break;
	case Enviroment::REMOTE:
		CommandExecuting = -4;
		break;
	default:
		CommandExecuting = -1;
	}

	__asm
	{
		xor eax, eax
		mov edi, command
		push eax
		call HaloExecuteAddress
		add esp, 4
	}

	CommandExecuting = -1;
}

void ExecuteSequenceThread(std::shared_ptr<work>& hThread)
{
	CommandSequence* cmd = static_cast<CommandSequence*>(hThread->GetParams());

	HaloThreadLock.Lock();
	CommandEnviroment env(cmd->Env, cmd->PlayerIndex, 4, true);
	bool done = ExecuteSequence(cmd->Command, env, cmd);
	HaloThreadLock.Unlock();

	if (done)
	{
		ThreadPool.DeleteTimer(cmd);
	}
}

void __stdcall CommandTimerDue(PVOID params, BOOLEAN)
{
	ThreadPool.NewWork(ExecuteSequenceThread, params);
}

bool ExecuteSequence(const std::wstring& sequence, const CommandEnviroment& env, CommandSequence* reusableTimer)
{
	size_t off = 0;
	bool semi_found = true;
	while (semi_found)
	{
		size_t semicolon = sequence.find_first_of(';', off);
		if (semicolon == std::wstring::npos)
		{
			semicolon = sequence.size();
			semi_found = false;
		}
		std::wstring command = sequence.substr(off, semicolon - off);
		TrimSpaces(command);
		
		if (!_wcsnicmp(command.c_str(), L"w8 ", 3))
		{
			size_t seconds;
			if (semicolon != sequence.size() && ToNumber(command.substr(3), seconds))
			{
				if (reusableTimer != nullptr)
				{
					reusableTimer->DueTime = seconds * 1000;
					reusableTimer->Command = sequence.substr(semicolon + 1);
					ThreadPool.UpdateTimer(reusableTimer, CommandTimerDue);
				}
				else
				{
					Enviroment env_type = env.type != Enviroment::INIT ? env.type : Enviroment::SCRIPT;
					auto NewBuff = std::make_unique<CommandSequence>(seconds * 1000, env_type, env.sPlayerIndex, sequence.substr(semicolon + 1));
					ThreadPool.AddTimer(std::move(NewBuff), CommandTimerDue);
				}
				return false;
			}
		}
		else if (!_wcsnicmp(command.c_str(), L"wait ", 5))
		{
			size_t milliseconds;
			if (semicolon != sequence.size() && ToNumber(command.substr(5), milliseconds))
			{
				if (reusableTimer != nullptr)
				{
					reusableTimer->DueTime = milliseconds;
					reusableTimer->Command = sequence.substr(semicolon + 1);
					ThreadPool.UpdateTimer(reusableTimer, CommandTimerDue);
				}
				else
				{
					Enviroment env_type = env.type != Enviroment::INIT ? env.type : Enviroment::SCRIPT;
					auto NewBuff = std::make_unique<CommandSequence>(milliseconds, env_type, env.sPlayerIndex, sequence.substr(semicolon + 1));
					ThreadPool.AddTimer(std::move(NewBuff), CommandTimerDue);
				}
				return false;
			}
		}
		else
		{
			ExecuteSingleCommand(command, env);
		}
		off = semicolon + 1;
	}
	return true;
}

int ExecuteSingleCommand(std::wstring& command, const CommandEnviroment& env, bool block)
{
	if (command.empty())
		return 1;

	WStrReplace(env.sPlayerIndex, command);

	ArgContainer args(command);

	stats::Commands++;

	int ret = GlobalCommandFactory.ExecuteCommand(args, env, block);
	if (ret == -2)
	{
		if (block)
			return 1;

		if (env.PrivilegeLevel < 4) // Only level 4 admins have right to execute Halo commands
		{
			env.Print("You do not have the rights to execute this command!");
			return -1;
		}

		if (env.type == Enviroment::RCON) // Let Halo's rcon handler execute the command
			return 2;

		if (env.type == Enviroment::CONSOLE && !env.FromSequence)
		{
			PrintConsole(FormatArgs("Unknown command: %hs", WToStr(command).c_str()).c_str());
			return 1;
		}

		if (command.size() > 254)
			command.resize(254);

		return ExecuteHaloCommand(env.sPlayerIndex, env.type, WToStr(command).c_str()); // Trying to execute a Halo (or an unknown) command from the chat, a script, init.txt or from remote console
	}
	else
	{
		return ret;
	}
}