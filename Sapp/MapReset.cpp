#include "MapReset.h"

#include "Addresses.h"
#include "Event.h"
#include "NetworkEvents.h"
#include "Patcher.h"
#include "PlayerJoin-Leave.h"

DWORD MapResetString;

void __stdcall MapReset()
{
	PlayersLeftStats.clear();
	GlobalEventHandler.Execute(-1, EVENT_MAP_RESET, std::vector <std::pair<std::wstring, std::wstring>>());
	NetworkEvent<DataSync<int>>(NE_DataSync_TimeLeft, GetTimeLeft());
}

DWORD BackToMapReset;
void __declspec(naked) MapResetCodecave()
{
	__asm
	{
		call MapReset
		push MapResetString
		jmp BackToMapReset
	}
}

void InstallMapResetHandler()
{
	Patches.NewPatch(new MemoryAddress("Map Reset String", sv_map_reset_Code, 2, &MapResetString));
	Patches.NewPatch(new CodeCaveHook("Map Reset Hook", sv_map_reset_Code, 5, 1, &BackToMapReset, MapResetCodecave));
}