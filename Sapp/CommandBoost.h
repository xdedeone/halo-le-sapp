#pragma once

#include "PlayerCommand.h"

extern bool Teleported[16];

class CommandBoost final : public PlayerCommand
{
public:
	CommandBoost() : PlayerCommand(L"boost", L"", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{	
		Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

		vect3f pos;
		GetPlayerPosition(PlayerIndex, pos);
		pos.z += 0.6f; // camera

		vect3f trans = Player->Camera * 1000.0f;
		vect3f out;
		DWORD unit;
		if (Intersect(PlayerTable[PlayerIndex].ObjectIndex, pos, trans, out, unit))
		{
			if (Player->VehicleIndexNum == 0xffff)
			{
				Player->PlayerVectors.Pos = out;
				env_Print(FormatWArgs(L"Boosted %ls.",  PlayerTable[PlayerIndex].name).c_str());
				PlayerTable[PlayerIndex].LastSyncToOthers = 0;
			}
			else
			{
				DWORD VehicleAddr = (DWORD)ObjectTable[Player->VehicleIndexNum].Offset;
				*(vect3f*)(VehicleAddr + 0x5c) = out;
				env_Print(FormatWArgs(L"Boosted %ls's vehicle.",  PlayerTable[PlayerIndex].name).c_str());
				PlayerTable[PlayerIndex].LastVehicleSyncToOthers = 0;
			}
			Teleported[PlayerIndex] = true;
			PlayerTable[PlayerIndex].LastSync = 0;
		}
		return true;
	}
};