#pragma once

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;
typedef unsigned int SOCKET;

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
	DWORD SappVersion;
};

#pragma pack(pop)