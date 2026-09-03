#pragma once

#include "GenericCommand.h"
#include "LogThread.h"

void Load();
void Unload();

class CommandReload final : public GenericCommand
{
public:
	CommandReload() : GenericCommand(L"reload") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if (env.type != Enviroment::INIT && env.type != Enviroment::SCRIPT && env.type != Enviroment::REMOTE)
		{
			Unload();

			Load();

			WriteLog(L"RELOAD", L"SAPP was reloaded!");

			sv_say(L"SAPP was reloaded!");

			PrintConsole(10, "SAPP was reloaded!");
			return true;
		}
		else
		{
			env.Print(L"This command can't be executed from here!");
			return false;
		}
	}
};