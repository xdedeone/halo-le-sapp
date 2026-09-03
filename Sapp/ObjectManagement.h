#pragma once

#include <string>
#include <vector>

#include "SuperHeader.h"

extern std::vector <SpawnedVehicle> SpawnedVehicles;
extern DWORD LastWeaponSpawned;
extern bool SappSpawning;

Tag_Entry* LookupTag(const char* tagType, const char* tag);
Tag_Entry* LookupTag(DWORD dwMapId);
Dynamic_Player* GetObjectAddress(DWORD objectId);

DWORD SpawnObject(const char* tagType, const char* tagName, const vect3f& location, float rotation, size_t PlayerIndex = -1, DWORD MapID = 0);
DWORD SpawnProjectile(DWORD MapID, DWORD ParentID, const vect3f& location, const vect3f& movement);
void DestroyObject(DWORD m_objId);
bool AssignPlayerWeapon(size_t PlayerIndex, DWORD m_weaponId);
void DropWeapon(size_t PlayerIndex);
bool EnterVehicle(size_t PlayerIndex, DWORD m_vehicleId, DWORD seat);
void EjectVehicle(size_t PlayerIndex);
void KillPlayer(size_t PlayerIndex);
void ForceSpawn(size_t PlayerIndex, DWORD respawn_time);
void SyncAmmo(DWORD weaponId, DWORD index = 0);
void MakeInvisible(size_t PlayerIndex, size_t duration);
bool InteractWithPowerup(size_t PlayerIndex, DWORD ObjectID);
bool Intersect(DWORD parent, const vect3f& pos, const vect3f& trans, vect3f& out, DWORD& unit_hit);