#include "NetworkEvents.h"

#include "Addresses.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "NetworkClient.h"
#include "OnlineAdmins.h"
#include "Patcher.h"
#include "Prefs.h"

void Anticheat_Session(int PlayerIndex, const ACSession* data)
{
	AnticheatPlayers.emplace_back(data->SessionKey, PlayerIndex, IndexToNumber(PlayerIndex), (DWORD)(PlayerID[PlayerIndex] >> 16));
	NetworkEventPlayer<ACSession>(PlayerIndex, ServerID);
	GlobalSAPPServer.SendBigEvent(event_join_server, (char*)&AnticheatPlayers.back(), sizeof(SappAnticheatPlayer));
}

void DataSync_Handshake(int PlayerIndex, const DSHandshake* data)
{
	PlayerApps[PlayerIndex] = data->App;
	NetworkEventPlayer<DSHandshake>(PlayerIndex, APP_SAPP, CurrentVersion);
	//NetworkEventPlayer<DataSync<char>>(PlayerIndex, NE_DataSync_Level, (char)GlobalOnlineAdminList.GetAdminLevel(PlayerIndex));
	NetworkEventPlayer<DataSync<float>>(PlayerIndex, NE_DataSync_GameSpeed, *GameSpeed);
	NetworkEventPlayer<DataSync<float>>(PlayerIndex, NE_DataSync_Gravity, *Gravity);
	NetworkEventPlayer<DataSync<bool>>(PlayerIndex, NE_DataSync_MTV, GlobalPrefs->MTV);
	NetworkEventPlayer<DataSync<bool>>(PlayerIndex, NE_DataSync_SJ, GlobalOnlineAdminList.GetAdminLevel(PlayerIndex) >= GlobalPrefs->SJLevel);
	NetworkEventPlayer<DataSync<byte>>(PlayerIndex, NE_DataSync_Zombies, GlobalPrefs->Zombies);
	NetworkEventPlayer<DataSync<int>>(PlayerIndex, NE_DataSync_TimeLeft, GetTimeLeft());
	NetworkEventPlayer<DataSync<int>>(PlayerIndex, NE_DataSync_TimeElapsed, *GameTicks);
}

void HandleNetworkEvents(int PlayerIndex, const NetworkEventBase* data)
{
	switch (data->Type)
	{
	case NE_Anticheat:
	{
		switch (data->Opcode)
		{
		case NE_Anticheat_Session:
		{
			Anticheat_Session(PlayerIndex, static_cast<const ACSession*>(data));
			break;
		}
		}
		break;
	}
	case NE_DataSync:
	{
		switch (data->Opcode)
		{
		case NE_DataSync_Handshake:
		{
			DataSync_Handshake(PlayerIndex, static_cast<const DSHandshake*>(data));
			break;
		}
		}
		break;
	}
	}
}

DWORD NetworkEventPacket = 0;

void BroadcastPacket(wchar_t* data, size_t size)
{
	ChatData d;
	d.type = 5;
	d.player = -1;
	d.msg = data;

	ChatData* d_ptr = &d;

	NetworkEventPacket = size;
	DWORD retval = BuildPacket(0, 0x0F, 0, (LPBYTE)&d_ptr, 0, 1, 0);
	NetworkEventPacket = 0;

	if (retval)
	{
		AddPacketToGlobalQueue(retval, 1, 1, 0, 1, 3);
	}
}

void SendPacket(int PlayerIndex, wchar_t* data, size_t size)
{
	ChatData d;
	d.type = 5;
	d.player = -1;
	d.msg = data;

	ChatData* d_ptr = &d;

	NetworkEventPacket = size;
	DWORD retval = BuildPacket(0, 0x0F, 0, (LPBYTE)&d_ptr, 0, 1, 0);
	NetworkEventPacket = 0;

	if (retval)
	{
		AddPacketToPlayerQueue(IndexToNumber(PlayerIndex), retval, 1, 1, 0, 1, 3);
	}
}

void __declspec(naked) BuildChatPacketHookCC()
{
	__asm
	{
		mov eax, NetworkEventPacket
		cmp eax, 0
		jne no_wcslen
			mov eax, dword ptr ss : [esp + 4]
			do_len:
				mov cx, word ptr ds : [eax]
				add eax, 2
				test cx, cx
			jnz do_len
			sub eax, dword ptr ss : [esp + 4]
			sar eax, 1
			sub eax, 1
	no_wcslen:
		retn
	}
}

void InstallBuildChatPacketHook()
{
	Patches.NewPatch(new CodeCaveHook("BuildChatPacketHook", BuildChatPacketHook, 5, 7, NULL, BuildChatPacketHookCC, true));
}