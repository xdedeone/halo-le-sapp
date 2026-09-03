#include "ObjectRespawn.h"

#include "Patcher.h"

const byte ObjectRespawnPatch[2] = { 0x90, 0x90 }; // nops

void InstallObjectRespawnPatch()
{
	Patches.NewPatch(new CodePatch("Object Respawn Patch", ObjectSpawnTimerCode, 2, 11, ObjectRespawnPatch));
}

void UninstallObjectRespawnPPatch()
{
	Patches.Remove("Object Respawn Patch");
}