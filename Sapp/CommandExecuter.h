#pragma once

#include <string>

#include "CommandEnviroment.h"

struct CommandSequence final : public TimerStructBase
{
	Enviroment Env;
	size_t PlayerIndex;
	std::wstring Command;
	CommandSequence(DWORD DueTime, Enviroment Env, size_t PlayerIndex, std::wstring &&Command) : TimerStructBase(0, DueTime), Env(Env), PlayerIndex(PlayerIndex), Command(std::move(Command)) {}
};

extern int CommandExecuting;

bool ExecuteHaloCommand(size_t PlayerIndex, Enviroment type, const char* command);

/* eg 'sv_say "Hello \n!";sv_kick $n;kill *' */
bool ExecuteSequence(const std::wstring& sequence, const CommandEnviroment& env, CommandSequence* reusableTimer = nullptr);
/* eg 'sv_say "Hy there \n!"'*/
int ExecuteSingleCommand(std::wstring& command, const CommandEnviroment& env, bool block = false);