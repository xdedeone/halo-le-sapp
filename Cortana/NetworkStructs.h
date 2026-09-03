#pragma once

#include <Windows.h>

class Locker
{
	CRITICAL_SECTION g_cs;
public:
	Locker()
	{
		InitializeCriticalSection(&g_cs);
	}
	void Lock()
	{
		EnterCriticalSection(&g_cs);
	}
	void Unlock()
	{
		LeaveCriticalSection(&g_cs);
	}
	~Locker()
	{
		DeleteCriticalSection(&g_cs);
	}
};

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

struct AnticheatPlayerPacket
{
	DWORD PlayerIndex;
	char AnticheatName[32];
};

#pragma pack(pop)