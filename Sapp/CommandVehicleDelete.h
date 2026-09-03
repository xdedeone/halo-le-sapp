#pragma once

#include "GenericCommand.h"
#include "ObjectManagement.h"
#include "PlayerCommand.h"

class CommandVehicleDelete final : public PlayerCommand
{
public:
	CommandVehicleDelete() : PlayerCommand(L"vdel", L"", 4, Naughty::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		size_t n = 0;
		while (n < SpawnedVehicles.size())
		{
			if (SpawnedVehicles[n].PlayerIndex == PlayerIndex)
			{
				DestroyObject(SpawnedVehicles[n].ObjectID);
				SpawnedVehicles.erase(SpawnedVehicles.begin() + n);
			}
			else n++;
		}

		env_Print(FormatWArgs(L"The vehicle(s) assigned to %ls were deleted.", PlayerTable[PlayerIndex].name).c_str());

		return true;
	}
};

class CommandVehicleDeleteAll final : public GenericCommand
{
public:
	CommandVehicleDeleteAll() : GenericCommand(L"vdel_all", L"", 4, ServerOnly::yes, ClientOnly::no, Naughty::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		for (size_t i = 0; i < SpawnedVehicles.size(); i++)
		{
			DestroyObject(SpawnedVehicles[i].ObjectID);
		}

		SpawnedVehicles.clear();

		env_Print("All the manually spawned vehicles were deleted.");

		return true;
	}
};