#include "CDKeyCheck.h"

#include "Addresses.h"
#include "AliasThread.h"
#include "bans.h"
#include "HaloUtilities.h"
#include "LogThread.h"
#include "OnlineAdmins.h"
#include "Patcher.h"
#include "Prefs.h"
#include "TextUtilities.h"

bool valid_cdkey[16] = {};

void CDKeyCheckHandler(const char* reply, DWORD valid)
{
	std::vector <std::string> Items;
	Split(Items, std::string(reply + 1), '\\');

	size_t PlayerMachineNumber = -1;
	for (size_t i = 0; i < 16; i++)
	{
		if (Items[3] == GetPlayerHash(i))
		{
			PlayerMachineNumber = i;
			break;
		}
	}

	if(PlayerMachineNumber == -1)
		return;

	DWORD Value;
	if(!valid)
	{
		if(!strncmp(Items[7].c_str(), "Invalid CD Key", 14))
			Value = 2; // Invalid CD-Key
		else if(!strncmp(Items[7].c_str(), "Invalid authentication", 22))
			Value = 3; // Attempt to steal hash or exploit the server (modified authentication packet)
		else
			Value = 0; // Pending / Unknown error
	}
	else
		Value = 1; // Valid CD-Key

#ifdef HALORANK
	if(GlobalPrefs->HaloRank)
	{
		HREventCDKeyValidation Event;

		Event.Type = HR_EVENT_CDKEY_VALIDATION;
		Event.IP = (**Server->MachineInfo[PlayerMachineNumber].IPAddress)->IP;
		Event.Time = (DWORD)(stats::last_game - time(0));
		Event.Value = Value;

		GlobalSAPPServer.SendHREvent(&Event, sizeof(HREventCDKeyValidation));
	}
#endif

	for (Player_Hash_List* hash_list = PlayerHashList->next; hash_list && hash_list->data; hash_list = hash_list->next)
	{
		if (hash_list->data->id == Server->MachineInfo[PlayerMachineNumber].HashID)
		{
			hash_list->data->validation = 1;
			break;
		}
	}

	if(Value == 1)
	{
		size_t PlayerIndex = NumberToIndex(PlayerMachineNumber);
		valid_cdkey[PlayerIndex] = true;
		if(GlobalPrefs->Alias && GlobalPrefs->OnlyValidAlias)
			AddAlias(GetPlayerHash(PlayerMachineNumber), Server->PlayerData[PlayerIndex].name);
	}
	else if(Value == 3)
	{
		size_t PlayerIndex = NumberToIndex(PlayerMachineNumber);		
		if(PlayerPresent(PlayerIndex))
		{		
			GlobalOnlineAdminList.DeleteAdmin(PlayerIndex);
			NetInfoStruct* info = **Server->MachineInfo[PlayerMachineNumber].IPAddress;
			if (info != NULL)
			{
				WriteLog(L"IPBAN", FormatWArgs(L"Player %ls was IP banned for using fake CD-Key", Server->PlayerData[PlayerIndex].name));
				GlobalIPBanlist.LoadIPBans();
				GlobalIPBanlist.BanIP(info->IP, 0xFFFFFFFF, 32, time(0) + 86400, Server->PlayerData[PlayerIndex].name, L"FAKE CDKEY");
				GlobalIPBanlist.SaveIPBans();
			}
			if(!GlobalPrefs->FullIPBan)
				sv_kick(PlayerIndex);
		}
	}
}

void InstallCDKeyCheckHandler()
{
	Patches.NewPatch(new CodeCaveHook("CDKey Validation 1", CDKeyCheckCode1, 5, 3, NULL, CDKeyCheckHandler, true));
	Patches.NewPatch(new CodeCaveHook("CDKey Validation 2", CDKeyCheckCode2, 5, 2, NULL, CDKeyCheckHandler, true));
}