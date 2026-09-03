#pragma once

#include "PlayerCommand.h"

class CommandChangeColor final : public PlayerCommand
{
	bool setColor;
	short colorToSet;

public:
	CommandChangeColor() : PlayerCommand(L"color", L"[color index]", 4, Naughty::yes) {}

	bool ParseArgs(ArgContainer& args) override
	{
		if(args.argc < 3)
		{
			setColor = false;
			return true;
		}
		else if(args.argc == 3)
		{
			setColor = true;
			if(ToNumber(args[2], colorToSet))
				return true;
		}
		return false;
	}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if (setColor)
		{
			if (!*TeamGame)
			{
				PlayerTable[PlayerIndex].Prescene.color = colorToSet;					
			}
			else
			{
				env_Print("This function does not work in team game!");
				return true;
			}
		}

		const wchar_t* color;
		switch (PlayerTable[PlayerIndex].Prescene.color)
		{
		case 0:
			color = L"white";
			break;
		case 1:
			color = L"black";
			break;
		case 2:
			color = L"red";
			break;
		case 3:
			color = L"blue";
			break;
		case 4:
			color = L"gray";
			break;
		case 5:
			color = L"yellow";
			break;
		case 6:
			color = L"green";
			break;
		case 7:
			color = L"pink";
			break;
		case 8:
			color = L"purple";
			break;
		case 9:
			color = L"cyan";
			break;
		case 10:
			color = L"cobalt";
			break;
		case 11:
			color = L"orange";
			break;
		case 12:
			color = L"teal";
			break;
		case 13:
			color = L"sage";
			break;
		case 14:
			color = L"brown";
			break;
		case 15:
			color = L"tan";
			break;
		case 16:
			color = L"maroon";
			break;
		default:
			color = L"salmon";
		}

		if (setColor && !*TeamGame)
		{
			env_Print(FormatWArgs(L"Color %ls has been set for %ls!", color, PlayerTable[PlayerIndex].name).c_str());
			return true;
		}

		env_Print(FormatWArgs(L"%ls's color: %ls", PlayerTable[PlayerIndex].name, color).c_str());	

		return true;
	}
};