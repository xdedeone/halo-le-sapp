#pragma once

#include "EventScan.h"
#include "Locations.h"
#include "PlayerCommand.h"

class CommandTeleport final : public PlayerCommand
{
	std::wstring loc_name;
	vect3f loc;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			loc_name = std::move(args[2]);
			loc = vect3f();
			return true;
		}
		else if (args.argc == 5)
		{
			loc_name.clear();
			if (ToNumber(args[2], loc.x) && ToNumber(args[3], loc.y) && ToNumber(args[4], loc.z))
				return true;
		}
		return false;
	}

public:
	CommandTeleport() : PlayerCommand(L"t", L"<location name>\n<x> <y> <z>", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		if (!loc_name.empty())
		{
			size_t i = GlobalLocations.Find(loc_name);
			if (i != -1)
			{
				loc = GlobalLocations.Get(i);
			}
		}

		Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

		if (Player->VehicleIndexNum == 0xffff)
		{				
			env_Print(FormatWArgs(L"Teleporting %ls to %4.2f %4.2f %4.2f", PlayerTable[PlayerIndex].name, loc.x, loc.y, loc.z).c_str());
			Player->PlayerVectors.Pos = loc;
		}
		else
		{
			DWORD VehicleAddr = (DWORD)ObjectTable[Player->VehicleIndexNum].Offset;
				    					
			env_Print(FormatWArgs(L"Teleporting %ls's vehicle to %4.2f %4.2f %4.2f", PlayerTable[PlayerIndex].name, loc.x, loc.y, loc.z).c_str());
			*(vect3f*)(VehicleAddr + 0x5c) = loc;
		}

		Teleported[PlayerIndex] = true;
		PlayerTable[PlayerIndex].LastSync = 0;
		PlayerTable[PlayerIndex].LastSyncToOthers = 0;
		PlayerTable[PlayerIndex].LastVehicleSyncToOthers = 0;

		return true;
	}
};

class CommandMove: public PlayerCommand
{
	vect3f loc;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 5)
		{
			if (ToNumber(args[2], loc.x) && ToNumber(args[3], loc.y) && ToNumber(args[4], loc.z))
				return true;
		}
		return false;
	}

public:
	CommandMove() : PlayerCommand(L"m", L"<x> <y> <z>", 4, Naughty::yes, AliveOnly::yes) {}
	
	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

		if (Player->VehicleIndexNum == 0xffff)
		{
			env_Print(FormatWArgs(L"Moving %ls by %4.2f %4.2f %4.2f", PlayerTable[PlayerIndex].name, loc.x, loc.y, loc.z).c_str());
			Player->PlayerVectors.Pos += loc;
		}
		else
		{
			DWORD VehicleAddr = (DWORD)ObjectTable[Player->VehicleIndexNum].Offset;

			env_Print(FormatWArgs(L"Moving %ls's vehicle by %4.2f %4.2f %4.2f", PlayerTable[PlayerIndex].name, loc.x, loc.y, loc.z).c_str());
			*(vect3f*)(VehicleAddr + 0x5c) += loc;
		}

		Teleported[PlayerIndex] = true;
		PlayerTable[PlayerIndex].LastSync = 0;
		PlayerTable[PlayerIndex].LastSyncToOthers = 0;
		PlayerTable[PlayerIndex].LastVehicleSyncToOthers = 0;

		return true;
	}
};

class CommandTeleportToPlayer: public PlayerCommand
{
	size_t player;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			if (ToNumber(args[2], player))
				return true;
		}
		return false;
	}

public:
	CommandTeleportToPlayer() : PlayerCommand(L"tp", L"<player index>", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

		int index = ToPlayerIndex(player);
		if (index == -1)
		{
			env_Print(FormatWArgs(L"Can't teleport %ls, target is not present.", PlayerTable[PlayerIndex].name).c_str());
			return true;
		}

		if (!PlayerAliveOnly(index))
		{
			env_Print(FormatWArgs(L"Can't teleport %ls, target is dead.", PlayerTable[PlayerIndex].name).c_str());
			return true;
		}

		Dynamic_Player *ToPlayer = ObjectTable[PlayerTable[index].ObjectIndexNum].Offset;
		if (Player->VehicleIndexNum == 0xffff)
		{
			if (ToPlayer->VehicleIndexNum == 0xffff)
			{
				env_Print(FormatWArgs(L"Teleporting %ls to %ls.", PlayerTable[PlayerIndex].name, PlayerTable[index].name).c_str());
				Player->PlayerVectors.Pos = ToPlayer->PlayerVectors.Pos;
				Player->PlayerVectors.Pos.z += 1;
			}
			else
			{
				DWORD ToVehicleAddr = (DWORD)ObjectTable[ToPlayer->VehicleIndexNum].Offset;

				env_Print(FormatWArgs(L"Teleporting %ls to %ls's vehicle.", PlayerTable[PlayerIndex].name, PlayerTable[index].name).c_str());
				Player->PlayerVectors.Pos = *(vect3f*)(ToVehicleAddr + 0x5c);
			}
		}
		else
		{
			DWORD VehicleAddr = (DWORD)ObjectTable[Player->VehicleIndexNum].Offset;

			if (ToPlayer->VehicleIndexNum == 0xffff)
			{
				env_Print(FormatWArgs(L"Teleporting %ls's vehicle to %ls.", PlayerTable[PlayerIndex].name, PlayerTable[index].name).c_str());
				*(vect3f*)(VehicleAddr + 0x5c) = ToPlayer->PlayerVectors.Pos;
			}
			else
			{
				DWORD ToVehicleAddr = (DWORD)ObjectTable[ToPlayer->VehicleIndexNum].Offset;

				env_Print(FormatWArgs(L"Teleporting %ls's vehicle to %ls's vehicle.", PlayerTable[PlayerIndex].name, PlayerTable[index].name).c_str());
				*(vect3f*)(VehicleAddr + 0x5c) = *(vect3f*)(ToVehicleAddr + 0x5c);
				((vect3f*)(VehicleAddr + 0x5c))->z += 1;
			}
		}

		Teleported[PlayerIndex] = true;
		PlayerTable[PlayerIndex].LastSync = 0;
		PlayerTable[PlayerIndex].LastSyncToOthers = 0;
		PlayerTable[PlayerIndex].LastVehicleSyncToOthers = 0;

		return true;
	}
};