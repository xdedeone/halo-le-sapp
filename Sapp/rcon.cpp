#include "rcon.h"

#include "Addresses.h"
#include "Admins.h"
#include "bans.h"
#include "CommandExecuter.h"
#include "HaloUtilities.h"
#include "LogThread.h"
#include "LuaHandler.h"
#include "MiscellaneousFunctions.h"
#include "OnlineAdmins.h"
#include "Patcher.h"
#include "Prefs.h"
#include "TextUtilities.h"

int RconFails[16] = {};

int __stdcall RconHandler(int MachineID, char* password, char* command)
{
	int PlayerIndex = NumberToIndex(MachineID);

	if(!PlayerPresent(PlayerIndex))
		return -1;

	if (GlobalPrefs->Lua && !GlobalPrefs->ScrimMode)
	{
		std::vector<std::unique_ptr<lua::LuaEventArgBase>> args;
		args.emplace_back(std::make_unique<lua::LuaEventArgString>(command));
		args.emplace_back(std::make_unique<lua::LuaEventArgInt>(1));
		args.emplace_back(std::make_unique<lua::LuaEventArgString>(password));
		if (!lua::LuaEventCallback(EVENT_COMMAND, PlayerIndex, args))
			return 0;
	}

	int level = -1;

	int admin_level = GlobalOnlineAdminList.GetAdminLevel(PlayerIndex);
	if(GlobalPrefs->SappRcon)
	{
		if (RconPassword[0] != 0 && !strcmp(password, RconPassword))
			level = admin_level;
		else
		{
			unsigned char pwmd5[16];
			MD5_Hash(password, strlen(password), pwmd5);

			level = GlobalAdminList.IsAdmin(PlayerTable[PlayerIndex].name, ConvertToMD5Hash(pwmd5));
			if (level > -1)
			{
				Login(PlayerIndex, level);
			}
		}
	}
	else
	{
		if (RconPassword[0] != 0 && !strcmp(password, RconPassword))
		{
			level = 4;
			if (level > admin_level)
			{
				Login(PlayerIndex, level);
			}
		}
	}

	if(level < 0)
	{
		PrintRcon(PlayerIndex, "Access Denied");

		WriteLog(L"COMMAND_RCON", FormatWArgs(L"Player %ls failed to access rcon, command: %hs", PlayerTable[PlayerIndex].name, command));
		
		if(++RconFails[PlayerIndex] == 3)
			PrintRcon(PlayerIndex, "Warning! One more failed rcon access and you will be IP banned!");
		else if(RconFails[PlayerIndex] == 4)
		{
			WriteLog(L"IPBAN", FormatWArgs(L"Player %ls was IP banned for more than 3 failed rcon attempts", PlayerTable[PlayerIndex].name));

			NetInfoStruct* info = **Server->MachineInfo[MachineID].IPAddress;
			if (info != NULL)
			{
				GlobalIPBanlist.BanIP(info->IP, 0xFFFFFFFF, 32, time(0) + 3600, PlayerTable[PlayerIndex].name, L"FAILED RCON ATTEMPTS");
			}		
			if(!GlobalPrefs->FullIPBan)
				sv_kick(PlayerIndex);	
		}

		return 0;
	}
	else
	{	
		WriteLog(L"COMMAND_RCON", FormatWArgs(L"Admin %ls executed the following rcon command: %hs", PlayerTable[PlayerIndex].name, command));

		CommandEnviroment env(Enviroment::RCON, PlayerIndex, level);
		int ret = ExecuteSingleCommand(FormatWArgs(L"%hs", command), env);
		if(ret == -1) // No rights
			ret = 0;

		return ret; // -1: Bad client (trying to exploit?), 0: "rcon command failed", 1: "rcon command finished", 2: keep executing to check if it's a Halo command
	}
}

DWORD ContinueCommandAddr;
DWORD FinishCommandAddr;
#ifndef HPC
void __declspec(naked) RconHandlerCodecave()
{
	__asm
	{
		lea edx, [esp+0x14]
		lea ecx, [esp+0x1d]
		
		push ecx
		push edx
		push ebp
		call RconHandler
		cmp eax, -1
		je bad_client
		cmp eax, 2
		jb finish_command

		jmp ContinueCommandAddr

finish_command:
		jmp FinishCommandAddr

bad_client:
		pop edi
		pop ebp
		pop ebx
		add esp, 0x54
		ret
	}
}
#else
void __declspec(naked) RconHandlerCodecave()
{
	__asm
	{
		lea edx, [esp+0x0c]
		lea ecx, [esp+0x15]

		push ecx
		push edx
		push ebp
		call RconHandler
		cmp eax, -1
		je bad_client
		cmp eax, 2
		jb finish_command

		jmp ContinueCommandAddr

finish_command:
		jmp FinishCommandAddr

bad_client:
		pop edi
		pop ebp
		add esp, 0x50
		ret
	}
}
#endif

void InstallRconHandler()
{
	Patches.NewPatch(new CodeCaveHook("Rcon Hook", RconHookCode, 6, 0, NULL, RconHandlerCodecave));
	Patches.NewPatch(new FunctionAddress("Rcon Continue", RconContinueCode, 6, &ContinueCommandAddr));
	Patches.NewPatch(new FunctionAddress("Rcon Finish", RconFinishCode, 0, &FinishCommandAddr));	
}