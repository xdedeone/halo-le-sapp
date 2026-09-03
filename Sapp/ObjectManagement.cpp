#include "ObjectManagement.h"

#include <iomanip>

#include "Addresses.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "MiscellaneousFunctions.h"
#include "StringCompare.h"
#include "TextUtilities.h"

using namespace std;

vector <SpawnedVehicle> SpawnedVehicles;
DWORD LastWeaponSpawned = 0xFFFFFFFF;
bool SappSpawning = false;

Tag_Entry* LookupTag(const char* tagType, const char* tag)
{
	DWORD type = endian_swap(*(DWORD*)tagType);

	for (size_t i = 0; i < TagTableHeader->TagCount; i++)
	{
		if (TagTable[i].Class0 == type && !strcmp(TagTable[i].TagName, tag))
		{
			return &TagTable[i];
		}
	}

	return nullptr;
}

Tag_Entry* LookupTag(DWORD dwMapId)
{
	DWORD Index = dwMapId & 0xFFFF;

	if (Index < TagTableHeader->TagCount && TagTable[Index].TagID == dwMapId)
	{
		return &TagTable[Index];
	}
	
	return nullptr;
}

Dynamic_Player* GetObjectAddress(DWORD objectId)
{
	WORD hi = (objectId >> 16) & 0xFFFF;
	WORD lo = (objectId & 0xFFFF);

	if (hi == 0xFFFF || lo == 0xFFFF)
		return nullptr;

	Object_Table_Array* obj = &ObjectTable[lo];

	return obj->ObjectID == hi ? obj->Offset : nullptr;
}

// Code partly from Phasor, credits to Oxide
DWORD SpawnObject(const char* tagType, const char* tagName, const vect3f& location, float rotation, size_t PlayerIndex, DWORD MapID)
{
	Tag_Entry* tag = MapID == 0 ? LookupTag(tagType, tagName) : LookupTag(MapID);

	if (tag == nullptr)
		return 0xFFFFFFFF;

	// Build the request data for <halo>.CreateObject
	DWORD mapId = tag->TagID;
	ObjectCreationQuery query;
	__asm
	{
		push 0xFFFFFFFF // ParentID
		push mapId
		lea eax, [query]
		call dword ptr ds:[FUNC_CREATEOBJECTQUERY]
		add esp, 8
	}

	// Set the spawn coordinates and rotation
	query.pos = location;
	query.RotationVectorX = cos(rotation);
	query.RotationVectorY = sin(rotation);

	SappSpawning = true;

	// Create the object
	DWORD newobj_id = 0xFFFFFFFF;
	__asm
	{
		push 0
		lea eax, [query]
		push eax
		call dword ptr ds:[FUNC_CREATEOBJECT]
		add esp, 8
		mov newobj_id, eax
	}

	SappSpawning = false;

	Dynamic_Player* obj = GetObjectAddress(newobj_id);

	if (obj == nullptr)
		return 0xFFFFFFFF;

	if (!strcmp(tagType, "weap"))
		LastWeaponSpawned = newobj_id;
	else if (!strcmp(tagType, "vehi"))
		SpawnedVehicles.emplace_back(newobj_id, location, rotation, PlayerIndex);
	
	return newobj_id;
}

DWORD SpawnProjectile(DWORD MapID, DWORD ParentID, const vect3f& location, const vect3f& movement)
{
	ObjectCreationQuery query;
	__asm
	{
		push ParentID
		push MapID
		lea eax, [query]
		call dword ptr ds : [FUNC_CREATEOBJECTQUERY]
		add esp, 8
	}

	vect3f normalized = movement.Normalize();

	query.unk1[1] &= 2;
	query.pos = location;
	query.RotationVectorX = normalized.x;
	query.RotationVectorY = normalized.y;
	query.RotationVectorZ = normalized.z;

	SappSpawning = true;

	// Create the object
	DWORD newobj_id = 0xFFFFFFFF;
	__asm
	{
		push 3
		lea eax, [query]
		push eax
		call dword ptr ds : [FUNC_CREATEOBJECT]
		add esp, 8
		mov newobj_id, eax
	}

	SappSpawning = false;

	Dynamic_Player* obj = GetObjectAddress(newobj_id);

	if (obj == nullptr)
		return 0xFFFFFFFF;

	obj->Unknown0 = 0;
	*((BYTE*)obj + 0x278) = 1;

	obj->PlayerVectors.Mov = movement;

	__asm
	{
		mov esi, newobj_id
		call FUNC_INITPROJ
	}

	DWORD retval = BuildPacketProjectileWrapper(newobj_id);
	if (retval)
	{			
		AddPacketToGlobalQueue(retval, 1, 1, 0, 0, 3);
	}

	return newobj_id;
}

