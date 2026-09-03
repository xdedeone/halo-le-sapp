#include "Stick.h"

#include "Addresses.h"
#include "HaloUtilities.h"
#include "LuaHandler.h"
#include "ObjectManagement.h"
#include "Patcher.h"
#include "Prefs.h"

void __stdcall Stick(DWORD Victim, DWORD Object, DWORD where)
{
	if (GlobalPrefs->Lua && !GlobalPrefs->ScrimMode)
	{
		Dynamic_Player* object = GetObjectAddress(Object);
		if (object)
		{
			size_t ParentPlayerIndex = object->PlayerIndex;

			if (ParentPlayerIndex < 16)
			{
				Dynamic_Player* player = GetObjectAddress(Victim);
				if (player)
				{
					size_t VictimPlayerIndex = player->PlayerIndex;

					std::vector<std::unique_ptr<lua::LuaEventArgBase>> args;
					args.emplace_back(std::make_unique<lua::LuaEventArgInt>(IndexToNumber(VictimPlayerIndex) + 1));
					args.emplace_back(std::make_unique<lua::LuaEventArgDouble>((double)Object));
					args.emplace_back(std::make_unique<lua::LuaEventArgDouble>((double)Victim));
					args.emplace_back(std::make_unique<lua::LuaEventArgInt>(where));
					lua::LuaEventCallback(EVENT_STICK, ParentPlayerIndex, args);
				}
			}
		}
	}
}

DWORD ContinueStick;
__declspec(naked) void StickCodecave()
{
	__asm
	{
		xor edx, edx
		mov dx, word ptr ss : [ebp + 0x3e]

		push eax
		push edx

		push edx // where
		push ebx // what
		push eax // who
		call Stick

		pop edx
		pop eax

		jmp ContinueStick
	}
}

void InstallStickHook()
{
	Patches.NewPatch(new CodeCaveHook("Stick Hook", EventStick, 6, 3, &ContinueStick, StickCodecave));
}