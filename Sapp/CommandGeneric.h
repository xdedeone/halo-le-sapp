#pragma once

#include "CommandBase.h"
#include "ArgContainer.h"
#include "globals.h"

const char * NoPlayers = "There are no players in the server.";
const char * DrunkAdmin = "Are you drunk?! Ignoring command: drunk admin";

class CommandGeneric : public CommandBase
{
public:
	//vector <string> specialArguments;
	std::string commandString;

	bool ServerOnly;
	int CommandLevel;

	CommandGeneric()
	{
		this->ServerOnly = false;
		this->CommandLevel = 4;
	}

	//virtual bool CanExecute(ArgContainer &args, SappCommand *env) = 0;

	virtual bool Execute(ArgContainer &args, SappCommand *env) = 0;
	//virtual bool ExecuteSpecialCommand(ArgContainer &args, SappCommand *env)
	//{
	//	return false;
	//}
};