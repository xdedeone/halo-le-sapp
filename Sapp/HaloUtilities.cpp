#include "HaloUtilities.h"

#include "Addresses.h"
#include "CommandExecuter.h"
#include "Event.h"
#include "EventScan.h"
#include "LogThread.h"
#include "NetworkEvents.h"
#include "Prefs.h"
#include "TextUtilities.h"
#include "OnlineAdmins.h"

bool ServerRunning()
{
	return *ServerPointer != nullptr;
}

int IndexToNumber(size_t index)
{
	if (!ServerRunning() || index > 15)
		return -1;

	return MachineIndexes[index];
}

int NumberToIndex(size_t number)
{
	if (!ServerRunning() || number > 15)
		return -1;
	
	return PlayerIndexes[number];
}

bool PlayerPresent(size_t i)
{
	if (i > 15)
		return false;

	if (Server->PlayerData[i].player_status != 0)
		return false;

	return true;
}

int ToPlayerIndex(size_t PlayerNumber)
{
	int IndexNumber = NumberToIndex(PlayerNumber - 1);
	if (!PlayerPresent(IndexNumber))
		return -1;

	return IndexNumber;
}

bool PlayerInVehicle(size_t playerIndex)
{
	if (!PlayerAlive(playerIndex))
		return false;

	Dynamic_Player *Player = ObjectTable[PlayerTable[playerIndex].ObjectIndexNum].Offset;

	if (Player->VehicleIndexNum == 0xffff)
		return false;

	return true;
}

bool PlayerAlive(size_t i)
{
	if (!PlayerPresent(i))
		return false;
	if (PlayerTable[i].PlayerID == 0)
		return false;
	if (PlayerTable[i].ObjectIndexNum == 0xffff)
		return false;
	if (PlayerTable[i].ObjectID == 0xffff)
		return false;

	return true;
}

bool PlayerAliveOnly(size_t i)
{
	if (PlayerTable[i].PlayerID == 0)
		return false;
	if (PlayerTable[i].ObjectIndexNum == 0xffff)
		return false;
	if (PlayerTable[i].ObjectID == 0xffff)
		return false;

	return true;
}

//before calling this function have to check if the player is alive!
void GetPlayerPosition(size_t PlayerIndex, vect3f &loc)
{
	Dynamic_Player* Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;

	if (Player->VehicleIndexNum == 0xffff)
	{
		loc = Player->PlayerVectors.Pos;
	}
	else
	{
		char* VehicleAddr = (char*)ObjectTable[Player->VehicleIndexNum].Offset;
		loc = *(vect3f*)(VehicleAddr + 0x5c);
	}
}

size_t GetPlayerNumber()
{
	if (ServerRunning())
		return Server->PlayerNumber;
	else
		return 0;
}

size_t GetRedPlayerNumber()
{
	size_t RedPlayerNumber = 0;
	if (ServerRunning())
		for (size_t i = 0; i < 16; i++)
			if (PlayerPresent(i))
				if (PlayerTable[i].Team == 0)
					RedPlayerNumber++;

	return RedPlayerNumber;
}

size_t GetBluePlayerNumber()
{
	size_t BluePlayerNumber = 0;
	if (ServerRunning())
		for (size_t i = 0; i < 16; i++)
			if (PlayerPresent(i))
				if (PlayerTable[i].Team == 1)
					BluePlayerNumber++;

	return BluePlayerNumber;
}

void GetPlayers(int &RealPlayerNumber, size_t *TakenSlotIndexes)
{
	for (size_t i = 0; i < 16; i++)
		if (PlayerPresent(i))
			TakenSlotIndexes[RealPlayerNumber++] = i;
}

void GetReds(int &RedPlayerNumber, size_t *TakenSlotIndexes)
{
	for (size_t i = 0; i < 16; i++)
		if (PlayerPresent(i) && PlayerTable[i].Team == 0)
			TakenSlotIndexes[RedPlayerNumber++] = i;
}