// Code partly from Phasor, credits to Oxide
void DestroyObject(DWORD m_objId)
{
	if (GetObjectAddress(m_objId))
	{
		__asm
		{
			mov eax, m_objId
			call dword ptr ds:[FUNC_DESTROYOBJECT]
		}
	}
}

// Forces a player to equip (and change to) the specified weapon.
// Code partly from Phasor, credits to Oxide
bool AssignPlayerWeapon(size_t PlayerIndex, DWORD m_weaponId)
{
	bool bSuccess = false;

	// player must have an object
	if (PlayerAlive(PlayerIndex))
	{
		Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

		if(Player->VehicleIndexNum == 0xffff) //and not be in a vehicle
		{
			// make sure they passed a weapon
			Dynamic_Player* m_object = GetObjectAddress(m_weaponId);

			if (m_object)
			{
				Tag_Entry* tag = LookupTag(m_object->MetaNumber);

				if (tag->Class0 == 0x77656170) // weap
				{
					DWORD mask = (PlayerTable[PlayerIndex].PlayerID << 0x10) | PlayerIndex;
					DWORD playerObj = PlayerTable[PlayerIndex].ObjectIndex;

					__asm
					{
						push 1
						mov eax, m_weaponId
						mov ecx, playerObj
						call dword ptr ds:[FUNC_PLAYERASSIGNWEAPON] // assign the weapon
						add esp, 4
						mov bSuccess, al
						cmp al, 1
						jnz ASSIGNMENT_FAILED
						mov ecx, mask
						mov edi, m_weaponId
						push -1
						push -1
						push 7
						push 1
						call DWORD PTR ds:[FUNC_NOTIFY_WEAPONPICKUP] // notify clients of the assignment
						add esp, 0x10
					ASSIGNMENT_FAILED:
					}
				}
			}
		}
	}

	return bSuccess;
}

void DropWeapon(size_t PlayerIndex)
{
	DWORD playerObj = PlayerTable[PlayerIndex].ObjectIndex;

	__asm
	{
		push 1
		mov ecx, playerObj
		call dword ptr ds:[FUNC_DROPWEAPON_1]
		add esp, 4

		push 1
		push playerObj
		call dword ptr ds:[FUNC_DROPWEAPON_2]
		add esp, 8
	}
}

// Code partly from Phasor, credits to Oxide
bool EnterVehicle(size_t PlayerIndex, DWORD m_vehicleId, DWORD seat)
{
	if (PlayerAlive(PlayerIndex))
	{
		Dynamic_Player* m_object = GetObjectAddress(m_vehicleId);

		if (m_object)
		{
			Tag_Entry* tag = LookupTag(m_object->MetaNumber);

			if (tag->Class0 == 0x76656869) // vehi
			{
				// set interaction info
				PlayerTable[PlayerIndex].SwapID = m_vehicleId;
				PlayerTable[PlayerIndex].SwapType = 8; // vehicle interaction
				PlayerTable[PlayerIndex].SwapSeat = (WORD)seat;

				DWORD mask = (PlayerTable[PlayerIndex].PlayerID << 0x10) | PlayerIndex;

				// enter the vehicle (if seat is free)
				__asm
				{
					push mask
					call dword ptr ds:[FUNC_ENTERVEHICLE]
					add esp, 4
				}
				return true;
			}
		}
	}
	return false;
}


