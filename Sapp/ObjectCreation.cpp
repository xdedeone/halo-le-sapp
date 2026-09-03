#include "ObjectCreation.h"

#include "Addresses.h"
#include "LuaHandler.h"
#include "ObjectManagement.h"
#include "Patcher.h"
#include "Prefs.h"
#include "Spawn.h"

bool __stdcall OnCreateObject(ObjectCreationQuery* info)
{
	if (GlobalPrefs->Lua && !GlobalPrefs->ScrimMode)
	{
		size_t PlayerIndex;
		if (NextPlayerToSpawn == -1)
		{
			WORD index = info->ObjectIndexNum;
			if (index != 0xFFFF)
				PlayerIndex = index;
			else
				PlayerIndex = -1;
		}
		else
		{
			PlayerIndex = NextPlayerToSpawn;
		}

		WORD Max = ObjectTableHeader->MaxObjects;
		WORD Next = ObjectTableHeader->NextSlot;
		DWORD ObjectID = 0xFFFFFFFF;
		while (Next < Max)
		{
			if (ObjectTable[Next].ObjectID == 0)
			{
				ObjectID = ObjectTableHeader->NextObjectID << 0x10 | Next;
				break;
			}
			else
			{
				Next++;
			}
		}

		std::vector<std::unique_ptr<lua::LuaEventArgBase>> args;
		args.emplace_back(std::make_unique<lua::LuaEventArgDouble>((double)info->map_id));
		args.emplace_back(std::make_unique<lua::LuaEventArgDouble>((double)info->parent));
		args.emplace_back(std::make_unique<lua::LuaEventArgDouble>((double)ObjectID));
		args.emplace_back(std::make_unique<lua::LuaEventArgInt>(SappSpawning));
		double dret = (double)info->map_id;
		if (lua::LuaEventCallback(EVENT_OBJECT_SPAWN, PlayerIndex, args, &dret))
		{
			size_t ret = (size_t)dret;
			if (ret != info->map_id)
			{
				Tag_Entry* original_tag = LookupTag(info->map_id);
				Tag_Entry* new_tag = LookupTag(ret);
				if (original_tag && new_tag && original_tag->Class0 == new_tag->Class0)
				{
					info->map_id = ret;
				}
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return true;
	}
}

DWORD BackToCreateObject;
void __declspec(naked) CreateObjectCodecave()
{
	__asm
	{
		mov ecx, dword ptr ss:[esp + 4]
		push ecx
		call OnCreateObject
		test al, al
		jz block_create

		sub esp, 0x21c
		jmp BackToCreateObject

	block_create:
		mov eax, -1
		ret
	}
}

void InstallObjectCreationHandler()
{
	Patches.NewPatch(new CodeCaveHook("ObjectSpawnHook", func_createobject, 6, 0, &BackToCreateObject, CreateObjectCodecave));
}