#pragma once

#include <Windows.h>

class SappServer;

#pragma pack(push, 1)

struct PacketHeader
{
	DWORD Size;
	union
	{
		struct
		{
			DWORD Info;
		};
		struct
		{
			BYTE Type;
			BYTE SubType;
			WORD Flags;
		};
	};
};
struct PacketVersion : public PacketHeader
{
	DWORD Version;
};
struct ServerIP : public PacketHeader
{
	DWORD IP;
};
struct ACLogin : public PacketHeader
{
	DWORD ServerID;
};

struct MapDownloadEntry
{
	WORD ID;
	WORD padding;
	char Name[36];
};

struct ServerStats
{
	wchar_t ServerName[64];
	wchar_t Map[32];
	wchar_t Mode[32];
	DWORD IP;
	WORD Port;
	BYTE MaxPlayers;
	BYTE Players;
	BYTE Password;
	BYTE HaloVerType; // PC/CE (0/1)
	BYTE NoLead;
	BYTE HAC;
	DWORD Version;
	char VersionString[16];
	union
	{
		struct
		{
			BYTE V1;
			BYTE V2;
			BYTE V3;
			BYTE V4;
		};
		struct
		{
			DWORD SappVersion;
		};
	};
	char GameType[32];
};

struct ACJoinInfo
{
	SappServer* ServerID;
	DWORD IP;
	WORD Port;
	wchar_t Name[12];
};

struct SappAnticheatPlayer
{
	DWORD SessionKey;
	DWORD PlayerIndex;
};

struct AnticheatPlayerPacket
{
	DWORD PlayerIndex;
	char AnticheatName[32];
};

struct AntcheatReport
{
	DWORD Reporter;
	DWORD Reported;
	char Message[64];
};

struct CurrentGame
{
	char MapName[32];
	char GTName[32];
	char GameType[31];
	bool TeamGame;
};

struct PlayerStats
{
	wchar_t Name[12];
	char IP[32];
	char Hash[32];
	DWORD ValidKey;
	DWORD Left;
	DWORD Team;
	DWORD Kills;
	DWORD Assists;
	DWORD Deaths;
	DWORD Suicides;
	DWORD Betrays;
	DWORD Experince;
};

struct loginfoStr
{
	char username[32];
	unsigned char password[32];
};

#pragma pack(pop)