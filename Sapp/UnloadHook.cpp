#include "UnloadHook.h"

#include "Patcher.h"

extern HMODULE hMyDLL;

void OnUnload();

DWORD ContinueFreeLibrary;
void __declspec(naked) UnloadHandler()
{
	__asm
	{
		mov eax, dword ptr ds:[esp + 4]
		cmp hMyDLL, eax
		jne not_sapp
		call OnUnload

	not_sapp:
		push ebp
		mov ebp, esp
		jmp ContinueFreeLibrary
	}
}

void InstallUnloadHook()
{
	Patches.NewPatch(new APIHook("Unload Hook", FreeLibrarySign, 21, &ContinueFreeLibrary, UnloadHandler));
}