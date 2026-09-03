#include "GSQueryPatch.h"

#include "Addresses.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "MiscellaneousFunctions.h"
#include "Patcher.h"
#include "Prefs.h"
#include "TextUtilities.h"

using namespace std;

string NextMap;
string NextMode;

const char* const QueryFinalStr = "final\\Sapp\\queryid\\1.1\\sapp\\%hs %hs\\sapp_flags\\%u\\nextmap\\%hs\\nextmode\\%hs";
char QueryFinal[4096] = {};

map<string, string> ExtraQueryItems;

void FormatQueryString()
{
	string str = FormatArgs(QueryFinalStr, GlobalPrefs->AppVersion, GlobalPrefs->AppHaloType, GlobalPrefs->Flags, NextMap.c_str(), NextMode.c_str());
	for (map<string, string>::iterator it = ExtraQueryItems.begin(); it != ExtraQueryItems.end(); ++it)
	{
		str += '\\';
		str += it->first;
		str += '\\';
		str += it->second;
	}
	if (str.size() > 4095)
		str.resize(4095);
	strcpy(QueryFinal, str.c_str());
}

void PingPatch(DWORD SomeID, DWORD PlayerIndex, char* buff)
{
	char pingbuff[16];
	_itoa(PlayerTable[PlayerIndex].ping, pingbuff, 10);

	DWORD pos = *(DWORD*)(buff+2048); // The current pos is at the end of the buff[2048]
	for(size_t i = 0; pingbuff[i]; i++)
		*(buff + pos++) = pingbuff[i];

	*(DWORD*)(buff+2048) = pos;
}

DWORD PingEndAddr;
void __declspec(naked) PingPatchHandlerCodecave()
{
	__asm
	{
		cmp ecx, 0x18
		je patch
		call eax
		jmp finish

	patch:
		call PingPatch
		xor al, al

	finish:
		add esp, 0x0c
		jmp PingEndAddr
	}
}

void __stdcall PortPatch(char* buff)
{
	char portbuff[16];
	_itoa(Port, portbuff, 10);

	DWORD pos = *(DWORD*)(buff+2048); // The current pos is at the end of the buff[2048]
	for(size_t i = 0; portbuff[i]; i++)
		*(buff + pos++) = portbuff[i];

	*(DWORD*)(buff+2048) = pos;
}

void __declspec(naked) PortPatchHandlerCodecave()
{
	__asm
	{
		cmp eax, 4
		jne not_port

		push ebx
		call PortPatch

		mov eax, 4

	not_port:
		dec eax
		cmp eax, 0x35
		lea edi, [esi+8]
		ret
	}
}

void __stdcall MapNamePatch(char* buff)
{
	DWORD pos = *(DWORD*)(buff + 2048); // The current pos is at the end of the buff[2048]
	for (size_t i = 0; i < CurrentMap.size(); i++)
		*(buff + pos++) = CurrentMap[i];

	*(buff + pos++) = 0;
	*(DWORD*)(buff + 2048) = pos;
}

void __declspec(naked) MapNamePatchHandlerCodecave()
{
	__asm
	{
		push ebx
		call MapNamePatch
		pop edi
		pop esi
		pop ebp
		pop ebx
		add esp, 0x104
		ret
	}
}

void InstallGSQueryFixHandler()
{
	DWORD patch = (DWORD)&QueryFinal;
	Patches.NewPatch(new CodePatch("QueryNextMap Patch", QueryNextMapCode, 4, 1, (byte*)&patch));
	Patches.NewPatch(new CodeCaveHook("QueryPing Fix", PingPatchCode, 5, 3, &PingEndAddr, PingPatchHandlerCodecave));
	Patches.NewPatch(new CodeCaveHook("QueryPort Fix", QueryPortCode, 7, 0, NULL, PortPatchHandlerCodecave, true));

	if (GetHaloVersion() < 10)
		Patches.NewPatch(new CodeCaveHook("QueryMapName Fix", QueryMapNameCode, 6, 0, NULL, MapNamePatchHandlerCodecave));
}