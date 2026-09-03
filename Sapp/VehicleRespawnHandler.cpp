#include "VehicleRespawnHandler.h"

#include "ObjectManagement.h"
#include "Patcher.h"

byte VehicleRespawnPatch[] = { 0xB0, 0x01, 0x90, 0x90, 0x90, 0x90, 0x90 };

vect3f spawncoord;
float rotation;
bool __stdcall SappVehicleRespawnCheck(DWORD ObjectID)
{
	for (size_t i = 0; i < SpawnedVehicles.size(); i++)
	{
		if (SpawnedVehicles[i].ObjectID == ObjectID)
		{
			spawncoord = SpawnedVehicles[i].Coords;
			rotation = SpawnedVehicles[i].Rotation;
			return true;
		}
	}
	return false;
}

const vect3f nullcooord;
void __declspec(naked) RespawnHandlerCodecave()
{
	__asm
	{
		push eax
		push ebp
		call SappVehicleRespawnCheck
		test al, al
		pop eax
		jnz sapp_vehicle_found	

		test eax, eax
		jnz have_spawnpoint

		lea eax, nullcooord
		fld dword ptr ds:[eax]
		ret

	have_spawnpoint:
		add eax, 8
		fld dword ptr ds:[eax + 0x0c]
		ret

	sapp_vehicle_found:
		lea eax, spawncoord
		fld dword ptr ds:[rotation]
		ret
	}
}

void InstallVehicleRespawnHandler()
{
	Patches.NewPatch(new CodeCaveHook("Vehicle Respawn Handler", VehicleRespawnHandlerCode, 6, 0, NULL, RespawnHandlerCodecave, true));
	Patches.NewPatch(new CodePatch("Vehicle Respawn Patch", VehicleRespawnPatchCode, 7, 7, VehicleRespawnPatch));
}