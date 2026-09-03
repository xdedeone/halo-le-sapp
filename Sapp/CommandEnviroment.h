#pragma once

#include <sstream>

#include "SuperHeader.h"

#define env_Print if(env.type != Enviroment::SCRIPT || ScriptEcho) env.Print

extern bool ScriptEcho;

class CommandEnviroment final
{
public:
	size_t sPlayerIndex;
	Enviroment type;
	int PrivilegeLevel;
	bool FromSequence;

	void Print(const char* message) const;
	void Print(const wchar_t* message) const;

	void StreamPrint(const std::wostringstream& wos) const;
    
	CommandEnviroment(Enviroment env_type, size_t PlayerIndex = -1, int Level = 4, bool Sequence = false) : type(env_type), sPlayerIndex(PlayerIndex), PrivilegeLevel(Level), FromSequence(Sequence) {}
};