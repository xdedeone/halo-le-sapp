#include "Score.h"

#include "Addresses.h"
#include "Event.h"
#include "Patcher.h"
#include "Prefs.h"

void __stdcall EventScore(DWORD PlayerIndex)
{
	PlayerIndex &= 0xFFFF;
	GlobalEventHandler.Execute(PlayerIndex, EVENT_SCORE, std::vector<std::pair<std::wstring, std::wstring>>());
}

DWORD CTFScoreFunc;
DWORD BackToScoreFunc;
void __declspec(naked) EventCTFScoreCodecave()
{
	__asm
	{
		call CTFScoreFunc

		push esi
		call EventScore

		inc stats::Captures

		jmp BackToScoreFunc
	}
}

void __declspec(naked) EventRallyScoreCodecave()
{
	__asm
	{
		mov ecx, RaceScores
		mov dword ptr ds : [ecx + Race_Scores::FlagToTouch], eax

		push ebp
		call EventScore

		add esp, 8
		pop esi
		pop ebx
		pop edi
		pop ebp
		add esp, 8
		retn
	}
}

void __declspec(naked) EventRaceScoreCodecave()
{
	__asm
	{
		push ebp
		call EventScore

		add esp, 4
		pop esi
		pop ebx
		pop edi
		pop ebp
		add esp, 8
		retn
	}
}

void InstallEventScoreHandler()
{
	Patches.NewPatch(new RelativeMemoryValue("Event CTF Score Func", EventCTFScoreCode, 8, &CTFScoreFunc));
	Patches.NewPatch(new CodeCaveHook("Event CTF Score", EventCTFScoreCode, 5, 7, &BackToScoreFunc, EventCTFScoreCodecave));
	Patches.NewPatch(new CodeCaveHook("Event Rally Score", EventRallyScoreCode, 5, 0, NULL, EventRallyScoreCodecave));
	Patches.NewPatch(new CodeCaveHook("Event Race Score", EventRaceScoreCode, 5, 0, NULL, EventRaceScoreCodecave));
}