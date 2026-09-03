#include "TabCompletion.h"

#include <vector>

#include "CommandFactory.h"
#include "Patcher.h"
#include "TextUtilities.h"

char** ToComplete;
short* MatchedCommandCount;
WORD* Flags;

std::vector<std::string> last_commands;

short __stdcall TabCompletion(const char** base)
{
	short count = *MatchedCommandCount;
#ifndef HPC
	if (*Flags & 0x20) // Tab pressed, not actual executing of the command
#else
	if (*Flags == 1)
#endif
	{
		last_commands.clear();

		std::wstring to_compare = FormatWArgs(L"%hs", *ToComplete);
		for (auto& it : GlobalCommandFactory.GetStorage())
		{
			if (count >= 256)
				break;
			if (it.first.get().compare(0, to_compare.length(), to_compare) == 0)
			{
				last_commands.emplace_back(WToStr(it.first.get()));
			}
		}
		for (auto& it : last_commands)
		{
			base[count++] = it.c_str();
		}
	}
	*MatchedCommandCount = count;
	return count;
}

void __declspec(naked) TabCompletionCodecave()
{
	__asm
	{
		push ebp
		call TabCompletion
		movsx edx, ax
		ret
	}
}

void InstallTabCompletionHook()
{
	Patches.NewPatch(new MemoryAddress("Tab Completion Addr", TabCompletionAddr, 1, (DWORD*)&ToComplete));
	Patches.NewPatch(new MemoryAddress("Tab Completion Flags", TabCompletionAddr, 20, (DWORD*)&Flags));
	Patches.NewPatch(new MemoryAddress("Tab Completion Count", TabCompletionCode, 3, (DWORD*)&MatchedCommandCount));
	Patches.NewPatch(new CodeCaveHook("Tab Completion", TabCompletionCode, 7, 0, NULL, TabCompletionCodecave, true));
}