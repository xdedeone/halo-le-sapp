#include "DamageApplicationHook.h"

#include "Addresses.h"
#include "HaloUtilities.h"
#include "LuaHandler.h"
#include "Patcher.h"
#include "Prefs.h"

bool __stdcall DamageApplication(DWORD receiver, DWORD causer, DWORD MetaID, float* damage, char* hit_string, bool backtap)
{
	if (GlobalPrefs->Lua && !GlobalPrefs->ScrimMode)
	{
		size_t PlayerIndex = -1;
		for (size_t i = 0; i < 16; i++)
		{
			if (PlayerTable[i].ObjectIndex == receiver)
			{
				PlayerIndex = i;
				break;
			}
		}

		if (PlayerIndex == -1)
			return true;

		std::vector<std::unique_ptr<lua::LuaEventArgBase>> args;
		args.emplace_back(std::make_unique<lua::LuaEventArgInt>(IndexToNumber(causer) + 1));
		args.emplace_back(std::make_unique<lua::LuaEventArgDouble>((double)MetaID));
		args.emplace_back(std::make_unique<lua::LuaEventArgDouble>((double)*damage));
		args.emplace_back(std::make_unique<lua::LuaEventArgString>(hit_string));
		args.emplace_back(std::make_unique<lua::LuaEventArgInt>(backtap));
		double dret = *damage;
		if (lua::LuaEventCallback(EVENT_DAMAGE_APPLICATION, PlayerIndex, args, &dret))
		{
			*damage = (float)dret;
			return true;
		}
		else
		{
			return false;
		}
	}

	return true;
}

// mostly reversed by Oxide
DWORD ContinueApplyDamage;
__declspec(naked) void DamageApplicationCodecave()
{
	__asm
	{
		mov ecx, [esp + 0x38]
		cmp byte ptr ds:[ecx], 2 // melee damage
		je possible_backtap
		mov al, 0 // can't be a backtap, not melee damage

	possible_backtap:
		movzx eax, al
		push eax

		cmp eax, 0
		je not_backtap

		mov ecx, [GlobalPrefs]
		cmp [ecx + prefs::DisableBacktap], 0
		je not_backtap
		mov bl, 0

	not_backtap:
		push esi // hit location string
		lea eax, [esp + 0x20]
		push eax // damage
		mov eax, [ebp]
		mov ecx, [ebp + 0x08]
		mov edx, [esp + 0x48]	
		push eax // MetaID of the damage object
		and ecx, 0xffff
		push ecx // causer player
		push edx // receiver obj
		call DamageApplication
		test al, al
		jz block_damage

		cmp byte ptr ss:[esp + 0x40], 1
		jmp ContinueApplyDamage
		
	block_damage:
		pop edi
		pop esi
		pop ebp
		pop ebx
		add esp, 0x94
		ret
	}
}

void InstallDamageApplicationHook()
{
	Patches.NewPatch(new CodeCaveHook("Damage Application Hook", DamageApplicationCode, 5, 0, &ContinueApplyDamage, DamageApplicationCodecave));
}