// Code from Phasor, credits to Oxide
void EjectVehicle(size_t PlayerIndex)
{
	DWORD playerObj = PlayerTable[PlayerIndex].ObjectIndex;

	__asm
	{
		mov eax, playerObj
		call dword ptr ds:[FUNC_EJECTVEHICLE]
	}
}

// Code partly from Phasor, credits to Oxide
void KillPlayer(size_t PlayerIndex)
{
	DWORD playerMask = (PlayerTable[PlayerIndex].PlayerID << 0x10) | PlayerIndex;
	DWORD playerObj = PlayerTable[PlayerIndex].ObjectIndex;
	
	__asm
	{
		mov eax, dword ptr ds:[ADDR_GAMESTATE]
		cmp dword ptr ds:[eax], 0
		jne GAME_ENDING
		push 1
		push -1
		push -1
		push -1
		mov eax, playerObj
		call DWORD PTR ds:[FUNC_ONPLAYERDEATH]
		add esp, 0x10
	GAME_ENDING:
		mov eax, playerObj
		call DWORD PTR ds:[FUNC_ACTIONDEATH_1]
		mov eax, playerMask
		call DWORD PTR ds:[FUNC_ACTIONDEATH_2]
		push playerMask
		call DWORD PTR ds:[FUNC_ACTIONDEATH_3]
		add esp, 4
	}
}

void ForceSpawn(size_t PlayerIndex, DWORD respawn_time)
{
	DWORD playerMask = (PlayerTable[PlayerIndex].PlayerID << 0x10) | PlayerIndex;
	PlayerTable[PlayerIndex].RespawnTimer = respawn_time;

	__asm
	{
		mov eax, playerMask
		call DWORD PTR ds:[FUNC_ACTIONDEATH_2]
	}
}

// Code from Phasor, credits to Oxide
void SyncAmmo(DWORD weaponId, DWORD index)
{
	__asm
	{
		mov ebx, index
		mov ecx, weaponId
		call dword ptr ds:[FUNC_UPDATEAMMO]
	}
}

// Code from Phasor, credits to Oxide
void MakeInvisible(size_t PlayerIndex, size_t duration)
{
	DWORD playerMask = (PlayerTable[PlayerIndex].PlayerID << 0x10) | PlayerIndex;
	DWORD count;

	if (duration != 0)
		count = duration;
	else
		count = 0x8000; // this duration is treated as infinite

	__asm
	{
		mov eax, PlayerIndex
		mov ebx, playerMask
		push count
		push 0
		call dword ptr ds:[FUNC_DOINVIS]
		add esp, 8
	}
}

bool InteractWithPowerup(size_t PlayerIndex, DWORD ObjectID)
{
	if (PlayerAlive(PlayerIndex))
	{
		Dynamic_Player* m_object = GetObjectAddress(ObjectID);

		if (m_object)
		{
			Tag_Entry* tag = LookupTag(m_object->MetaNumber);

			if (tag->Class0 == 0x65716970) // eqip
			{
				DWORD playerMask = (PlayerTable[PlayerIndex].PlayerID << 0x10) | PlayerIndex;

				__asm
				{
					push ObjectID
					push playerMask
					call dword ptr ds:[FUNC_INTERACTWITHPOWERUP]
					add esp, 8
				}
				return true;
			}
		}
	}
	return false;
}

// Thx to Skyline
bool __declspec(naked) Intersect(DWORD parent, const vect3f& pos, const vect3f& trans, vect3f& out, DWORD& unit_hit)
{
	__asm
	{
		push ebp
		mov ebp, esp
		sub esp, 0x64
	}

	s_intersection_output buff;

	__asm
	{
		lea eax, buff
		push eax // arg5: buff
		push parent //arg4: PID
		push trans // arg3: trans
		push pos // arg2: pos
		push 0x1000E9 //arg1: const , cant remember what this is, possibly options of some sort?
		call VarBspIntersect
		add esp, 0x14

		push eax // it returns a bool if there was an intersection or not
	}

	out = buff.hit_pos; // at offset 0x18 of the out struct is the intersection
	if (buff.hit == 3)
		unit_hit = buff.hit_obj;

	__asm
	{
		pop eax

		add esp, 0x64
		pop ebp
		ret
	}
}