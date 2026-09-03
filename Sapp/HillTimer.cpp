#include "HillTimer.h"

#include "Patcher.h"
#include "Prefs.h"

size_t GetHillTimer()
{
	return GlobalPrefs->HillTimer;
}

DWORD HillTimer;
__declspec(naked) void HillTimerCodecave()
{
	__asm
	{
		push eax
		push ecx

		mov ecx, [HillTimer]
		call GetHillTimer
		mov [ecx], eax

		pop ecx
		pop eax
		ret
	}
}

void InstallHillTimerHook()
{
	Patches.NewPatch(new MemoryAddress("Hill Timer Addr", SetKingTime, 4, &HillTimer));
	Patches.NewPatch(new CodeCaveHook("Hill Timer", SetKingTime, 10, 2, NULL, HillTimerCodecave, true));
	Patches.NewPatch(new CodeCaveHook("Hill Timer Start", SetKingTimeStart, 10, 5, NULL, HillTimerCodecave, true));
	Patches.NewPatch(new CodeCaveHook("Hill Timer Reset", SetKingTimeReset, 10, 6, NULL, HillTimerCodecave, true));
}