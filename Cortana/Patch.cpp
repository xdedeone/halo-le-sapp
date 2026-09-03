#include "patch.h"

#include "ConsoleHook.h"
#include "DirectXPatch.h"
#include "ForwardCameraStructPatch.h"
#include "ForwardCrossairPatch.h"
#include "ForwardTablesPatch.h"
#include "ForwardTagsTablePatch.h"
#include "ForwardPlayersTablePatch.h"
#include "FoVChange.h"
#include "HaloThreadHook.h"
#include "JoinPatch.h"
#include "KeystonePatch.h"
#include "MapCache.h"
#include "MapChecksum.h"
#include "NetworkEvents.h"
#include "ScoreboardPatch.h"
#include "ScreenShots.h"
#include "SendChatMessage.h"
#include "RasterisePatches.h"
#include "WindowNamePatch.h"

PatchContainer Patches;

__declspec(noinline) Patch::Patch(HMODULE hModule, const short* sign, size_t length, size_t where) : sig(sign), slen(length)
{
	ENCRYPT_BEGIN
	if (!where)
		PatchAddress = FindCodeFromHandle(hModule, sign, length);
	else
		PatchAddress = where;

	if (PatchAddress == NULL)
	{
		found = false;
	}
	else
		found = true;
	ENCRYPT_END
}

const short* Patch::GetSig(size_t &length)
{
	length = slen;
	return sig;
}

__declspec(noinline) FunctionAddress::FunctionAddress(HMODULE hModule, const short* sign, size_t length, int Difference, DWORD *Address) : Patch(hModule, sign, length)
{
	ENCRYPT_BEGIN
	if (found)
	{
		PatchAddress += Difference;
		*Address = PatchAddress;
	}
	ENCRYPT_END
}

__declspec(noinline) MemoryAddress::MemoryAddress(HMODULE hModule, const short* sign, size_t length, int Difference, DWORD *Value) : Patch(hModule, sign, length)
{
	ENCRYPT_BEGIN
	if (found)
	{
		PatchAddress += Difference;
		*Value = (DWORD)*(DWORD*)(PatchAddress);
	}
	ENCRYPT_END
}

__declspec(noinline) MemoryValue::MemoryValue(HMODULE hModule, const short* sign, size_t length, int Difference, DWORD *Value) : Patch(hModule, sign, length)
{
	ENCRYPT_BEGIN
	if (found)
	{
		PatchAddress += Difference;
		*Value = *(DWORD*)*(DWORD*)(PatchAddress);
	}
	ENCRYPT_END
}

__declspec(noinline) MemoryValueFromOPCode::MemoryValueFromOPCode(HMODULE hModule, const short* sign, size_t length, int Difference, DWORD *Value) : Patch(hModule, sign, length)
{
	ENCRYPT_BEGIN
	if (found)
	{
		PatchAddress += Difference;
		*Value = *(DWORD*)PatchAddress + PatchAddress + 4;
	}
	ENCRYPT_END
}

__declspec(noinline) CodePatch::CodePatch(HMODULE hModule, const short* sign, size_t length, size_t size, int Difference, byte* PatchCode, DWORD Pointer) : Patch(hModule, sign, length), size(size)
{
	ENCRYPT_BEGIN
	if (found)
	{
		PatchAddress += Difference;

		if (Pointer > 0)
		{
			if (Pointer == 1)
				PatchAddress = *(DWORD*)PatchAddress;
			else
				PatchAddress = *(DWORD*)PatchAddress + PatchAddress + 4;
		}

		memcpy(OrginalCode, (LPVOID)PatchAddress, size);

		DWORD OldProtect;
		VirtualProtect((LPVOID)PatchAddress, size, PAGE_EXECUTE_READWRITE, &OldProtect);

		memcpy((LPVOID)PatchAddress, PatchCode, size); // patch the code

		VirtualProtect((LPVOID)PatchAddress, size, OldProtect, &OldProtect);
	}
	ENCRYPT_END
}
__declspec(noinline) CodePatch::~CodePatch()
{
	ENCRYPT_BEGIN
	if (found)
	{
		DWORD OldProtect;
		VirtualProtect((LPVOID)PatchAddress, size, PAGE_EXECUTE_READWRITE, &OldProtect);

		memcpy((LPVOID)PatchAddress, OrginalCode, size); // restore the original code

		VirtualProtect((LPVOID)PatchAddress, size, OldProtect, &OldProtect);
	}
	ENCRYPT_END
}

__declspec(noinline) CodeCaveHook::CodeCaveHook(HMODULE hModule, const short* sign, size_t length, size_t size, int Difference, DWORD *BackAddress, LPVOID Function, bool Call, size_t where) : Patch(hModule, sign, length, where), size(size)
{
	ENCRYPT_BEGIN
	if (found)
	{
		PatchAddress += Difference;

		char *cc_code = (char*)PatchAddress;
		memcpy(OrginalCode, cc_code, size);

		if (BackAddress != NULL)
			*BackAddress = PatchAddress + size;

		DWORD OldProtect;
		// insert codecave jump
		VirtualProtect((LPVOID)PatchAddress, size, PAGE_EXECUTE_READWRITE, &OldProtect);

		if (Call)
			cc_code[0] = '\xE8'; // opcode of call
		else
			cc_code[0] = '\xE9'; // opcode of jmp

		DWORD CallOffset = (DWORD)Function - (PatchAddress + 5);
		*(DWORD*)(&cc_code[1]) = CallOffset;

		memset(&cc_code[5], 0x90, size - 5); // fill the rest with nops

		VirtualProtect((LPVOID)PatchAddress, size, OldProtect, &OldProtect);
	}
	ENCRYPT_END
}
__declspec(noinline) CodeCaveHook::~CodeCaveHook()
{
	ENCRYPT_BEGIN
	if (found)
	{
		DWORD OldProtect;
		VirtualProtect((LPVOID)PatchAddress, size, PAGE_EXECUTE_READWRITE, &OldProtect);

		memcpy((LPVOID)PatchAddress, OrginalCode, size); // restore the original code

		VirtualProtect((LPVOID)PatchAddress, size, OldProtect, &OldProtect);
	}
	ENCRYPT_END
}


void PatchContainer::NewPatch(Patch* newPatch)
{
	Patches.emplace(newPatch);
}

void PatchContainer::RemoveAll()
{
	ENCRYPT_BEGIN
	for (std::set <Patch*>::iterator it = Patches.begin(); it != Patches.end(); ++it)
	{
		delete *it;
	}
	Patches.clear();
	ENCRYPT_END
}

void __declspec(noinline) InstallPatches()
{
	ENCRYPT_BEGIN
	InstallChatMessageSender();
	InstallConsoleHook();
	InstallForwardTablesPatch();
	InstallForwardCameraStructPatch();
	InstallForwardPlayersTablePatch();
	InstallForwardTagsTablePatch();
	InstallForwardCrossairPatch();
	InstallMapCache();
	InstallMapChecksumCode();
	InstallKeystonePatchCode();
	InstallDXPatchCode();
	//InstallPresentHookCode();
	InstallRasterisePatchesCode();
	InstallWindowNamePatchCode();
	InstallJoinPatch();
	InstallScoreboardHookCode();
	InstallHaloThreadHookCode();
	InstallFoVHook();
	InstallNetworkeventsHook();
	ENCRYPT_END
}