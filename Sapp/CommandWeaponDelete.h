#pragma once

#include "ObjectManagement.h"
#include "PlayerCommand.h"

class CommandWeaponDelete final : public PlayerCommand
{
	size_t what;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc < 3)
		{
			what = 5;
			return true;
		}
		if (args.argc == 3)
		{
			if (ToNumber(args[2], what) && what <= 5)
				return true;
		}
		return false;
	}

public:
	CommandWeaponDelete() : PlayerCommand(L"wdel", L"[weapon index(=5, 0-5)]", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;
		if (what == 0)
		{
			if (Player->WeaponIndexNum != 0xffff)
			{
				DWORD Obj = Player->WeaponID << 0x10 | Player->WeaponIndexNum;
				DestroyObject(Obj);
			}
		}
		else
		{
			if ((what == 5 || what == 1) && Player->PrimaryWeaponIndex != 0xffff)
			{
				DWORD Obj = Player->PrimaryWeaponID << 0x10 | Player->PrimaryWeaponIndex;
				DestroyObject(Obj);
			}
			if ((what == 5 || what == 2) && Player->SecondaryWeaponIndex != 0xffff)
			{
				DWORD Obj = Player->SecondaryWeaponID << 0x10 | Player->SecondaryWeaponIndex;
				DestroyObject(Obj);
			}
			if ((what == 5 || what == 3) && Player->TertiaryWeaponIndex != 0xffff)
			{
				DWORD Obj = Player->TertiaryWeaponID << 0x10 | Player->TertiaryWeaponIndex;
				DestroyObject(Obj);
			}
			if ((what == 5 || what == 4) && Player->QuaternaryWeaponIndex != 0xffff)
			{
				DWORD Obj = Player->QuaternaryWeaponID << 0x10 | Player->QuaternaryWeaponIndex;
				DestroyObject(Obj);
			}
		}

		env_Print(FormatWArgs(L"The selected weapon(s) have been removed from %ls.", PlayerTable[PlayerIndex].name).c_str());

		return true;
	}
};