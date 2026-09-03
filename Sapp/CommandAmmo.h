#pragma once

#include "ObjectManagement.h"
#include "PlayerCommand.h"

class CommandAmmo final : public PlayerCommand
{
	std::wstring statsToSet;
	size_t what;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			statsToSet = std::move(args[2]);
			what = 0;
			return true;

		}
		else if (args.argc == 4)
		{
			statsToSet = std::move(args[2]);
			if (ToNumber(args[3], what) && what <= 5)
				return true;

		}
		return false;
	}

public:
	CommandAmmo() : PlayerCommand(L"ammo", L"<amount> [weapon index(=0, 0-5)]", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		Dynamic_Player* Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

		if(what == 0)
		{					
			if (Player->WeaponIndexNum == 0xFFFF)
			{
				env_Print("Error: Player doesn't hold any weapon.");
			}
			else
			{
				DWORD WeaponAddr = (DWORD)ObjectTable[Player->WeaponIndexNum].Offset;
				ProcessOperator(statsToSet, *(short*)(WeaponAddr + 0x2b6));
				*(byte*)(WeaponAddr + 0x13) |= 0x04;
			}
		}
		else
		{
			if(Player->PrimaryWeaponIndex != 0xffff && (what == 5 || what == 1))
			{
				DWORD PWeaponAddr = (DWORD)ObjectTable[Player->PrimaryWeaponIndex].Offset;
				ProcessOperator(statsToSet, *(short*)(PWeaponAddr + 0x2b6));
				*(byte*)(PWeaponAddr + 0x13) |= 0x04;
			}
			if(Player->SecondaryWeaponIndex != 0xffff && (what == 5 || what == 2))
			{
				DWORD SWeaponAddr = (DWORD)ObjectTable[Player->SecondaryWeaponIndex].Offset;
				ProcessOperator(statsToSet, *(short*)(SWeaponAddr + 0x2b6));
				*(byte*)(SWeaponAddr + 0x13) |= 0x04;
			}
			if(Player->TertiaryWeaponIndex != 0xffff && (what == 5 || what == 3))
			{
				DWORD TWeaponAddr = (DWORD)ObjectTable[Player->TertiaryWeaponIndex].Offset;
				ProcessOperator(statsToSet, *(short*)(TWeaponAddr + 0x2b6));
				*(byte*)(TWeaponAddr + 0x13) |= 0x04;
			}
			if(Player->QuaternaryWeaponIndex!= 0xffff && (what == 5 || what == 4))
			{
				DWORD QWeaponAddr = (DWORD)ObjectTable[Player->QuaternaryWeaponIndex].Offset;
				ProcessOperator(statsToSet, *(short*)(QWeaponAddr + 0x2b6));
				*(byte*)(QWeaponAddr + 0x13) |= 0x04;
			}
		}

		env_Print(FormatWArgs(L"Successfully set ammo of %ls's weapon(s).", PlayerTable[PlayerIndex].name).c_str());

		return true;
	}
};

class CommandMag final : public PlayerCommand
{
	std::wstring statsToSet;
	size_t what;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			statsToSet = std::move(args[2]);
			what = 0;
			return true;

		}
		else if (args.argc == 4)
		{
			statsToSet = std::move(args[2]);
			if (ToNumber(args[3], what) && what <= 5)
				return true;

		}
		return false;
	}

public:
	CommandMag() : PlayerCommand(L"mag", L"<amount> [weapon index(=0, 0-5)]", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		Dynamic_Player* Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

		if (what == 0)
		{
			if (Player->WeaponIndexNum == 0xFFFF)
			{
				env_Print("Error: Player doesn't hold any weapon.");
			}
			else
			{
				DWORD WeaponAddr = (DWORD)ObjectTable[Player->WeaponIndexNum].Offset;
				ProcessOperator(statsToSet, *(short*)(WeaponAddr + 0x2b8));
				SyncAmmo((Player->WeaponID << 0x10) | Player->WeaponIndexNum);
			}
		}
		else
		{
			if (Player->PrimaryWeaponIndex != 0xffff && (what == 5 || what == 1))
			{
				DWORD PWeaponAddr = (DWORD)ObjectTable[Player->PrimaryWeaponIndex].Offset;
				ProcessOperator(statsToSet, *(short*)(PWeaponAddr + 0x2b8));
				SyncAmmo((Player->PrimaryWeaponID << 0x10) | Player->PrimaryWeaponIndex);
			}
			if (Player->SecondaryWeaponIndex != 0xffff && (what == 5 || what == 2))
			{
				DWORD SWeaponAddr = (DWORD)ObjectTable[Player->SecondaryWeaponIndex].Offset;
				ProcessOperator(statsToSet, *(short*)(SWeaponAddr + 0x2b8));
				SyncAmmo((Player->SecondaryWeaponID << 0x10) | Player->SecondaryWeaponIndex);
			}
			if (Player->TertiaryWeaponIndex != 0xffff && (what == 5 || what == 3))
			{
				DWORD TWeaponAddr = (DWORD)ObjectTable[Player->TertiaryWeaponIndex].Offset;
				ProcessOperator(statsToSet, *(short*)(TWeaponAddr + 0x2b8));
				SyncAmmo((Player->TertiaryWeaponID << 0x10) | Player->TertiaryWeaponIndex);
			}
			if (Player->QuaternaryWeaponIndex != 0xffff && (what == 5 || what == 4))
			{
				DWORD QWeaponAddr = (DWORD)ObjectTable[Player->QuaternaryWeaponIndex].Offset;
				ProcessOperator(statsToSet, *(short*)(QWeaponAddr + 0x2b8));
				SyncAmmo((Player->QuaternaryWeaponID << 0x10) | Player->QuaternaryWeaponIndex);
			}
		}

		env_Print(FormatWArgs(L"Successfully set mags of %ls's weapon(s).", PlayerTable[PlayerIndex].name).c_str());

		return true;
	}
};

