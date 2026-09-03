#pragma once

#include "GenericCommand.h"
#include "Locations.h"
#include "ObjectManagement.h"

class CommandSpawnObject final : public GenericCommand
{
	std::string tag_type;
	std::string tag_name;
	size_t player_index;
	vect3f loc;
	float rot;
	std::wstring loc_name;
	size_t usage_case;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			tag_type = WToStr(args[1]);
			tag_name = WToStr(args[2]);
			player_index = -1;
			loc = vect3f();
			rot = 0;
			usage_case = 0;
			return true;
		}
		else if (args.argc == 4 || args.argc == 5)
		{
			tag_type = WToStr(args[1]);
			tag_name = WToStr(args[2]);
			loc = vect3f();
			if (ToNumber(args[3], player_index))
			{
				loc_name.clear();
			}
			else
			{
				player_index = -1;
				loc_name = std::move(args[3]);
			}
			if (args.argc == 4)
			{
				rot = 0;
			}
			else
			{
				if (!ToNumber(args[4], rot))
					return false;
			}
			usage_case = 1;
			return true;
		}
		else if (args.argc >= 6)
		{
			tag_type = WToStr(args[1]);
			tag_name = WToStr(args[2]);
			player_index = -1;
			if (!ToNumber(args[3], loc.x))
				return false;
			if (!ToNumber(args[4], loc.y))
				return false;
			if (!ToNumber(args[5], loc.z))
				return false;
			if (args.argc > 6)
			{
				if (!ToNumber(args[6], rot))
					return false;
			}
			else
			{
				rot = 0;
			}
			usage_case = 2;
			return true;
		}
		return false;
	}

public:
	CommandSpawnObject() : GenericCommand(L"spawn", L"<tag type> <tag name> [player index] [rotation]\n<tag type> <tag name> [location name] [rotation]\n<tag type> <tag name> [<x> <y> <z>] [rotation]", 4, ServerOnly::yes, ClientOnly::no, Naughty::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (usage_case == 0)
		{
			if(PlayerAlive(env.sPlayerIndex))
			{
				player_index = env.sPlayerIndex;
			}
			else
			{
				for (size_t i = 0; i < 16; i++)
				{
					if (PlayerAlive(i))
					{
						player_index = i;
						break;
					}
				}
			}
		}
		else if (usage_case == 1)
		{
			if (loc_name.empty())
			{
				player_index = NumberToIndex(player_index - 1);
			}
			else
			{
				size_t index = GlobalLocations.Find(loc_name);
				if (index != -1)
					loc = GlobalLocations.Get(index);
			}	
		}
	
		if(PlayerAlive(player_index))
			GetPlayerPosition(player_index, loc);

		DWORD NewObj = SpawnObject(tag_type.c_str(), tag_name.c_str(), loc, rot, player_index);
		if(NewObj != 0xFFFFFFFF)
		{
			env_Print(FormatWArgs(L"Successfully spawned a %hs.%hs", tag_name.c_str(), tag_type.c_str()).c_str());
		}
		else
		{
			env_Print(FormatWArgs(L"Failed to spawn a %hs.%hs", tag_name.c_str(), tag_type.c_str()).c_str());
		}

		return true;
	}
};