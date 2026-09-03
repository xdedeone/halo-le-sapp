#pragma once

#include <Windows.h>

#define _PLAYER_NUMBER_ 16

#define pushad \
	__asm push eax   \
	__asm push ecx   \
	__asm push edx   \
	__asm push ebx   \
	__asm push esi   \
	__asm push edi


#define popad \
	__asm pop edi   \
	__asm pop esi   \
	__asm pop ebx   \
	__asm pop edx   \
	__asm pop ecx   \
	__asm pop eax

#pragma pack(push, 1)

struct TagTableHeader
{
	DWORD TagTableEntryPointer;
	DWORD FirstTagID;
	DWORD MapID;
	DWORD TagCount;
	DWORD VerticesCount;
	DWORD VerticesOffset;
	DWORD IndicesCount;
	DWORD IndicesOffset;
	DWORD ModelDataSize;
	DWORD Signature;
};

struct TagEntry
{
	DWORD class0;
	char Class1[4];
	char Class2[4];
	DWORD TagID;
	char* TagNameAddress;
	DWORD TagStruct;
	char Unknown[8];
};

struct NetInfoStruct
{
	union
	{
		struct
		{
			DWORD IP;
		};
		struct
		{
			BYTE IPasBytes[4];
		};
	};	
	WORD Port;
	WORD Pad;
};

struct Player_Machine_Info
{
	NetInfoStruct*** IPAddress;
	DWORD Zero1;
	DWORD Zero2;
	WORD PlayerNumber; // Init to 0xFFFF
	WORD seven;
	BYTE unknown4[24];

	float angle_xy;
	float angle_z;
	BYTE unknown0[44];
	DWORD HashID; // Init to 0xFFFFFFFF
#ifndef HPC
	BYTE unknown1[32];
	char ip[32];  // at 116
	char cd_key_hash[32];
	BYTE unknown[44]; // 172
#endif	
};

struct PresenceItem
{
	wchar_t name[12];
	union
	{
		struct  
		{
			ULONGLONG Prescene; // Init to 0xFFFFFFFFFFFFFFFF
		};
		struct  
		{
			long un1;
			char machine_index;		
			char player_status;
			char team;
			char player_index;
		};
	};
};

struct ServerInfoStruct
{
	DWORD *SomeAddress;
	WORD ServerState;  // 0 - inactive, 1 - game, 2 - game break\results 
	WORD SomeBitmask; // 7 ??
	wchar_t ServerName[66];
	char MapName[128];
	wchar_t GameType[24];
#ifndef HPC
	BYTE unknown2[169];
#else
	BYTE unknown2[105];
#endif
	BYTE MaxPlayers;
	WORD SomeOne;
	BYTE PlayerNumber;
	BYTE unknown3;
	PresenceItem PlayerData[_PLAYER_NUMBER_];
	BYTE unknown4[6];
	DWORD GamesPlayed;
	BYTE unknown5[4];
	Player_Machine_Info MachineInfo[_PLAYER_NUMBER_];
	BYTE unknown6[68];
	wchar_t Password[16];
	DWORD EndPad;
};

#pragma pack(pop)