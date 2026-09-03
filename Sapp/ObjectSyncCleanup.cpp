#include "ObjectSyncCleanup.h"

#include "Patcher.h"

DWORD ObjectSyncStruct;
DWORD GetObjectSyncIndex;
DWORD RemoveSync;
DWORD RemoveObject;
void __declspec(naked) CleanUpObjectSyncCodecave()
{
	__asm
	{
		push eax
		push edx
		push edi
		push esi

		mov ecx, edx
		mov esi, ObjectSyncStruct

		call GetObjectSyncIndex
		cmp eax, -1
		pop esi
		je not_found
			mov edi, dword ptr ds:[esp + 4]
			and byte ptr ds : [esi + 2], 0xF7
			call RemoveSync
	not_found:
		pop edi
		pop edx
		pop eax
		jmp RemoveObject
	}
}

void InstallObjectSyncCleanupHandler()
{
	Patches.NewPatch(new FunctionAddress("ObjectRemoveFunc", ObjectRemoveFuncCode, 0, &RemoveObject));
	Patches.NewPatch(new FunctionAddress("RemoveObjectSyncFunc", RemoveObjectSyncFuncCode, 0, &RemoveSync));
	Patches.NewPatch(new MemoryValue("ObjectSyncStruct", ObjectSyncStructCode, 2, &ObjectSyncStruct));
	Patches.NewPatch(new RelativeMemoryValue("GetObjectSyncIndexFunc", ObjectSyncStructCode, 12, &GetObjectSyncIndex));

	ObjectSyncStruct += 0x0C;
}

void ActivateObjectSyncCleanupHandler()
{
	Patches.NewPatch(new CodeCaveHook("CleanUpSyncObject", ObjectRemoveCode, 5, 0, NULL, CleanUpObjectSyncCodecave, true));
}

void DeactivateObjectSyncCleanupHandler()
{
	Patches.Remove("CleanUpSyncObject");
}