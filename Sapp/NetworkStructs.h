#pragma once

//#include <Windows.h>

#pragma pack(push, 1)

struct PacketHeader
{
	DWORD Size;
	BYTE Type;
	BYTE SubType;
	WORD Flags;
};

struct PacketVersion : public PacketHeader
{
	DWORD Version;
};

struct ServerIP : public PacketHeader
{
	DWORD IP;
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
	byte MaxPlayers;
	byte Players;
	byte Password;
	byte HaloVerType; // PC/CE (0/1)
	byte NoLead;
	byte HAC;
	DWORD Version;
	char VersionString[16];
	DWORD SappVersion;
	char GameType[32];
};

struct SappAnticheatPlayer
{
	DWORD SessionKey;
	DWORD PlayerIndex;

	SappAnticheatPlayer(DWORD SessionKey, DWORD PlayerIndex) : SessionKey(SessionKey), PlayerIndex(PlayerIndex) {}
};

struct LocalAntcheatPlayer : public SappAnticheatPlayer
{
	DWORD MachineID;
	DWORD IP;
	LocalAntcheatPlayer(DWORD SessionKey, DWORD PlayerIndex, DWORD MachineID, DWORD IP) : SappAnticheatPlayer(SessionKey, PlayerIndex), MachineID(MachineID), IP(IP) {}
};

struct AntcheatReport
{
	DWORD Reporter;
	DWORD Reported;
	char Message[64];
};

#pragma pack(pop)