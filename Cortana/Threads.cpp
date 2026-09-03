#include "Threads.h"

#include "Addresses.h"
#include "ForwardTagsTablePatch.h"
#include "IsDebuggerPresentTrick.h"
#include "MiscFunctions.h"
#include "NetworkClient.h"

#define SCP		10
#define SC		12
#define CASC	13
#define GSC		14
#define RS		16
#define PS		17
#define ES		42
#define SV		47

AntiAntiIsDP Cucc;

int DeathClock = 30;

void DeathClockThread(LPVOID)
{
#ifndef DEBUG
	ENCRYPT_BEGIN
	NtSIT(GetCurrentThread(), 0x11, 0, 0);
	PerfomanceCounter PC;
	while (DeathClock > 0)
	{
		PC.StartQPC();
		Sleep(1024);
		DeathClock -= PC.GetQPC() / 1000;
	}	
	myClient->Disconnect();
	ENCRYPT_END
#endif
}

void ScanThread(LPVOID)
{
	ENCRYPT_BEGIN
	NtSIT(GetCurrentThread(), 0x11, 0, 0);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
	ENCRYPT_END
	while (true)
	{
		ListProcess();
		Sleep(5000);
		ListModules();
		Sleep(900000);
	}
}

extern bool* Wireframe;
extern bool* FogA;
extern bool* FogP;
extern bool* Walls;
void ThreadPong(LPVOID)
{
	ENCRYPT_BEGIN
	DWORD count = 0;
	NtSIT(GetCurrentThread(), 0x11, 0, 0);
	myClient->SendSmallEvent(event_load_cortana);
	ENCRYPT_END
	while (true)
	{
		ENCRYPT_BEGIN
		unsigned char hash[32];
		HashHalo(hash);
		if (memcmp(hHash, hash, 32))
			myClient->SendSmallEvent(event_wrong_checksum);

		//if (TagHashReady)
		//{
		//	Tag_Entry* pTagsTable = (Tag_Entry*)(TagsTable ^ MagicUserKey);
		//	ULONGLONG tags_hash = 0;
		//	for (size_t i = 0; i < count; i++)
		//		tags_hash += (pTagsTable[i].TagID ^ pTagsTable[i].TagStruct) * (i + 1);

		//	if (tags_hash != TagsTableHash)
		//		myClient->SendSmallEvent(event_wrong_tags_hash);
		//}

		if (*Wireframe || !*FogA || !*FogP || !*Walls)
			myClient->SendSmallEvent(event_rasterise_change);

		for (size_t i = 4; i < 119; i++)
		{
			if (i == SCP || i == SC || i == CASC || i == GSC || i == RS || i == PS || i == ES || i == SV)
				continue;

			if (*(unsigned char*)(VTable[i]) == 0xE9)
				myClient->SendSmallEvent(event_illegal_dx_hook, (WORD)i);
		}

		if (*(byte*)NtSIT == 0xE9 || *(byte*)NtQIP == 0xE9)
			myClient->SendSmallEvent(event_anti_debugger_detect);

		HANDLE hDebugObject = NULL;
		if (!NtQIP(cProc, 0x1e, &hDebugObject, 4, NULL) && hDebugObject)
			myClient->SendSmallEvent(event_debug_object);

		HANDLE NoDebugInherit = NULL;
		if (!NtQIP(cProc, 0x1f, &NoDebugInherit, 4, NULL) && !NoDebugInherit)
			myClient->SendSmallEvent(event_no_debug_inherit);

		myClient->SendSmallEvent(event_ping);

		ENCRYPT_END

		if (myClient->StartWait(10000))
			myClient->SendSmallEvent(event_sem_wait_timeout);
	}
}

void ThreadPing(LPVOID)
{
	ENCRYPT_BEGIN
	NtSIT(GetCurrentThread(), 0x11, 0, 0);
	_beginthread(ThreadPong, 0, 0);
	ENCRYPT_END
	while (true)
	{
		ENCRYPT_BEGIN
		if (!Cucc.Check())
			myClient->SendSmallEvent(event_debugger_attached);

		myClient->EndWait();
		ENCRYPT_END
		Sleep(5000);
		//__try
		//{
		//	KillProcess();
		//	__asm int 3
		//	myClient->SendSmallEvent(event_int3_stepover);
		//}
		//__except (EXCEPTION_EXECUTE_HANDLER)
		//{
		//	ReleaseSemaphore(sem, 1, 0);
		//	Sleep(2000);
		//}
	}
}

void EventThread(LPVOID)
{
#ifndef DEBUG
	ENCRYPT_BEGIN
	NtSIT(GetCurrentThread(), 0x11, 0, 0);
	_beginthread(ThreadPing, 0, 0);
	PerfomanceCounter PC;
	ENCRYPT_END
	Sleep(10000);
	while (true)
	{
		ENCRYPT_BEGIN
		PC.StartQPC();
		Sleep(100);		
		if (PC.GetQPC() > 2000)
		{
			myClient->SendSmallEvent(event_timing_attack);
		}	
		if (KillMe)
		{
			KillMe = false;
			Sleep(2000);
			myClient->Disconnect();
		}
		ENCRYPT_END
	}
#endif
}
