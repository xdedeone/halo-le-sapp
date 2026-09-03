#include "PingUpdate.h"

#include "Addresses.h"
#include "HaloUtilities.h"
#include "Patcher.h"

double PCFreq;
LARGE_INTEGER LastUpdate[16] = {};
bool WaitingForReply[16] = {};

Ping_Table_Item PingTable[16];
Ping_Table_Item* PingTablePointers[16];

void UpdatePings()
{
	if (GetPlayerNumber() > 0)
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);

		bool PacketBuilt = false;
		DWORD retval;
		for (size_t i = 0; i < 16; i++)
		{
			if (PlayerPresent(i) && double(li.QuadPart - LastUpdate[i].QuadPart) / PCFreq >= 250 && (WaitingForReply[i] == false || double(li.QuadPart - LastUpdate[i].QuadPart) / PCFreq > 1000))
			{
				if (!PacketBuilt)
				{
					size_t PlayerNumber = 0;
					for (size_t j = 0; j < 16; j++)
					{
						if (PlayerPresent(j))
						{
							PingTable[PlayerNumber].PlayerIndex = j;
							PingTable[PlayerNumber].Ping = PlayerTable[j].ping;
							PlayerNumber++;
						}
					}

					retval = BuildPacket(0, 0x35, 0, (LPBYTE)&PingTablePointers, 0, PlayerNumber, 0);
					PacketBuilt = true;
				}

				LastUpdate[i] = li;
				WaitingForReply[i] = true;

				AddPacketToPlayerQueue(IndexToNumber(i), retval, 1, 0, 0, 0, 3);
			}
		}
	}
}

void __stdcall UpdatePlayerPing(Player_Table_Item* Player)
{
	size_t PlayerIndex = Player->Prescene.player_index;
	if (WaitingForReply[PlayerIndex])
	{
		WaitingForReply[PlayerIndex] = false;

		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);

		Player->ping = DWORD(floor(double(li.QuadPart - LastUpdate[PlayerIndex].QuadPart) / PCFreq + 0.5));		
	}
}

DWORD PingUpdateSendEnd;
void __declspec(naked) UpdatePingsCC()
{
	__asm
	{
		call UpdatePings
		jmp PingUpdateSendEnd
	}
}

DWORD PingUpdateRecvEnd;
void __declspec(naked) UpdatePlayerPingCC()
{
	__asm
	{
		push edi
		call UpdatePlayerPing
		jmp PingUpdateRecvEnd
	}
}

void InstallPingUpdateHook()
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	PCFreq = double(li.QuadPart) / 1000;

	for (size_t i = 0; i < 16; i++)
	{
		PingTablePointers[i] = &PingTable[i];
	}

	Patches.NewPatch(new FunctionAddress("PingUpdateRecv Func", PingUpdateCode, 23, &PingUpdateRecvEnd));
	Patches.NewPatch(new CodeCaveHook("PingUpdateRecv Hook", PingUpdateCode, 10, 0, NULL, UpdatePlayerPingCC));
	Patches.NewPatch(new FunctionAddress("PingUpdateSend Func", PingUpdateIntervalCode, 24, &PingUpdateSendEnd));
	Patches.NewPatch(new CodeCaveHook("PingUpdateSend Hook", PingUpdateIntervalCode, 5, -13, NULL, UpdatePingsCC));
}