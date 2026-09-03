#include "VehicleEnter-Exit.h"

#include "Addresses.h"
#include "Event.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "Patcher.h"
#include "TextUtilities.h"

void __stdcall VehicleEnterHandler(DWORD i, int seat)
{
	Teleported[i] = true;
	std::vector <std::pair<std::wstring, std::wstring>> EventArgs;
	EventArgs.emplace_back(L"$seat", ToWStr(seat));
	GlobalEventHandler.Execute(i, EVENT_VEHICLE_ENTER, EventArgs);
}

DWORD VehicleEnterFunc;
DWORD ContinueVehicleEnter;
void __declspec(naked) VehicleEnterCodecave()
{
	__asm
	{
		call VehicleEnterFunc

		mov ecx, dword ptr ss:[esp + 4]
		push ecx
		mov ecx, [ebp + 8]
		and ecx, 0xffff
		push ecx
		call VehicleEnterHandler

		jmp ContinueVehicleEnter
	}
}

void InstallVehicleEnterHandler()
{
	Patches.NewPatch(new RelativeMemoryValue("VehicleEnterFunc", EventVehicleEnter, 12, &VehicleEnterFunc));
	Patches.NewPatch(new CodeCaveHook("VehicleEnter", EventVehicleEnter, 5, 11, &ContinueVehicleEnter, VehicleEnterCodecave));
}


void __stdcall VehicleExitHandler(Dynamic_Player* pPlayer)
{
	for (size_t i = 0; i < 16; i++)
	{
		if (PlayerAliveOnly(i))
		{
			Dynamic_Player *Player = ObjectTable[PlayerTable[i].ObjectIndexNum].Offset;
			if (pPlayer == Player)
			{
				Teleported[i] = true;
				GlobalEventHandler.Execute(i, EVENT_VEHICLE_EXIT, std::vector <std::pair<std::wstring, std::wstring>>());
				break;
			}
		}
	}
}

void __declspec(naked) VehicleExitCodecave()
{
	__asm
	{
		lea eax, [eax * 2 + eax]
		mov eax, dword ptr ds:[eax * 4 + edx + 8]

		push eax

		mov ecx, dword ptr ss:[ebp - 0x40]
		push ecx
		call VehicleExitHandler

		pop eax

		ret
	}
}

void __declspec(naked) VehicleAutoExitCodecave()
{
	__asm
	{
		push eax
		push ecx
		push edx

		push ebx
		call VehicleExitHandler

		pop edx
		pop ecx
		pop eax

		and eax, 0xffff
		ret
	}
}

void InstallVehicleExitHandler()
{
	Patches.NewPatch(new CodeCaveHook("VehicleExit", EventVehicleExit, 7, 0, NULL, VehicleExitCodecave, true));
	Patches.NewPatch(new CodeCaveHook("VehicleAutoExit", EventVehicleAutoExit, 5, 4, NULL, VehicleAutoExitCodecave, true));
}