void GetBlues(int &BluePlayerNumber, size_t *TakenSlotIndexes)
{
	for (size_t i = 0; i < 16; i++)
		if (PlayerPresent(i) && PlayerTable[i].Team == 1)
			TakenSlotIndexes[BluePlayerNumber++] = i;
}

int GetPlayerScore(size_t PlayerIndex)
{
	switch (CurrentGameType)
	{
	case GameType::CTF:
		return PlayerTable[PlayerIndex].ctf_score;
	case GameType::Slayer:
		return SlayerScores->PlayerScores[PlayerIndex];
	case GameType::Oddball:
		return *ScoreType == 2 ? OddballScores->PlayerScores[PlayerIndex] : OddballScores->PlayerScores[PlayerIndex] / 30;
	case GameType::King:
		return PlayerTable[PlayerIndex].king_time / 30;
	case GameType::Race:
		return PlayerTable[PlayerIndex].race_score;
	default:
		return 0;
	}
}

int GetTeamScore(size_t TeamIndex)
{
	switch (CurrentGameType)
	{
	case GameType::CTF:
		return TeamIndex == 0 ? CTFScores->Reds : CTFScores->Blues;
	case GameType::Slayer:
		return SlayerScores->TeamScores[TeamIndex];
	case GameType::Oddball:
		return *ScoreType == 2 ? OddballScores->TeamScores[TeamIndex] : OddballScores->TeamScores[TeamIndex] / 30;
	case GameType::King:
		return KingScores->TeamScores[TeamIndex] / 30;
	case GameType::Race:
		return RaceScores->TeamScores[TeamIndex];
	default:
		return 0;
	}
}

std::string GetPlayerHash(size_t MachineID)
{
	if (ServerRunning())
	{
#ifdef HPC
		for (Player_Hash_List* hash_list = PlayerHashList->next; hash_list && hash_list->data; hash_list = hash_list->next)
		{
			if (hash_list->data->id == Server->MachineInfo[MachineID].HashID)
			{
				return hash_list->data->hash;
			}
		}
#else
		return Server->MachineInfo[MachineID].cd_key_hash;
#endif
	}
	return "";
}

std::string GetPlayerIP(size_t MachineID)
{
	if (ServerRunning())
	{
#ifdef HPC
		NetInfoStruct* info = **Server->MachineInfo[MachineID].IPAddress;
		if (info != NULL)
		{
			return FormatArgs("%hhu.%hhu.%hhu.%hhu:%hu", info->IPasBytes[0], info->IPasBytes[1], info->IPasBytes[2], info->IPasBytes[3], info->Port);
		}
#else
		return Server->MachineInfo[MachineID].ip;
#endif
	}
	return "";
}

void sv_kick(size_t i)
{
	ExecuteHaloCommand(-1, Enviroment::CONSOLE, FormatArgs("sv_kick %d", IndexToNumber(i) + 1).c_str());
	stats::Commands++;
}
void sv_ban(size_t i, const char* s)
{
	ExecuteHaloCommand(-1, Enviroment::CONSOLE, FormatArgs("sv_ban %d %hs", IndexToNumber(i) + 1, s).c_str());
	stats::Commands++;
}

void crDirectory(const std::wstring &name)
{
	if(CreateDirectoryW(name.c_str(), NULL) == 0)
		if(GetLastError() != ERROR_ALREADY_EXISTS)
			WriteLog(L"CRDIR", FormatWArgs(L"ERROR: Failed to create \"%ls\" directory", name.c_str()));
}
void crFile(const std::wstring &name)
{
	static bool created = false;
	if (!created)
	{
		crDirectory(L"sapp");
		crDirectory(FormatWArgs(L"%hs\\sapp", GametypesFolder));
		crDirectory(FormatWArgs(L"%hs\\sapp\\lua", GametypesFolder));
		created = true;
	}
	if(CreateFileW(name.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0) == INVALID_HANDLE_VALUE)
	{
		if(GetLastError() != ERROR_FILE_EXISTS)
			WriteLog(L"CRFILE", FormatWArgs(L"ERROR: Failed to create \"%ls\" file", name.c_str()));
	}
	else
	{
		FILE *file;
		_wfopen_s(&file, name.c_str(), L"a+, ccs=UTF-16LE");
		if (file)
		{
			fclose(file);
		}
	}
}

