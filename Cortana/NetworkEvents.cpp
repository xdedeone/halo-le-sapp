#include "NetworkEvents.h"

#include "Addresses.h"
#include "Patch.h"

short GameSpeedSig[] = { 0xD8, 0x0D, -1, -1, -1, -1, 0x83, 0xEC, 0x08, 0xD9, 0x5C, 0x24, 0x08 }; // D80D????????83EC08D95C2408
short GravitySig[] = { 0xD8, 0x25, - 1, -1, -1, -1, 0xD9, 0x5C, 0x24, 0x24, 0xD9, 0x44, 0x24, 0x1C }; // D825????????D95C2424D944241C
short NetworkEventsSig[] = { 0x8B, 0x44, 0x24, 0x0C, 0x83, 0xF8, 0x04, 0x75 }; // 8B44240C83F80475
short GetPlayerIndexSig[] = { 0x0F, 0x84, -1, -1, -1, -1, 0x53, 0xE8, -1, -1, -1, -1, 0x8B, 0xD8, 0x83, 0xFB, 0xFF }; // 0F84????????53E8????????8BD883FBFF
short SendChatMessageSig[] = { 0x53, 0x8D, 0x54, 0x24, 0x1C, 0x66, 0xC7, 0x44, 0x7C, 0x1C, 0x00, 0x00 }; // 538D54241C66C7447C1C0000
short BuildChatPacketHookSig[] = { 0x8B, 0x7C, 0x24, 0x18, 0x57, 0x33, 0xF6, 0xE8 };  // 8B7C24185733F6E8

DWORD GetPlayerIndexFunc;
DWORD SendChatMessageFunc;

void Anticheat_Session(const ACSession* data)
{
	ENCRYPT_BEGIN
	Info.ServerID = data->SessionKey;
	myClient->SendBigEvent(event_join_client, (char*)&Info, sizeof(ACJoinInfo));
	ENCRYPT_END
}

void DataSync_Handshake(const DSHandshake* data)
{
	ENCRYPT_BEGIN
	ServerApp = data->App;
	ServerAppVersion = data->Version;
	ENCRYPT_END
}

void DataSync_GameSpeed(const DataSync<float>* data)
{
	ENCRYPT_BEGIN
	DWORD oldProtect;
	VirtualProtect(GameSpeed, 4, PAGE_READWRITE, &oldProtect);
	*GameSpeed = data->Data;
	VirtualProtect(GameSpeed, 4, oldProtect, &oldProtect);
	ENCRYPT_END
}

void DataSync_Gravity(const DataSync<float>* data)
{
	ENCRYPT_BEGIN
	DWORD oldProtect;
	VirtualProtect(GameSpeed, 4, PAGE_READWRITE, &oldProtect);
	*Gravity = data->Data;
	VirtualProtect(GameSpeed, 4, oldProtect, &oldProtect);
	ENCRYPT_END
}

void _stdcall NetworkEventsHook(const NetworkEventBase* data)
{
	switch (data->Type)
	{
	case NE_Anticheat:
	{
		switch (data->Opcode)
		{
		case NE_Anticheat_Session:
		{
			Anticheat_Session(static_cast<const ACSession*>(data));
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
			DataSync_Handshake(static_cast<const DSHandshake*>(data));
			break;
		}
		case NE_DataSync_GameSpeed:
		{
			DataSync_GameSpeed(static_cast<const DataSync<float>*>(data));
			break;
		}
		case NE_DataSync_Gravity:
		{
			if (ServerAppVersion > 0x09080100)
			{
				DataSync_Gravity(static_cast<const DataSync<float>*>(data));
			}
			break;
		}
		case NE_DataSync_TimeLeft:
		{
			TimeLeft = static_cast<const DataSync<int>*>(data)->Data;
			break;
		}
		}
		break;
	}
	}
}

DWORD NetworkEventPacket = 0;

void SendPacket(void* data, size_t len)
{
	__asm
	{
		ENCRYPT_BEGIN
		pushad
		call GetPlayerIndexFunc
		cmp eax, -1
		je not_in_server
		push eax
		mov edx, data
		mov eax, len
		mov NetworkEventPacket, eax
		mov eax, 5
		call SendChatMessageFunc
		mov NetworkEventPacket, 0
		add esp, 4
	not_in_server:
		popad
		ENCRYPT_END
	}
}

DWORD BackToNetworkEventsHook;
void __declspec(naked) NetworkEventsHookCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		mov eax, dword ptr ss : [esp + 0x0c]
		cmp eax, 5
		jne not_event
		pushad
		mov eax, dword ptr ss : [esp + 0x2c]
		push eax
		call NetworkEventsHook
		popad
	not_event:
		ENCRYPT_END
		cmp eax, 4
		jmp BackToNetworkEventsHook
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

void __declspec(noinline) InstallNetworkeventsHook()
{
	ENCRYPT_BEGIN
	Patches.NewPatch(new MemoryAddress(hhalo, GameSpeedSig, sizeof(GameSpeedSig), 2, (DWORD*)&GameSpeed));
	Patches.NewPatch(new MemoryAddress(hhalo, GravitySig, sizeof(GravitySig), 2, (DWORD*)&Gravity));
	Patches.NewPatch(new CodeCaveHook(hhalo, NetworkEventsSig, sizeof(NetworkEventsSig), 7, 0, &BackToNetworkEventsHook, NetworkEventsHookCC));
	Patches.NewPatch(new MemoryValueFromOPCode(hhalo, GetPlayerIndexSig, sizeof(GetPlayerIndexSig), 8, &GetPlayerIndexFunc));
	Patches.NewPatch(new MemoryValueFromOPCode(hhalo, SendChatMessageSig, sizeof(SendChatMessageSig), 13, &SendChatMessageFunc));
	Patches.NewPatch(new CodeCaveHook(hhalo, BuildChatPacketHookSig, sizeof(BuildChatPacketHookSig), 5, 7, NULL, BuildChatPacketHookCC, true));
	ENCRYPT_END
}