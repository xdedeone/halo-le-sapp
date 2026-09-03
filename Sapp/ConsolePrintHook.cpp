#include "ConsolePrintHook.h"

#include <string>
#include <vector>

#include "CommandExecuter.h"
#include "HaloUtilities.h"
#include "LuaHandler.h"
#include "Patcher.h"
#include "Prefs.h"
#include "RemoteConsole.h"

bool __stdcall PrintConsoleHook(const char* str)
{
	if (CommandExecuting == -2)
	{
		if (ScriptEcho && GlobalPrefs->Lua && !GlobalPrefs->ScrimMode)
		{
			std::vector<std::unique_ptr<lua::LuaEventArgBase>> args;
			args.emplace_back(std::make_unique<lua::LuaEventArgString>(str));
			lua::LuaEventCallback(EVENT_ECHO, -1, args);
		}
		return false;
	}
	else if (CommandExecuting > -1)
	{
		MsgSend(CommandExecuting, str);
		return false;
	}
	else if (CommandExecuting == -4)
	{
		GlobalRemoteConsole.SendRemoteOutput(str);
		return false;
	}

	return true;
}

DWORD BackToConsolePrint;
void __declspec(naked) ConsolePrintHookCodecave()
{
	__asm
	{
		push eax

		push eax
		call PrintConsoleHook
		
		test al, al
		pop eax
		jz block_print

		push ebp
		mov ebp, esp
		and esp, 0xfffffff8
		jmp BackToConsolePrint

	block_print:
		ret
	}
}

void InstallConsolePrintHook()
{
	Patches.NewPatch(new CodeCaveHook("Console Print Hook", PrintCode, 6, 0, &BackToConsolePrint, ConsolePrintHookCodecave));
}