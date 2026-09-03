#include "chat.h"

#include "Addresses.h"
#include "bans.h"
#include "CommandExecuter.h"
#include "HaloUtilities.h"
#include "LogThread.h"
#include "LuaHandler.h"
#include "Mapcycle.h"
#include "MapVote.h"
#include "MiscellaneousFunctions.h"
#include "NetworkEvents.h"
#include "OnlineAdmins.h"
#include "Patcher.h"
#include "Prefs.h"
#include "RemoteConsole.h"
#include "TextUtilities.h"

int ChatSpam[16] = {};
bool Skips[16] = {};

void TextBan(int PlayerIndex)
{
	DWORD mins;

	if(Server->ServerState == 1)
		mins = 60;
	else
		mins = 10;

	GlobalTextBans.Ban(PlayerIndex, mins);

	sv_say(FormatWArgs(L"%ls was textbanned for spamming (%u min).", PlayerTable[PlayerIndex].name, mins).c_str());
	WriteLog(L"TEXTBAN", FormatWArgs(L"Player %ls was textbanned for spamming", PlayerTable[PlayerIndex].name));
}

void Mute(int PlayerIndex)
{
	DWORD mins;

	if(Server->ServerState == 1)
		mins = 60;
	else
		mins = 10;

	GlobalMutedPlayers.Ban(PlayerIndex, mins);

	sv_say(FormatWArgs(L"%ls was muted for spamming (%u min).", PlayerTable[PlayerIndex].name, mins).c_str());
	WriteLog(L"MUTE", FormatWArgs(L"Player %ls was muted for spamming", PlayerTable[PlayerIndex].name));
}

void Skip(int PlayerIndex)
{
	if(GlobalPrefs->MapSkip && Server->ServerState == 1)
	{
		if(!Skips[PlayerIndex])
		{
			Skips[PlayerIndex] = true;
			size_t RealPlayerNumber = 0;
			size_t Skippers = 0;
			for (size_t i = 0; i < PlayerHeader->SlotsTaken; i++)
			{
				if (PlayerPresent(i))
				{
					RealPlayerNumber++;
					if (Skips[i])
						Skippers++;
				}
			}
				
			int need = (int)ceil((float)(GlobalPrefs->MapSkip / (100.0f / RealPlayerNumber))) - Skippers;
			if(need > 0)
				sv_say(FormatWArgs(L"%ls voted to skip this map; %d more votes are required to skip!", PlayerTable[PlayerIndex].name, need).c_str());
			else
			{
				sv_say(FormatWArgs(L"%ls voted to skip this map; enough people voted to skip!", PlayerTable[PlayerIndex].name).c_str());
				GlobalPrefs->MapSkip = 0;
				if(!GlobalPrefs->SappMapcycle || !GlobalMapcycle.Next())
					ExecuteHaloCommand(-1, Enviroment::CONSOLE, "sv_map_next");
			}
		}
		else
			MsgSend(PlayerIndex, "You already voted to skip this map!");
	}
	else
		MsgSend(PlayerIndex, "Map skipping is disabled!");
}

void CommandExecute(int PlayerIndex, int level, std::wstring &Text)
{
	Text.erase(Text.begin());

	if (level > -1 && level < 5)
		WriteLog(L"COMMAND_CHAT", FormatWArgs(L"Admin %ls executed the following command: %ls", PlayerTable[PlayerIndex].name, Text.c_str()));
	else if (level == -1)
		WriteLog(L"COMMAND_CHAT", FormatWArgs(L"Player %ls executed the following command: %ls", PlayerTable[PlayerIndex].name, Text.c_str()));

	bool block = false;
	if (GlobalPrefs->Lua && !GlobalPrefs->ScrimMode && level < 5)
	{
		std::vector<std::unique_ptr<lua::LuaEventArgBase>> args;
		args.emplace_back(std::make_unique<lua::LuaEventArgString>(WToStr(Text)));
		args.emplace_back(std::make_unique<lua::LuaEventArgInt>(2));
		block = !lua::LuaEventCallback(EVENT_COMMAND, PlayerIndex, args);
	}

	CommandEnviroment env(Enviroment::CHAT, PlayerIndex, level);
	if (!_wcsicmp(Text.c_str(), L"unload"))
		env.Print(L"You can only use this command from the rcon or the console!");
	else
		ExecuteSingleCommand(Text, env, block);
}

void LogChat(int PlayerIndex, int Type, const std::wstring &ChatText)
{
	wchar_t* type;
	switch (Type)
	{
	case 0:
		type = L"GLOBAL";
		break;
	case 1:
		type = L"TEAM";
		break;
	case 2:
		type = L"VEHICLE";
		break;
	default:
		type = L"OTHER";
	}

	WriteLog(L"CHAT", FormatWArgs(L"[%ls] %ls: %ls", type, PlayerTable[PlayerIndex].name, ChatText.c_str()));

	if(GlobalPrefs->ConChatEcho)
		PrintConsole(FormatArgs("[%ls] %hs: %hs", type, WToStr(PlayerTable[PlayerIndex].name).c_str(), WToStr(ChatText).c_str()).c_str());
}

