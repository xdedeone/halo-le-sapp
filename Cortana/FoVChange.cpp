#include "FoVChange.h"

#include "Addresses.h"
#include "Patch.h"

short FoVHookSig[] = { 0x8B, 0x93, 0xCC, 0x00, 0x00, 0x00, 0x89, 0x93, 0xAC, 0x00, 0x00, 0x00, 0x5B, 0x83, 0xC4, 0x14, 0xC3 };  // 8B93CC0000008993AC0000005B83C414C3

float NewFoV = 1.221730f;
const float FoVRatio = 1.221730f;

DWORD BackToFoV;
void __declspec(naked) FoVHookCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		fld [ebx + 0xcc]
		fdiv FoVRatio
		fmul NewFoV
		fstp[ebx + 0xac]
		ENCRYPT_END	
		jmp BackToFoV;
	}
}

void __declspec(noinline) InstallFoVHook()
{
	ENCRYPT_BEGIN
	Patches.NewPatch(new CodeCaveHook(hhalo, FoVHookSig, sizeof(FoVHookSig), 12, 0, &BackToFoV, FoVHookCC));
	ENCRYPT_END
}