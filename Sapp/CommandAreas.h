#pragma once

#include "Addresses.h"
#include "Areas.h"
#include "EventScan.h"
#include "GenericCommand.h"

class CommandAreaAddSphere final : public GenericCommand
{
	std::wstring area_name;
	vect3f pos;
	float rad;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 6)
		{
			area_name = std::move(args[1]);
			if (!ToNumber(args[2], pos.x))
				return false;
			if (!ToNumber(args[3], pos.y))
				return false;
			if (!ToNumber(args[4], pos.z))
				return false;
			if (ToNumber(args[5], rad))
				return true;
		}
		return false;
	}

public:
	CommandAreaAddSphere() : GenericCommand(L"area_add_sphere", L"<area name> <x> <y> <z> <radius>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (GlobalAreas.Find(area_name) != -1)
		{
			env_Print(FormatWArgs(L"Area %ls already exists for %hs.", area_name.c_str(), CurrentMap.c_str()).c_str());
			return false;
		}

		GlobalAreas.AddSphere(area_name, pos, rad);
		env_Print(FormatWArgs(L"Area %ls has been added to %hs.", area_name.c_str(), CurrentMap.c_str()).c_str());

		return true;
	}
};

class CommandAreaAddCuboid final : public GenericCommand
{
	std::wstring area_name;
	vect3f posA;
	vect3f posB;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 8)
		{
			area_name = std::move(args[1]);
			if (!ToNumber(args[2], posA.x))
				return false;
			if (!ToNumber(args[3], posA.y))
				return false;
			if (!ToNumber(args[4], posA.z))
				return false;
			if (!ToNumber(args[5], posB.x))
				return false;
			if (!ToNumber(args[6], posB.y))
				return false;
			if (ToNumber(args[7], posB.z))
				return true;
		}
		return false;
	}

public:
	CommandAreaAddCuboid() : GenericCommand(L"area_add_cuboid", L"<area name> <Ax> <Ay> <Az> <Bx> <By> <Bz>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (GlobalAreas.Find(area_name) != -1)
		{
			env_Print(FormatWArgs(L"Area %ls already exists for %hs.", area_name.c_str(), CurrentMap.c_str()).c_str());
			return false;
		}

		GlobalAreas.AddCuboid(area_name, posA, posB);
		env_Print(FormatWArgs(L"Area %ls has been added to %hs.", area_name.c_str(), CurrentMap.c_str()).c_str());

		return true;
	}
};

class CommandAreaDel final : public GenericCommand
{
	std::wstring area_name;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			area_name = std::move(args[1]);
			return true;
		}
		return false;
	}

public:
	CommandAreaDel() : GenericCommand(L"area_del", L"<area name>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		size_t i = GlobalAreas.Find(area_name);
		if (i != -1)
		{
			GlobalAreas.Del(i);
			env_Print(FormatWArgs(L"Removed %ls from %hs.", area_name.c_str(), CurrentMap.c_str()).c_str());
		}
		else
		{
			env_Print(FormatWArgs(L"Area %ls does not exists for %hs.", area_name.c_str(), CurrentMap.c_str()).c_str());
		}

		return true;
	}
};

class CommandAreaList final : public GenericCommand
{
public:
	CommandAreaList() : GenericCommand(L"area_list") {}

	bool Execute(const CommandEnviroment& env) override
	{
		env.StreamPrint(GlobalAreas.StreamOut());
		return true;
	}
};

class CommandAreaListAll final : public GenericCommand
{
public:
	CommandAreaListAll() : GenericCommand(L"area_listall") {}

	bool Execute(const CommandEnviroment& env) override
	{
		env.StreamPrint(GlobalAreas.StreamOutAll());
		return true;
	}
};