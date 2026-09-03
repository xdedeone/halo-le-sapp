#include "RasterisePatches.h"

#include "Addresses.h"
#include "Patch.h"

short DevmodeSig[] = { 0x8A, 0x0D, -1, -1, -1, -1, 0x80, 0xE2, 0x07, 0x84, 0xC9, 0xB0, 0x01, 0x66, 0x0F, 0xB6, 0xD2 }; // 8A0D????????80E20784C9B001660FB6D2
short WireframeSig[] = { 0x8A, 0x15, -1, -1, -1, -1, 0xA1, -1, -1, -1, -1, 0x8B, 0x08, 0x83, 0xC4, 0x08, 0xF6, 0xDA, 0x1B, 0xD2, 0x83, 0xC2, 0x03, 0x52, 0x6A, 0x08, 0x50 }; // 8A15????????A1????????8B0883C408F6DA1BD283C203526A0850
short FogAtmosphereSig[] = { 0xDA, 0xE9, 0xDF, 0xE0, 0xF6, 0xC4, 0x44, 0x7B, 0x09, 0xA0, -1, -1, -1, -1, 0x84, 0xC0, 0x75 }; // DAE9DFE0F6C4447B09A0????????84C075
short FogPlaneSig[] = { 0xF6, 0x02, 0x04, 0x75, -1, 0x8A, 0x0D, -1, -1, -1, -1, 0x84, 0xC9, 0x74 }; // F6020475??8A0D????????84C974
short WallSig[] = { 0x57, 0x8B, 0xF8, 0xA0, -1, -1, -1, -1, 0x33, 0xDB, 0x3A, 0xC3 }; // 578BF8A0????????33DB3AC3

byte DevmodePatch[6] = { 0xB1, 0x00, 0x90, 0x90, 0x90, 0x90 }; // mov cl, 0
byte WireframePatch[6] = { 0xB2, 0x00, 0x90, 0x90, 0x90, 0x90 }; // mov dl, 0
byte FogAtmospherePatch[5] = { 0xB0, 0x01, 0x90, 0x90, 0x90 }; // mov al, 1
byte FogPlanePatch[6] = { 0xB1, 0x01, 0x90, 0x90, 0x90, 0x90 }; // mov cl, 1

bool* Wireframe = NULL;
bool* FogA = NULL;
bool* FogP = NULL;
bool* Walls = NULL;

void __declspec(noinline) InstallRasterisePatchesCode()
{
	ENCRYPT_BEGIN
	Patches.NewPatch(new MemoryAddress(hhalo, WireframeSig, sizeof(WireframeSig), 2, (DWORD*)&Wireframe));
	Patches.NewPatch(new MemoryAddress(hhalo, FogAtmosphereSig, sizeof(FogAtmosphereSig), 10, (DWORD*)&FogA));
	Patches.NewPatch(new MemoryAddress(hhalo, FogPlaneSig, sizeof(FogPlaneSig), 7, (DWORD*)&FogP));
	Patches.NewPatch(new MemoryAddress(hhalo, WallSig, sizeof(WallSig), 4, (DWORD*)&Walls));

	Patches.NewPatch(new CodePatch(hhalo, DevmodeSig, sizeof(DevmodeSig), 6, 0, DevmodePatch));
	Patches.NewPatch(new CodePatch(hhalo, WireframeSig, sizeof(WireframeSig), 6, 0, WireframePatch));
	Patches.NewPatch(new CodePatch(hhalo, FogAtmosphereSig, sizeof(FogAtmosphereSig), 5, 9, FogAtmospherePatch));
	Patches.NewPatch(new CodePatch(hhalo, FogPlaneSig, sizeof(FogPlaneSig), 6, 5, FogPlanePatch));
	Patches.NewPatch(new CodePatch(hhalo, WallSig, sizeof(WallSig), 5, 3, FogAtmospherePatch));
	ENCRYPT_END
}