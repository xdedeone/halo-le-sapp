#include "NoLead.h"

#include <utility>

#include "Addresses.h"
#include "HaloUtilities.h"
#include "Patcher.h"

bool Lead[16];
size_t CustomLead[16];
size_t PingIndex[16] = {};

size_t HistoryIndex = 0;
NoLeadObjectItemRoot PlayerHistory[16][32] = {};

void CreateHistory(Dynamic_Player* Object, NoLeadObjectItem &Buffer)
{
	if (Buffer.Weapon == nullptr)
	{
		Buffer.Weapon = new NoLeadObjectItem();
	}
	if (Buffer.VehicleWeapon == nullptr)
	{
		Buffer.VehicleWeapon = new NoLeadObjectItem();
	}

	size_t size = Object->Sizeof1stBlock + Object->Sizeof2ndBlock + Object->Sizeof3rdBlock + Object->Sizeof4thBlock;
	if (Buffer.Size < size)
	{
		delete[] (uint8_t*)Buffer.Object;
		Buffer.Object = (Dynamic_Player*)new uint8_t[size];
		Buffer.Size = size;
	}

	Buffer.Valid = true;

	memcpy(Buffer.Object, Object, size);

	if (Object->WeaponIndexNum != 0xFFFF)
	{
		Dynamic_Player* Weapon = ObjectTable[Object->WeaponIndexNum].Offset;
		CreateHistory(Weapon, *Buffer.Weapon);
	}
	else
	{
		Buffer.Weapon->Valid = false;
	}

	if (Object->VehicleWeaponIndexNum != 0xFFFF)
	{
		Dynamic_Player* VehicleWeapon = ObjectTable[Object->VehicleWeaponIndexNum].Offset;
		CreateHistory(VehicleWeapon, *Buffer.VehicleWeapon);
	}
	else
	{
		Buffer.VehicleWeapon->Valid = false;
	}
}

void UpdateHistory()
{
	if (!HistoryIndex)
		HistoryIndex = 31;
	else
		HistoryIndex--;

	for (size_t i = 0; i < 16; i++)
	{
		if (PlayerAliveOnly(i))
		{			
			Dynamic_Player* Player = ObjectTable[PlayerTable[i].ObjectIndexNum].Offset;
			if (Player->VehicleIndexNum == 0xFFFF)
			{
				PlayerHistory[i][HistoryIndex].RootIndex = PlayerTable[i].ObjectIndexNum;
				CreateHistory(Player, PlayerHistory[i][HistoryIndex]);
			}
			else
			{
				Dynamic_Player* Vehicle = ObjectTable[Player->VehicleIndexNum].Offset;
				if (Vehicle->WeaponIndex == PlayerTable[i].ObjectIndex)
				{
					PlayerHistory[i][HistoryIndex].RootIndex = Player->VehicleIndexNum;
					CreateHistory(Vehicle, PlayerHistory[i][HistoryIndex]);
				}
				else
				{
					PlayerHistory[i][HistoryIndex].Valid = false;
				}
			}
		}
		else
		{
			PlayerHistory[i][HistoryIndex].Valid = false;
		}
	}
}

void ResetHistory(size_t PlayerIndex)
{
	for (size_t i = 0; i < 32; i++)
	{
		PlayerHistory[PlayerIndex][i].Valid = false;
	}
}

void ClearNoLeadCache()
{
	for (size_t i = 0; i < 16; i++)
	{
		for (size_t j = 0; j < 32; j++)
		{
			delete PlayerHistory[i][j].Weapon;
			delete PlayerHistory[i][j].VehicleWeapon;
			delete[] (uint8_t*)PlayerHistory[i][j].Object;
		}
	}

	memset(PlayerHistory, 0, sizeof(PlayerHistory));
}

void RevertObjectHistory(Dynamic_Player* &Object, NoLeadObjectItem &Buffer)
{
	std::swap(Object, Buffer.Object);

	Buffer.Restored = true;
		
	if (Buffer.Weapon->Valid)
		RevertObjectHistory(ObjectTable[Object->WeaponIndexNum].Offset, *Buffer.Weapon);
	if (Buffer.VehicleWeapon->Valid)
		RevertObjectHistory(ObjectTable[Object->VehicleWeaponIndexNum].Offset, *Buffer.VehicleWeapon);
}

void RestoreObjectHistory(Dynamic_Player* &Object, NoLeadObjectItem &Buffer)
{
	if (Buffer.VehicleWeapon->Restored)
		RestoreObjectHistory(ObjectTable[Object->VehicleWeaponIndexNum].Offset, *Buffer.VehicleWeapon);
	if (Buffer.Weapon->Restored)
		RestoreObjectHistory(ObjectTable[Object->WeaponIndexNum].Offset, *Buffer.Weapon);

	std::swap(Object, Buffer.Object);

	Buffer.Restored = false;
}

