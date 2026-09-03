#include "HaloThreadHook.h"

#include "Addresses.h"
#include "AimbotScan.h"
#include "CommandExecuter.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "LogThread.h"
#include "Mapcycle.h"
#include "Mapvote.h"
#include "NoLead.h"
#include "Prefs.h"
#include "Patcher.h"
#include "TextUtilities.h"
#include "ThreadCycleFix.h"

size_t server_inactive = 0;

DWORD HaloThreadHook()
{
	EventScan();

	if (GlobalPrefs->AimbotScanScoreLimit)
		AimbotScan();

	if (GlobalPrefs->NoLead)
		UpdateHistory();

	return GameTicksAddress;
}

void IdleCheck()
{
	if(ServerRunning())
	{
		server_inactive = 0;
	}
	else
	{
		server_inactive++;
		
		if(GlobalPrefs->MaxIdle && server_inactive > GlobalPrefs->MaxIdle)
		{
			server_inactive = 0;
			if ((!GlobalPrefs->SappMapcycle || !GlobalMapcycle.Begin()) && (!GlobalPrefs->MapVote || !GlobalMapVotes.Launch()))
				ExecuteHaloCommand(-1, Enviroment::CONSOLE, "sv_mapcycle_begin");

			if (ServerRunning())
			{
				std::string msg = FormatArgs("Game was automatically restarted after the server was inactive for %u seconds.", GlobalPrefs->MaxIdle / 30);
				PrintConsole(14, msg.c_str());
				WriteLog(L"RESTART", std::wstring(msg.begin(), msg.end()));
			}
		}
	}
}

void __declspec(naked) IdleCheckCodecave()
{
	__asm
	{
		push eax
		call IdleCheck
		pop eax

		cmp eax, 0x0c8
		ret
	}
}

void __declspec(naked) GetServerCodecave()
{
	__asm
	{
		mov Server, eax
		mov ecx, ServerPointer
		mov [ecx], eax
		ret
	}
}

void InstallHaloThreadHook()
{
	Patches.NewPatch(new MemoryValue("ServerTicks Address", ServerTicksCode, 1, (DWORD*)&GameTicksAddress));
	GameTicks = (DWORD*)(GameTicksAddress + 12);
	
	Patches.NewPatch(new CodeCaveHook("Halo Thread Hook", ServerTicksCode, 5, 0, NULL, HaloThreadHook, true));
	Patches.NewPatch(new CodeCaveHook("Halo Thread End Hook", ThreadHookCode, 5, 0, NULL, IdleCheckCodecave, true));

	Patches.NewPatch(new CodeCaveHook("Get Server Hook", GetServerCode, 5, 2, NULL, GetServerCodecave, true));
}