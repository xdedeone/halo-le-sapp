#include "BlockCheatSpawns.h"

#include "Patcher.h"
#include "Prefs.h"

bool CheckScrimMode()
{
	return GlobalPrefs->ScrimMode;
}

DWORD BackToCheatSpawn;
void _declspec(naked) CheatSpawnCC()
{
	__asm
	{
		call CheckScrimMode
		test al, al
		jnz block_spawn
		sub esp, 0xb8
		jmp BackToCheatSpawn

	block_spawn:
		ret
	}
}

void InstallBlockCheatSpawnHooks()
{
	Patches.NewPatch(new CodeCaveHook("CheatSpawn", func_cheat_spawn, 6, 0, &BackToCheatSpawn, CheatSpawnCC));
}