#include "mtv.h"

#include "Patcher.h"

byte MTVPatch[1] = {0x2A};

void InstallMTV()
{
	Patches.NewPatch(new CodePatch("MTV", MultiTeamVehiclesCode, 1, 8, MTVPatch));
}

void UninstallMTV()
{
	Patches.Remove("MTV");
}