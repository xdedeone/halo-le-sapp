#include "PlayerQuitKillFix.h"

#include "Patcher.h"

bool PlayerBeingKilled;
DWORD PlayerKillFunc;
DWORD ContinuePlayerKill;
void __declspec(naked) PlayerQuitKillCodecave()
{
	__asm
	{
		mov al, byte ptr ds:[esi + 0x107]
		and al, 0x20
		setnz PlayerBeingKilled
		call PlayerKillFunc
		mov PlayerBeingKilled, 0
		jmp ContinuePlayerKill
	}
}

void __declspec(naked) ApplyFallDamageCodecave()
{
	__asm
	{
		fnstsw ax
		cmp PlayerBeingKilled, 0
		je not_player
		test ah, 0x01
		ret

	not_player:
		test ah, 0x41
		ret
	}
}

void InstallPlayerQuitKillHandler()
{
	Patches.NewPatch(new RelativeMemoryValue("PlayerQuitKillFunc", PlayerQuitKillCode, 7, &PlayerKillFunc));
	Patches.NewPatch(new CodeCaveHook("PlayerQuitKill", PlayerQuitKillCode, 5, 6, &ContinuePlayerKill, PlayerQuitKillCodecave));
	Patches.NewPatch(new CodeCaveHook("ApplyFallDamage", ApplyFallDamageCode, 5, 16, NULL, ApplyFallDamageCodecave, true));
}