class CommandBattery final : public PlayerCommand
{
	std::wstring statsToSet;
	size_t what;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			statsToSet = std::move(args[2]);
			what = 0;
			return true;

		}
		else if (args.argc == 4)
		{
			statsToSet = std::move(args[2]);
			if (ToNumber(args[3], what) && what <= 5)
				return true;

		}
		return false;
	}

public:
	CommandBattery() : PlayerCommand(L"battery", L"<amount(%)> [weapon index(=0, 0-5)]", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		Dynamic_Player* Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

		float tmp;
		if (what == 0)
		{
			if (Player->WeaponIndexNum == 0xFFFF)
			{
				env_Print("Error: Player doesn't hold any weapon.");
			}
			else
			{
				DWORD WeaponAddr = (DWORD)ObjectTable[Player->WeaponIndexNum].Offset;
				if (WeaponAddr != NULL)
				{
					tmp = (1.0f - *(float*)(WeaponAddr + 0x240)) * 100.0f;
					ProcessOperator(statsToSet, tmp);
					*(float*)(WeaponAddr + 0x240) = 1.0f - tmp * 0.01f;
					*(byte*)(WeaponAddr + 0x13) |= 0x04;
				}
			}
		}
		else
		{
			if (Player->PrimaryWeaponIndex != 0xffff && (what == 5 || what == 1))
			{
				DWORD PWeaponAddr = (DWORD)ObjectTable[Player->PrimaryWeaponIndex].Offset;
				tmp = (1.0f - *(float*)(PWeaponAddr + 0x240)) * 100.0f;
				ProcessOperator(statsToSet, tmp);
				*(float*)(PWeaponAddr + 0x240) = 1.0f - tmp * 0.01f;
				*(byte*)(PWeaponAddr + 0x13) |= 0x04;
			}
			if (Player->SecondaryWeaponIndex != 0xffff && (what == 5 || what == 2))
			{
				DWORD SWeaponAddr = (DWORD)ObjectTable[Player->SecondaryWeaponIndex].Offset;
				tmp = (1.0f - *(float*)(SWeaponAddr + 0x240)) * 100.0f;
				ProcessOperator(statsToSet, tmp);
				*(float*)(SWeaponAddr + 0x240) = 1.0f - tmp * 0.01f;
				*(byte*)(SWeaponAddr + 0x13) |= 0x04;
			}
			if (Player->TertiaryWeaponIndex != 0xffff && (what == 5 || what == 3))
			{
				DWORD TWeaponAddr = (DWORD)ObjectTable[Player->TertiaryWeaponIndex].Offset;
				tmp = (1.0f - *(float*)(TWeaponAddr + 0x240)) * 100.0f;
				ProcessOperator(statsToSet, tmp);
				*(float*)(TWeaponAddr + 0x240) = 1.0f - tmp * 0.01f;
				*(byte*)(TWeaponAddr + 0x13) |= 0x04;
			}
			if (Player->QuaternaryWeaponIndex != 0xffff && (what == 5 || what == 4))
			{
				DWORD QWeaponAddr = (DWORD)ObjectTable[Player->QuaternaryWeaponIndex].Offset;
				tmp = (1.0f - *(float*)(QWeaponAddr + 0x240)) * 100.0f;
				ProcessOperator(statsToSet, tmp);
				*(float*)(QWeaponAddr + 0x240) = 1.0f - tmp * 0.01f;
				*(byte*)(QWeaponAddr + 0x13) |= 0x04;
			}
		}

		env_Print(FormatWArgs(L"Successfully set battery of %ls's weapon(s).", PlayerTable[PlayerIndex].name).c_str());

		return true;
	}
};

class CommandNades final : public PlayerCommand
{
	std::wstring statsToSet;
	size_t what;
	bool setStats;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc < 3)
		{
			setStats = false;
			return true;
		}
		else if (args.argc == 3)
		{
			statsToSet = std::move(args[2]);
			setStats = true;
			what = 0;
			return true;
		}
		else if (args.argc == 4)
		{
			statsToSet = std::move(args[2]);
			setStats = true;
			if (ToNumber(args[3], what) && what <= 2)
				return true;
		}
		return false;
	}

public:
	CommandNades() : PlayerCommand(L"nades", L"[amount] [type(=0, 0-2)]", 4, Naughty::yes, AliveOnly::yes) {}

	bool ExecuteOnPlayer(size_t PlayerIndex, const CommandEnviroment& env) override
	{
		Dynamic_Player* Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;
			
		if(setStats)
		{
			int tmp;
			if(what == 0 || what == 1)
			{
				tmp = Player->FragNades;
				ProcessOperator(statsToSet, tmp);
				Player->FragNades = static_cast<BYTE>(tmp);
			}
			if(what == 0 || what == 2)
			{
				tmp = Player->PlasmaNades;
				ProcessOperator(statsToSet, tmp);
				Player->PlasmaNades = static_cast<BYTE>(tmp);
			}
		}

		env_Print(FormatWArgs(L"%ls has %u frag-grenades and %u plasma.", PlayerTable[PlayerIndex].name, Player->FragNades, Player->PlasmaNades).c_str());
		return true;
	}
};