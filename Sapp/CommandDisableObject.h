#pragma once
#pragma warning(disable: 4800) // warning C4800: 'size_t' : forcing value to bool 'true' or 'false' (performance warning)

#include "GenericCommand.h"
#include "ObjectInteractions.h"
#include "Prefs.h"

class CommandDisableAllObjects final : public GenericCommand
{
	size_t team;
	bool disabled;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			if (!ToNumber(args[1], team) || team > 2)
				return false;

			size_t d;
			if (!ToNumber(args[2], d) || d > 1)
				return false;

			disabled = static_cast<bool>(d);
			return true;
		}
		return false;
	}

public:
	CommandDisableAllObjects() : GenericCommand(L"disable_all_objects", L"<team(0-2)> <disable(0-1)>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		char* t;
		if(team == 0)
		{
			GlobalPrefs->DisableObjectsRed = disabled;
			GlobalPrefs->DisableObjectsBlue = disabled;
			t = "both teams";
		}
		else if (team == 1)
		{
			GlobalPrefs->DisableObjectsRed = disabled;
			t = "the red team";
		}
		else
		{
			GlobalPrefs->DisableObjectsBlue = disabled;
			t = "the blue team";
		}

		env_Print(FormatArgs("Interaction with any object has been %hs for %hs.", disabled ? "disallowed" : "allowed", t).c_str());

		return true;
	}
};

class CommandDisableAllVehicles final : public GenericCommand
{
	size_t team;
	bool disabled;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			if (!ToNumber(args[1], team) || team > 2)
				return false;

			size_t d;
			if (!ToNumber(args[2], d) || d > 1)
				return false;

			disabled = static_cast<bool>(d);
			return true;
		}
		return false;
	}

public:
	CommandDisableAllVehicles() : GenericCommand(L"disable_all_vehicles", L"<team(0-2)> <disable(0-1)>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		char* t;
		if (team == 0)
		{
			GlobalPrefs->DisableVehiclesRed = disabled;
			GlobalPrefs->DisableVehiclesBlue = disabled;
			t = "both teams";
		}
		else if (team == 1)
		{
			GlobalPrefs->DisableVehiclesRed = disabled;
			t = "the red team";
		}
		else
		{
			GlobalPrefs->DisableVehiclesBlue = disabled;
			t = "the blue team";
		}

		env_Print(FormatArgs("Interaction with any vehicle has been %hs for %hs.", disabled ? "disallowed" : "allowed", t).c_str());

		return true;
	}
};

class CommandDisableObject final : public GenericCommand
{
	std::string object_name;
	size_t team;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			object_name = WToStr(args[1]);
			team = 0;
			return true;
		}
		else if (args.argc == 3)
		{
			object_name = WToStr(args[1]);
			if (ToNumber(args[2], team) && team <= 2)
				return true;
		}
		return false;
	}

public:
	CommandDisableObject() : GenericCommand(L"disable_object", L"<object name with path> [team(0-2)]") {}

	bool Execute(const CommandEnviroment& env) override
	{
		auto it = DisabledObjects.find(object_name);
		if (it != DisabledObjects.end())
		{
			if (team == 0 || team == 1)
				it->second.Red = true;
			if (team == 0 || team == 2)
				it->second.Blue = true;

			ObjectCache.clear();
		}
		else
		{
			ObjectUsage TB;

			if (team == 0 || team == 1)
				TB.Red = true;
			if (team == 0 || team == 2)
				TB.Blue = true;

			DisabledObjects[object_name] = TB;

			ObjectCache.clear();
		}

		char* t;
		if(team == 0)
			t = "both teams";
		else if (team == 1)
			t = "the red team";
		else
			t = "the blue team";

		env_Print(FormatArgs("Usage of object %hs has been forbidden for %hs.", object_name.c_str(), t).c_str());

		return true;
	}
};

class CommandEnableObject final : public GenericCommand
{
	std::string object_name;
	size_t object_id;
	size_t team;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			if (ToNumber(args[1], object_id))
			{
				object_name.clear();
			}
			else
			{
				object_name = WToStr(args[1]);
			}
			team = 0;
			return true;
		}
		else if (args.argc == 3)
		{
			if (ToNumber(args[1], object_id))
			{
				object_name.clear();
			}
			else
			{
				object_name = WToStr(args[1]);
			}
			if (ToNumber(args[2], team) && team <= 2)
				return true;
		}
		return false;
	}

public:
	CommandEnableObject() : GenericCommand(L"enable_object", L"<object name with path> [team(0-2)]\n<object id> [team(0-2)]") {}

	bool Execute(const CommandEnviroment& env) override
	{
		char* t;
		if (team == 0)
			t = "both teams";
		else if (team == 1)
			t = "the red team";
		else
			t = "the blue team";

		if (!object_name.empty())
		{
			auto it = DisabledObjects.find(object_name);
			if (it != DisabledObjects.end())
			{
				if (team == 0 || team == 1)
					it->second.Red = false;
				if (team == 0 || team == 2)
					it->second.Blue = false;
				RemoveIfEmpty(it);
				ObjectCache.clear();
				env_Print(FormatWArgs(L"Usage of object %hs has been allowed for %hs.", object_name.c_str(), t).c_str());
			}
			else
			{
				env_Print(FormatWArgs(L"Object %hs not found.", object_name.c_str()).c_str());
			}
		}
		else if (object_id >= DisabledObjects.size())
		{
			env_Print(FormatArgs("Object does not exist with ID '%u'.", object_id).c_str());
		}
		else
		{
			size_t i = 0;
			for (auto it = DisabledObjects.begin(); it != DisabledObjects.end(); ++it, i++)
			{
				if (i == object_id)
				{
					if (team == 0 || team == 1)
						it->second.Red = false;
					if (team == 0 || team == 2)
						it->second.Blue = false;
					RemoveIfEmpty(it);
					ObjectCache.clear();
					env_Print(FormatArgs("Usage of object %hs has been allowed for %hs.", it->first.c_str(), t).c_str());
					break;
				}
			}
		}

		return true;
	}
};

class CommandDisabledObjects final : public GenericCommand
{
public:
	CommandDisabledObjects() : GenericCommand(L"disabled_objects") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if(DisabledObjects.empty())
		{
			env_Print("There are no disabled objects.");
		}
		else
		{
			env_Print("ID Name");
			size_t i = 0;
			for (std::map <std::string, ObjectUsage>::iterator it = DisabledObjects.begin(); it != DisabledObjects.end(); ++it)
			{
				env_Print(FormatArgs("%2u %hs", i++, it->first.c_str()).c_str());
			}
		}

		return true;
	}
};