bool __stdcall ChatHandler(Player_Machine_Info* PlayerMachine, int Type, wchar_t* Text)
{
	if (!ServerRunning())
		return false;

	size_t PlayerMachineNumber = -1;
	for (size_t i = 0; i < 16; i++)
	{
		if(PlayerMachine == &Server->MachineInfo[i])
		{
			PlayerMachineNumber = i;
			break;
		}
	}

	int PlayerIndex = NumberToIndex(PlayerMachineNumber);

	if(!PlayerPresent(PlayerIndex))
		return false;

	if (Type == 5)
	{
		HandleNetworkEvents(PlayerIndex, reinterpret_cast<NetworkEventBase*>(Text));
		return false;
	}

	std::wstring ChatText(Text);

	int level = GlobalOnlineAdminList.GetAdminLevel(PlayerIndex);

	if(!FastValidate(PlayerIndex, level, ChatText.c_str(), ChatText.length()))
		return false;

	if (GlobalPrefs->Lua && !GlobalPrefs->ScrimMode && level >= -1 && level < 5)
	{
		std::vector<std::unique_ptr<lua::LuaEventArgBase>> args;
		args.emplace_back(std::make_unique<lua::LuaEventArgString>(WToStr(ChatText)));
		args.emplace_back(std::make_unique<lua::LuaEventArgInt>(Type));
		if (!lua::LuaEventCallback(EVENT_CHAT, PlayerIndex, args))
			return false;
	}
	
	if (level < 0)
	{
		if (GlobalTextBans.IsBanned(PlayerIndex))
		{
			return false;
		}
		else if (GlobalPrefs->AntiSpam == 1 && ChatSpam[PlayerIndex] > 150)
		{
			TextBan(PlayerIndex);
			return false;
		}

		if (GlobalMutedPlayers.IsBanned(PlayerIndex))
		{
			return false;
		}
		else if (GlobalPrefs->AntiSpam == 2 && ChatSpam[PlayerIndex] > 150)
		{
			Mute(PlayerIndex);
			return false;
		}
	}
	
	if(ChatText.length() < 3)
	{
		if(Server->ServerState == 2)
		{
			size_t vote;
			if(ToNumber(ChatText, vote))
				GlobalMapVotes.Vote(PlayerIndex, vote);
		}

	}
	else if(ChatText.length() == 4)
	{
		if(!_wcsicmp(ChatText.c_str(), L"skip"))
			Skip(PlayerIndex);
	}

	if((ChatText[0] == GlobalPrefs->CmdStart1 && ChatText[1] == GlobalPrefs->CmdStart1) || (ChatText[0] == GlobalPrefs->CmdStart2 && ChatText[1] == GlobalPrefs->CmdStart2))
	{
		ChatText.erase(ChatText.begin());
		wcsncpy(Text, ChatText.c_str(), ChatText.length() + 1);
	}
	else if(ChatText[0] == GlobalPrefs->CmdStart1 || ChatText[0] == GlobalPrefs->CmdStart2)
	{	
		CommandExecute(PlayerIndex, level, ChatText);
		return false;
	}

	LogChat(PlayerIndex, Type, ChatText);

	if(GlobalPrefs->AntiCaps && level < 0 && ChatText.length() > 8)
	{
		size_t NumberOfCaps = 0;
		for(size_t i = 0; i < ChatText.length(); i++)
			if(ChatText[i] >= 'A' && ChatText[i] <= 'Z')
				NumberOfCaps++;

		if(NumberOfCaps > ChatText.length() / 2)
			ToLower(Text);
	}

	if (GlobalPrefs->RemoteConsole)
	{
		GlobalRemoteConsole.SendRemoteChat(PlayerMachineNumber + 1, Type, Text);
	}

	ChatSpam[PlayerIndex] += 50;

	stats::Chat++;

	return true;
}

DWORD NormalFlowAddress;
#ifndef HPC
void __declspec(naked) ChatHandlerCodecave()
{
	__asm
	{
		je disallow_text

		mov ecx, [esp + 0x1c]
		mov eax, [esp + 0x24]
		
		push eax
		push ecx
		push esi
		call ChatHandler
		test al, al
		je disallow_text

		jmp NormalFlowAddress

	disallow_text:
		pop edi
		add esp, 0x228
		ret
	}
}
#else
void __declspec(naked) ChatHandlerCodecave()
{
	__asm
	{
		je disallow_text

		mov ecx, [esp + 0x08]
		mov eax, [esp + 0x10]
		
		push eax
		push ecx
		push esi
		call ChatHandler
		test al, al
		je disallow_text

		jmp NormalFlowAddress

	disallow_text:
		pop edi
		add esp, 0x224
		ret
	}
}
#endif

void InstallChatHandler()
{
	Patches.NewPatch(new CodeCaveHook("Chat Hook", ChatCode, 6, 49, &NormalFlowAddress, ChatHandlerCodecave));
}