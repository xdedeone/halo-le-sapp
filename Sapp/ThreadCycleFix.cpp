#include "ThreadCycleFix.h"

#include "Patcher.h"
#include "Prefs.h"
#include "SuperHeader.h"

byte UpdatePingPatch[2] = {0xE8, 0x03};
byte DisableLogFormatting[4] = { 0xC3, 0x90, 0x90, 0x90 };

HANDLE hSemaphore;

void __stdcall TimerTick(PVOID, BOOLEAN)
{
	ReleaseSemaphore(hSemaphore, 1, NULL);
}

HANDLE hHaloTimerQueue;
HANDLE hHaloTimer;
void CustomSleep()
{
	HaloThreadLock.Unlock();
	WaitForSingleObject(hSemaphore, INFINITE);
	HaloThreadLock.Lock();
}

DWORD BackToSleep;
void __declspec(naked) ThreadCycleSleepCodecave()
{
	__asm
	{
		call CustomSleep
		jmp BackToSleep
	}
}

void InstallTCycleFix()
{
	Patches.NewPatch(new FunctionAddress("ThreadCycleSleep End", ThreadCycleFixCode, 48, &BackToSleep));
	Patches.NewPatch(new CodeCaveHook("ThreadCycleSleep Patch", ThreadCycleFixCode, 6, 0, NULL, ThreadCycleSleepCodecave));
	Patches.NewPatch(new CodePatch("PingUpdateInterval Patch", PingUpdateIntervalCode, 2, 1, UpdatePingPatch));
	Patches.NewPatch(new CodePatch("LogFormatting Patch", DisableLogFormattingCode, 4, 0, DisableLogFormatting));
}

void CreateHaloTimer()
{
	hSemaphore = CreateSemaphoreW(NULL, 0, 0x7FFFFFFF, NULL);
	hHaloTimerQueue = CreateTimerQueue();
	CreateTimerQueueTimer(&hHaloTimer, hHaloTimerQueue, TimerTick, NULL, 0, GlobalPrefs->CustomSleep, WT_EXECUTEDEFAULT);
}

void SetHaloTimer()
{
	ChangeTimerQueueTimer(hHaloTimerQueue, hHaloTimer, 0, GlobalPrefs->CustomSleep);
}

void DeleteHaloTimer()
{
	DeleteTimerQueueEx(hHaloTimerQueue, NULL);
	CloseHandle(hSemaphore);
}