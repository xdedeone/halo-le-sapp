#include "ObjectInteractions.h"

#include "Addresses.h"
#include "ObjectManagement.h"
#include "Patcher.h"
#include "Prefs.h"

using namespace std;

map <string, ObjectUsage> DisabledObjects;
map <DWORD, ObjectUsage> ObjectCache;

void RemoveIfEmpty(map <string, ObjectUsage>::iterator it)
{
	if (it->second.Red == true || it->second.Blue == true)
		return;
	
	for (size_t i = 0; i < 16; i++)
	{
		if (it->second.Players[i] == true)
			return;
	}
	DisabledObjects.erase(it);
}

bool __stdcall ObjectInteraction(DWORD PlayerID, DWORD ObjectID, bool vehicle)
{
	size_t PlayerIndex = PlayerID & 0xffff;

	if ((!vehicle && GlobalPrefs->DisableObjectsPlayer[PlayerIndex]) || (vehicle && GlobalPrefs->DisableVehiclesPlayer[PlayerIndex]))
	{
		return true;
	}
	if ((!vehicle && GlobalPrefs->DisableObjectsRed) || (vehicle && GlobalPrefs->DisableVehiclesRed))
	{
		if (*TeamGame)
		{
			if (PlayerTable[PlayerIndex].Team == 0)
				return true;
		}
		else
			return true;
	}
	if ((!vehicle && GlobalPrefs->DisableObjectsBlue) || (vehicle && GlobalPrefs->DisableVehiclesBlue))
	{
		if (*TeamGame)
		{
			if (PlayerTable[PlayerIndex].Team == 1)
				return true;
		}
		else
			return true;
	}

	if (!DisabledObjects.empty())
	{
		Dynamic_Player* obj = GetObjectAddress(ObjectID);

		if (obj)
		{
			DWORD MetaNumber = obj->MetaNumber;
			map <DWORD, ObjectUsage>::iterator it = ObjectCache.find(MetaNumber);
			if (it != ObjectCache.end())
			{
				if (it->second.Players[PlayerIndex])
					return true;

				if (*TeamGame)
				{
					if (PlayerTable[PlayerIndex].Team == 0)
						return it->second.Red;
					else
						return it->second.Blue;
				}
				else
				{
					return it->second.Red;
				}
			}

			Tag_Entry* header = LookupTag(MetaNumber);

			if (header)
			{
				if (ObjectCache.size() > 4096)
					ObjectCache.clear();

				map <string, ObjectUsage>::iterator jt = DisabledObjects.find(header->TagName);
				if (jt != DisabledObjects.end())
				{
					auto& ptr = jt->second;
					ObjectCache[MetaNumber] = ptr;

					if (ptr.Players[PlayerIndex])
						return true;

					if (*TeamGame)
					{
						if (PlayerTable[PlayerIndex].Team == 0)
							return ptr.Red;
						else
							return ptr.Blue;
					}
					else
					{
						return ptr.Red;
					}
				}
				else
				{
					ObjectUsage TB;
					ObjectCache[MetaNumber] = TB;
					return false;
				}
			}
		}
	}

	return false;
}

DWORD ObjInteractionEnd;
DWORD ObjectInteractionAddrV;
void __declspec(naked) ObjectInteractionCodecaveV()
{
	__asm
	{
		push 1
		push ecx
		push edi
		call ObjectInteraction
		cmp al, 1
		je block_interaction

		call ObjectInteractionAddrV

	block_interaction:
		jmp ObjInteractionEnd
	}
}

DWORD ObjectInteractionAddr;
void __declspec(naked) ObjectInteractionCodecave()
{
	__asm
	{
		push 0
		push ecx
		push edi
		call ObjectInteraction
		cmp al, 1
		je block_interaction

		call ObjectInteractionAddr

	block_interaction:
		jmp ObjInteractionEnd
	}
}

void InstallObjectInteractionHandler()
{
	Patches.NewPatch(new FunctionAddress("Object Interaction Vehicles", ObjectInteractionVehi, 0, &ObjectInteractionAddrV));
	Patches.NewPatch(new FunctionAddress("Object Interaction Others", ObjectInteractionOther, 0, &ObjectInteractionAddr));
	Patches.NewPatch(new FunctionAddress("Object Interaction End", ObjectInteractionCode, 25, &ObjInteractionEnd));
	Patches.NewPatch(new CodeCaveHook("Object Interaction Vehicles CC", ObjectInteractionCode, 5, 2, NULL, ObjectInteractionCodecaveV));
	Patches.NewPatch(new CodeCaveHook("Object Interaction CC", ObjectInteractionCode, 5, 11, NULL, ObjectInteractionCodecave));
}