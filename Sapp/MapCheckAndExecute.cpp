#include "MapCheckAndExecute.h"

#include "Addresses.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "GSQueryPatch.h"
#include "Patcher.h"
#include "Prefs.h"

bool RunMapSuccess;
bool MapExecuted = false;
bool MapVoteCanceled = false;

DWORD HaloRunMapCheckAddress;
bool __declspec(naked) CheckMap(const char* map, const char* mode)
{
	__asm
	{
		push esi
		push edi
		mov esi, [esp + 0x0c] // map
		mov edi, [esp + 0x10] // mode
		call HaloRunMapCheckAddress // retuns a bool in al
		pop edi
		pop esi
		ret
	}
}

DWORD HaloRunMapAddress;
bool RunMap(const char* Map, const char* Mode)
{
	__asm
	{
		mov eax, Map
		mov ebx, Mode
		call HaloRunMapAddress
	}

	if (RunMapSuccess)
	{
		NextMap = Map;
		NextMode = Mode;
		FormatQueryString();
		MapExecuted = true;
		return true;
	}
	else
	{
		MapExecuted = false;
		return false;
	}
}

void sv_map_Handler(const char* nextMap, const char* nextMode)
{
	if (RunMapSuccess)
	{
		MapExecuted = true;
		NextMap = nextMap;
		NextMode = nextMode;
		FormatQueryString();
		if (GlobalPrefs->MapVote && !MapVoteCanceled)
		{
			MapVoteCanceled = true;
			sv_say(L"Map Voting has been cancelled!");
		}
	}
}

void __declspec(naked) sv_map_HandlerCodecave()
{
	__asm
	{
		push eax // mode
		push ebx // map
		call HaloRunMapAddress
		call sv_map_Handler
		add esp, 8
		ret
	}
}

void __declspec(naked) sv_map_CheckHandlerCodecave()
{
	__asm
	{
		call HaloRunMapCheckAddress
		mov RunMapSuccess, al
		ret
	}
}

void Install_sv_map_Handler()
{
	Patches.NewPatch(new FunctionAddress("RunMap function", RunMapCode, 0, &HaloRunMapAddress));
	Patches.NewPatch(new RelativeMemoryValue("sv_map Check Func", RunMapCode, 29, (DWORD*)&HaloRunMapCheckAddress));
	Patches.NewPatch(new CodeCaveHook("sv_map Check Hook", RunMapCode, 5, 28, NULL, sv_map_CheckHandlerCodecave, true));
	Patches.NewPatch(new CodeCaveHook("sv_map Hook", sv_map_Code, 5, 6, NULL, sv_map_HandlerCodecave, true));
}