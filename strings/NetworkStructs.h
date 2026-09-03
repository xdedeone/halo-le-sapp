#pragma once

#include <Windows.h>

#pragma pack(push, 1)

struct NetworkCoreStruct
{
	DWORD Key;
	SOCKET s;
};

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

struct loginfoStr
{
	char username[32];
	char password[32];
};

#pragma pack(pop)