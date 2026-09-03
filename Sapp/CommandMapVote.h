#pragma once

#include "GenericCommand.h"
#include "MapVote.h"

class CommandMapVotes final : public GenericCommand
{
public:
	CommandMapVotes() : GenericCommand(L"mapvotes") {}

	bool Execute(const CommandEnviroment& env) override
	{
		env.StreamPrint(GlobalMapVotes.StreamOut());
		return true;
	}
};

class CommandMapVoteAdd final : public GenericCommand
{
	std::wstring map;
	std::wstring mode;
	std::wstring description;
	size_t min;
	size_t max;
	size_t pos;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 4)
		{
			map = move(args[1]);
			mode = move(args[2]);
			description = move(args[3]);
			min = 0;
			max = 16;
			pos = -1;
			return true;
		}
		else if (args.argc == 5)
		{
			map = move(args[1]);
			mode = move(args[2]);
			description = move(args[3]);
			if (!ToNumber(args[4], min) || min > 16)
				return false;
			max = 16;
			pos = -1;
			return true;
		}
		else if (args.argc == 6)
		{
			map = move(args[1]);
			mode = move(args[2]);
			description = move(args[3]);
			if (!ToNumber(args[4], min) || min > 16)
				return false;
			if (!ToNumber(args[5], max) || max > 16)
				return false;
			pos = -1;
			return true;
		}
		else if (args.argc == 7)
		{
			map = move(args[1]);
			mode = move(args[2]);
			description = move(args[3]);
			if (!ToNumber(args[4], min) || min > 16)
				return false;
			if (!ToNumber(args[5], max) || max > 16)
				return false;
			if (!ToNumber(args[6], pos))
				return false;
			return true;
		}
		return false;
	}

public:
	CommandMapVoteAdd() : GenericCommand(L"mapvote_add", L"<map> <gametype> <description> [min players(=0, 0-16)] [max players(=16, 0-16)] [position]") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (!CheckMap(WToStr(map).c_str(), WToStr(mode).c_str()))
		{
			env_Print(FormatArgs("Invalid mapvote entry: \"%ls\" \"%ls\"", map.c_str(), mode.c_str()).c_str());
			return true;
		}

		if (!GlobalMapVotes.MapVoteAdd(move(map), move(mode), move(description), min, max, pos))
		{
			env_Print("Invalid position index!");
			return true;
		}

		env_Print("Successfully added the mapvote option!");
		GlobalMapVotes.SaveMapVotes();

		return true;
	}
};


class CommandMapVoteDel final : public GenericCommand
{
	size_t mapvote_index;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			if (ToNumber(args[1], mapvote_index))
				return true;
		}
		return false;
	}

public:
	CommandMapVoteDel() : GenericCommand(L"mapvote_del", L"<mapvote index>") {}
	
	bool Execute(const CommandEnviroment& env) override
	{
		if(!GlobalMapVotes.MapVoteDel(mapvote_index))
		{
			env_Print("Incorrect mapvote index!");
			return true;
		}

		env_Print("Successfully removed the mapvote entry!");
		GlobalMapVotes.SaveMapVotes();

		return true;
	}
};


class CommandMapvoteBegin final : public GenericCommand
{
public:
	CommandMapvoteBegin() : GenericCommand(L"mapvote_begin") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (!GlobalMapVotes.Launch())
		{
			env_Print("There are no mapvotes!");
		}
		else if (!GlobalPrefs->MapVote)
		{
			GlobalPrefs->MapVote = true;
			env_Print("Mapvote has been automatically enabled!");
			if (GlobalPrefs->SappMapcycle)
			{
				GlobalPrefs->SappMapcycle = false;
				GlobalPrefs->Flags &= ~4;
				env_Print("Sapp mapcycle is now disabled.");
			}
		}

		return true;
	}
};