size_t CurrentHistoryIndex;
void RevertHistory()
{
	for (size_t i = 0; i < 16; i++)
	{
		if (PlayerHistory[i][CurrentHistoryIndex].Valid)
		{
			RevertObjectHistory(ObjectTable[PlayerHistory[i][CurrentHistoryIndex].RootIndex].Offset, PlayerHistory[i][CurrentHistoryIndex]);
		}
	}
}

void RestoreHistory()
{
	for (size_t i = 0; i < 16; i++)
	{
		if (PlayerHistory[i][CurrentHistoryIndex].Restored)
		{
			RestoreObjectHistory(ObjectTable[PlayerHistory[i][CurrentHistoryIndex].RootIndex].Offset, PlayerHistory[i][CurrentHistoryIndex]);
		}
	}
}

size_t CurrentPlayerIndex = -1;
void __stdcall NoLeadBeginFromCC(size_t ObjectIndex)
{
	for (size_t i = 0; i < 16; i++)
	{
		if (PlayerAliveOnly(i))
		{
			if (PlayerTable[i].ObjectIndex == ObjectIndex)
			{
				if (!Lead[i])
				{
					CurrentPlayerIndex = i;
				}
				break;
			}
			else
			{
				Dynamic_Player* Player = ObjectTable[PlayerTable[i].ObjectIndexNum].Offset;
				if (Player->VehicleIndex == ObjectIndex)
				{
					if (!Lead[i])
					{
						CurrentPlayerIndex = i;
					}
					break;
				}	
			}
		}
	}

	if (CurrentPlayerIndex != -1)
	{
		CurrentHistoryIndex = (HistoryIndex + PingIndex[CurrentPlayerIndex]) % 32;
		RevertHistory();
	}
}

void __stdcall NoLeadEndFromCC()
{
	if (CurrentPlayerIndex != -1)
	{
		CurrentPlayerIndex = -1;
		RestoreHistory();
	}
}

DWORD ContinueIntersect;
__declspec(naked) void IntersectFunc()
{
	__asm
	{
		sub esp, 0x438
		jmp ContinueIntersect
	}
}

void __stdcall ValidateObject(s_intersection_output* out)
{
	if (out->hit == 3 && (ObjectTable[out->ObjectIndexNum].Offset == nullptr || ObjectTable[out->ObjectIndexNum].ObjectID != out->ObjectID))
	{
		out->hit = 2;
	}
}

size_t Restores = 0;
DWORD RetAddr;
__declspec(naked) void IntersectBeginCodecave()
{
	__asm
	{
		mov eax, dword ptr ss:[esp + 0x10]
		cmp eax, -1
		je skip_nolead

		cmp dword ptr ss:[esp + 4], 0x1000E9
		jne skip_nolead

		mov ecx, Restores
		inc ecx
		cmp ecx, 1
		mov Restores, ecx
		jne skip_nolead

		push eax
		call NoLeadBeginFromCC

		pop RetAddr

		call IntersectFunc

		mov Restores, 0

		push eax

		call NoLeadEndFromCC

		cmp byte ptr ss:[esp], 0
		je no_validation

		push dword ptr ss:[esp + 0x14]
		call ValidateObject

	no_validation:
		pop eax
		jmp RetAddr

	skip_nolead:
		sub esp, 0x438
		jmp ContinueIntersect
	}
}

DWORD BulletMagnetFunc;
DWORD ContinueBulletPhysics;
void __declspec(naked) BulletMagnetismCodecave()
{
	__asm
	{
		mov Restores, 1

		push ecx
		call NoLeadBeginFromCC

		call BulletMagnetFunc

		mov Restores, 0

		push eax

		call NoLeadEndFromCC

		pop eax

		jmp ContinueBulletPhysics
	}
}

void InstallNoLeadHandler()
{
	Patches.NewPatch(new CodeCaveHook("Collision Check", func_intersect, 6, 0, &ContinueIntersect, IntersectBeginCodecave));
	Patches.NewPatch(new RelativeMemoryValue("Bullet Physics Function", BulletMagnetFuncCall, 5, &BulletMagnetFunc));
	Patches.NewPatch(new CodeCaveHook("Bullet Magnetism", BulletMagnetFuncCall, 5, 4, &ContinueBulletPhysics, BulletMagnetismCodecave));
}

void UninstallNoLeadHandler()
{
	Patches.Remove("Collision Check");
	Patches.Remove("Bullet Physics Function");
	Patches.Remove("Bullet Magnetism");
}