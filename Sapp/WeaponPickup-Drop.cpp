#include "WeaponPickup-Drop.h"

#include "Addresses.h"
#include "Event.h"
#include "Patcher.h"
#include "TextUtilities.h"

void __stdcall WeaponPickupHandler(DWORD PlayerID, int WeaponIndex, int type)
{
	size_t i = 0;
	for (; i < 16; i++)
	{
		if (PlayerTable[i].ObjectIndex == PlayerID)
			break;
	}

	if (i > 15)
		return;

	std::vector <std::pair<std::wstring, std::wstring>> EventArgs;
	EventArgs.emplace_back(L"$index", ToWStr(WeaponIndex + 1));
	EventArgs.emplace_back(L"$type", ToWStr(type));
	GlobalEventHandler.Execute(i, EVENT_WEAPON_PICKUP, EventArgs);
}

void __declspec(naked) WeaponPickupCodecave()
{
	__asm
	{
		push 1
		push eax
		push esi
		call WeaponPickupHandler

		movsx eax, word ptr ss:[esp + 0x20]

		ret
	}
}

void InstallWeaponPickupHandler()
{
	Patches.NewPatch(new CodeCaveHook("WeaponPickup", EventWeaponPickup, 5, 11, NULL, WeaponPickupCodecave, true));
}


void __declspec(naked) NadePickupCodecave()
{
	__asm
	{
		or edi, 0x04000000

		push edx

		push 2
		movsx eax, word ptr ds:[esi + 0x30a]
		push eax
		push ebx
		call WeaponPickupHandler

		pop edx

		ret
	}
}

void InstallNadePickupHandler()
{
	Patches.NewPatch(new CodeCaveHook("NadePickup", EventNadePickup, 6, 14, NULL, NadePickupCodecave, true));
}


void __stdcall WeaponDropHandler(DWORD PlayerID, int WeaponIndex)
{
	size_t i = 0;
	for (; i < 16; i++)
	{
		if (PlayerTable[i].ObjectIndex == PlayerID)
			break;
	}

	if (i > 15)
		return;

	std::vector <std::pair<std::wstring, std::wstring>> EventArgs;
	EventArgs.emplace_back(L"$index", ToWStr(WeaponIndex + 1));
	GlobalEventHandler.Execute(i, EVENT_WEAPON_DROP, EventArgs);
}

void __declspec(naked) WeaponDropCodecave()
{
	__asm
	{
		mov dword ptr ds:[ecx * 4 + edi + 0x2f8], eax

		push ecx
		push ebp
		call WeaponDropHandler

		or eax, -1
		ret
	}
}

void InstallWeaponDropHandler()
{
	Patches.NewPatch(new CodeCaveHook("WeaponDrop", EventWeaponDrop, 7, 3, NULL, WeaponDropCodecave, true));
}