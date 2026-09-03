#include "ComboFix.h"

#include "Addresses.h"
#include "Patcher.h"

DWORD LastKill[16] = {};

void __stdcall ComboFix(DWORD PlayerIndex, DWORD max_time)
{
	PlayerIndex &= 0xFFFF;
	if (LastKill[PlayerIndex] >= max_time)
	{
		PlayerTable[PlayerIndex].combo++;
	}
	else
	{
		PlayerTable[PlayerIndex].combo = 1;
	}
	LastKill[PlayerIndex] = *GameTicks;
}

__declspec(naked) void ComboCodecave()
{
	__asm
	{
		push edx

		push ecx
		push esi
		call ComboFix

		pop edx
		ret
	}
}

void InstallComboFix()
{
	Patches.NewPatch(new CodeCaveHook("Combo Fix", ComboFixCode, 11, 0, NULL, ComboCodecave, true));
}