void __declspec(naked) PrintConsoleAsm(const char* buffer)
{
	__asm
	{
		push ebp
		mov ebp, esp

		push CommandExecuting
		mov CommandExecuting, -1
		
		mov eax, buffer
		call HaloPrintAddress

		pop CommandExecuting

		mov esp, ebp
		pop ebp
		retn
	}
}

void __declspec(naked) PrintRconAsm(size_t PlayerIndex, const char* buffer)
{
	__asm
	{
		push ebp
		mov ebp, esp

		mov eax, buffer
		push PlayerIndex
		call HaloRconPrintAddress
		add esp, 4	

		mov esp, ebp
		pop ebp
		retn
	}
}

void PrintConsole(const char* buffer)
{
	PrintConsole(GlobalPrefs->ConAttrib, buffer);
}

void PrintConsole(WORD attrib, const char* buffer)
{
	if (GlobalPrefs->ConAttrib != attrib)
	{
		SetConsoleTextAttribute(hConsole, attrib);
		PrintConsoleAsm(buffer);
		SetConsoleTextAttribute(hConsole, GlobalPrefs->ConAttrib);
	}
	else
	{
		PrintConsoleAsm(buffer);
	}
 }

void PrintRcon(size_t PlayerIndex, const char* buffer)
{
	if (ServerRunning() && PlayerPresent(PlayerIndex))
	{
		int MachineIndex = IndexToNumber(PlayerIndex);

		PrintRconAsm(MachineIndex, buffer);
	}
}

BYTE BuildPacketBuffer[4096];
DWORD BuildPacket(DWORD arg1, DWORD packettype, DWORD arg3, LPBYTE dataPtr, DWORD arg4, DWORD arg5, DWORD arg6)
{
	__asm
	{
		push arg6
		push arg5
		push arg4
		push dataPtr
		push arg3
		push packettype
		push arg1
		mov edx, 0x7FF8
		lea eax, BuildPacketBuffer
		call FUNC_BUILDPACKET
		add esp, 0x1c
	}
}

DWORD BuildPacketProjectileWrapper(DWORD ObjectIndex)
{
	__asm
	{
		push 0x7FF8;
		lea eax, BuildPacketBuffer
		push eax
		push ObjectIndex
		call FUNC_BUILDPACKET_PROJWRAPPER
		add esp, 0x0c
	}
}

void AddPacketToGlobalQueue(DWORD packetCode, DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4, DWORD arg5)
{
	__asm
	{
		mov ecx, dword ptr ds : [ServerPointer]
		mov ecx, [ecx]

		cmp ecx, 0
		je not_ready_to_send

		push arg5
		push arg4
		push arg3
		push arg2
		lea eax, BuildPacketBuffer
		push eax
		push arg1
		mov eax, packetCode
		call FUNC_ADDPACKETTOQUEUE
		add esp, 0x18

	not_ready_to_send:
	}
}

void AddPacketToPlayerQueue(DWORD player, DWORD packetCode, DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4, DWORD arg5)
{
	__asm
	{
		mov esi, dword ptr ds : [ServerPointer]
		mov esi, [esi]

		cmp esi, 0
		je not_ready_to_send

		push arg5
		push arg4
		push arg3
		push arg2
		push packetCode
		lea eax, BuildPacketBuffer
		push eax
		push arg1
		mov eax, player
		call FUNC_ADDPACKETTOPQUEUE
		add esp, 0x1c

	not_ready_to_send:
	}
}

