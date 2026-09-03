#pragma once

#include "PlayerCommand.h"

class CommandPlayerDescription final : public PlayerCommand 
{
public:
	CommandPlayerDescription() : PlayerCommand(L"d", L"", 4, Naughty::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		size_t MachineIndex = IndexToNumber(PlayerIndex);
		env_Print(FormatWArgs(L"Name: %ls, Team: %ls, Privilege level: %d", PlayerTable[PlayerIndex].name, PlayerTable[PlayerIndex].Team == 0 ? L"red" : L"blue", GlobalOnlineAdminList.GetAdminLevel(PlayerIndex)).c_str());
		env_Print(FormatArgs("PlayerTable Index: %u, MachineInfo Index: %u", PlayerIndex, MachineIndex).c_str());

		if(PlayerAliveOnly(PlayerIndex))
		{
			Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

			if (GlobalPrefs->DebugStrings)
			{
				env_Print(FormatArgs("Player: %08X Index: %08X", Player, PlayerTable[PlayerIndex].ObjectIndex).c_str());

				if (Player->WeaponIndexNum != 0xFFFF)
				{
					Dynamic_Player *Weapon = ObjectTable[Player->WeaponIndexNum].Offset;
					env_Print(FormatArgs("Weapon: %08X Index: %08X", Weapon, Player->WeaponIndex).c_str());
				}

				if(Player->VehicleIndexNum != 0xFFFF)
				{
					Dynamic_Player *Vehicle = ObjectTable[Player->VehicleIndexNum].Offset;
					env_Print(FormatArgs("Vehicle: %08X Index: %08X", Vehicle, Player->VehicleIndex).c_str());
				}	

				if (Player->VehicleWeaponIndexNum != 0xFFFF)
				{
					Dynamic_Player *VehicleWeapon = ObjectTable[Player->VehicleWeaponIndexNum].Offset;
					env_Print(FormatArgs("Vehicle Weapon: %08X Index: %08X", VehicleWeapon, Player->VehicleWeaponIndex).c_str());
				}
			}

			//char *PWeaponAddr = (char*)ObjectTableArray[Player->PrimaryWeaponIndex].Offset;
			//char *SWeaponAddr = (char*)ObjectTableArray[Player->SecondaryWeaponIndex].Offset;
			//env_Print("Weapon: 0x%08X, Weapon: 0x%08X, Weapon: 0x%08X", WeaponAddr, PWeaponAddr, SWeaponAddr);
			//env_Print("Machine Address: 0x%08X", Server->MachineInfo[IndexToNumber(PlayerIndex)].Address);
			env_Print(FormatArgs("Shield: %4.2f Health: %4.2f Speed: %4.2f", Player->Shield, Player->Health, PlayerTable[PlayerIndex].speed).c_str());
			env_Print(FormatArgs("%hs", Player->IsInvisible == 0x51 ? "Player is invisible" : "Player is not invisible").c_str());
			vect3f loc;
			GetPlayerPosition(PlayerIndex, loc);
			env_Print(FormatArgs("x: %4.2f y: %4.2f z: %4.2f", loc.x, loc.y, loc.z).c_str());
			//env_Print(FormatArgs("x: %.4f y: %.4f z: %.4f", Player->Camera.x, Player->Camera.y, Player->Camera.z).c_str());
			//env_Print(FormatArgs("xy angle: %f z angle: %f", Server->MachineInfo[MachineIndex].angle_xy, Server->MachineInfo[MachineIndex].angle_z).c_str());
		}
		else
		{
			env_Print(FormatWArgs(L"%ls is dead.", PlayerTable[PlayerIndex].name).c_str());
		}
		return true;
	}
};


class CommandCoord final : public PlayerCommand 
{
public:
	CommandCoord() : PlayerCommand(L"coord", L"", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		vect3f loc;
		GetPlayerPosition(PlayerIndex, loc);
		env_Print(FormatWArgs(L"%ls's coordinates: x: %4.2f y: %4.2f z: %4.2f", PlayerTable[PlayerIndex].name, loc.x, loc.y, loc.z).c_str());
		return true;
	}
};

class CommandInf final : public PlayerCommand 
{
public:
	CommandInf() : PlayerCommand(L"inf", L"", 3) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		env_Print(FormatWArgs(L"Name: %ls  Index: %d  IP: %hs  CD-Key Hash: %hs", PlayerTable[PlayerIndex].name, Server->PlayerData[PlayerIndex].machine_index + 1, GetPlayerIP(IndexToNumber(PlayerIndex)).c_str(), GetPlayerHash(IndexToNumber(PlayerIndex)).c_str()).c_str());
		return true;
	}
};