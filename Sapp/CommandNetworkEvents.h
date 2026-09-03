#pragma once

#include "Addresses.h"
#include "GenericCommand.h"
#include "NetworkEvents.h"

class CommandGameSpeed final : public GenericCommand
{
	float speed;
	bool set;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 1)
		{
			set = false;
			return true;
		}
		else if (args.argc == 2)
		{
			if (ToNumber(args[1], speed))
			{
				set = true;
				return true;
			}			
		}
		return false;
	}

public:
	CommandGameSpeed() : GenericCommand(L"gamespeed", L"[value]", 4, ServerOnly::no, ClientOnly::no, Naughty::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{		
		if (set)
		{
			DWORD oldProtect;
			VirtualProtect(GameSpeed, 4, PAGE_READWRITE, &oldProtect);
			*GameSpeed = speed;
			VirtualProtect(GameSpeed, 4, oldProtect, &oldProtect);
			NetworkEvent<DataSync<float>>(NE_DataSync_GameSpeed, speed);
		}
		env_Print(FormatArgs("Game Speed: %.2f", *GameSpeed).c_str());
		return true;
	}
};

class CommandGravity final : public GenericCommand
{
	float gravity;
	bool set;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 1)
		{
			set = false;
			return true;
		}
		else if (args.argc == 2)
		{
			if (ToNumber(args[1], gravity))
			{
				set = true;
				return true;
			}
		}
		return false;
	}

public:
	CommandGravity() : GenericCommand(L"gravity", L"[value]", 4, ServerOnly::no, ClientOnly::no, Naughty::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (set)
		{
			DWORD oldProtect;
			VirtualProtect(GameSpeed, 4, PAGE_READWRITE, &oldProtect);
			*Gravity = gravity;
			VirtualProtect(GameSpeed, 4, oldProtect, &oldProtect);
			NetworkEvent<DataSync<float>>(NE_DataSync_Gravity, gravity);
		}
		env_Print(FormatArgs("Gravity: %f", *Gravity).c_str());
		return true;
	}
};