void BroadcastMotd(const char* message)
{
	const char* msg = message;

	DWORD retval = BuildPacket(0, 0x36, 0, (LPBYTE)&msg, 0, 1, 0);
	if (retval)
	{
		AddPacketToGlobalQueue(retval, 1, 1, 0, 0, 3);
	}
}

DWORD sv_say_player = 0;

void ChatSend(size_t PlayerIndex, std::wstring& message)
{
	ChatData d;
	d.type = 3;
	if (GlobalPrefs->SPre)
		d.player = -1;
	else
		d.player = PlayerIndex;
	d.msg = message.c_str();

	if (message.length() > 250)
		message[250] = 0;

	ChatData* d_ptr = &d;

	DWORD retval = BuildPacket(0, 0x0F, 0, (LPBYTE)&d_ptr, 0, 1, 0);
	if (retval)
	{
		AddPacketToPlayerQueue(IndexToNumber(PlayerIndex), retval, 1, 1, 0, 1, 3);
	}
}

void ChatSendAll(std::wstring& message)
{
	ChatData d;
	d.type = 3;
	if (GlobalPrefs->SPre)
	{
		d.player = -1;
	}
	else
	{
		d.player = sv_say_player;
	}
	d.msg = message.c_str();

	if (message.length() > 250)
		message[250] = 0;

	ChatData* d_ptr = &d;

	DWORD retval = BuildPacket(0, 0x0F, 0, (LPBYTE)&d_ptr, 0, 1, 0);
	if (retval)
	{
		AddPacketToGlobalQueue(retval, 1, 1, 0, 1, 3);
	}
}

void MsgSend(size_t PlayerIndex, const char* message, bool admin)
{
	if (ServerRunning())
	{
		ChatSend(PlayerIndex, FormatWArgs(L"%ls%hs", admin ? GlobalPrefs->admin_prefix.c_str() : GlobalPrefs->msg_prefix.c_str(), message));
	}
}

void MsgSend(size_t PlayerIndex, const wchar_t* message, bool admin)
{
	if (ServerRunning())
	{
		ChatSend(PlayerIndex, FormatWArgs(L"%ls%ls", admin ? GlobalPrefs->admin_prefix.c_str() : GlobalPrefs->msg_prefix.c_str(), message));
	}
}

void sv_say(const char* message, bool admin)
{
	if (ServerRunning() && Server->PlayerNumber > 0)
	{
		ChatSendAll(FormatWArgs(L"%ls%hs", admin ? GlobalPrefs->admin_prefix.c_str() : GlobalPrefs->msg_prefix.c_str(), message));
	}
}

void sv_say(const wchar_t* message, bool admin)
{
	if (ServerRunning() && Server->PlayerNumber > 0)
	{
		ChatSendAll(FormatWArgs(L"%ls%ls", admin ? GlobalPrefs->admin_prefix.c_str() : GlobalPrefs->msg_prefix.c_str(), message));
	}
}

void Login(size_t PlayerIndex, int level)
{
	GlobalOnlineAdminList.SetAdminLevel(PlayerIndex, level);
	WriteLog(L"LOGIN", FormatWArgs(L"Admin %ls level %d has successfully logged in", Server->PlayerData[PlayerIndex].name, level));
	GlobalEventHandler.Execute(PlayerIndex, EVENT_LOGIN, std::vector <std::pair<std::wstring, std::wstring>>());
	NetworkEventPlayer<DataSync<bool>>(PlayerIndex, NE_DataSync_SJ, GlobalOnlineAdminList.GetAdminLevel(PlayerIndex) >= GlobalPrefs->SJLevel);
}

void UpdateSJLevel()
{
	for (size_t i = 0; i < 16; i++)
	{
		if (PlayerPresent(i))
		{
			NetworkEventPlayer<DataSync<bool>>(i, NE_DataSync_SJ, GlobalOnlineAdminList.GetAdminLevel(i) >= GlobalPrefs->SJLevel);
		}
	}
}