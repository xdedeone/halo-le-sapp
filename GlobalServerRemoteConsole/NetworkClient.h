#pragma once

#include "NetworkStructs.h"
#include <stdio.h>

class NetworkClient
{
	SOCKET s;
	bool connected;
	bool ShutDown;
	bool ClientThreadRunning;
	DWORD IP;
	DWORD PCFreq;
	LONGLONG QPC;
	static void StartThread(void* ClassPointer);
	void ClientThread();
	bool ProcessPacket(const PacketHeader& Header, char* Buffer);
	bool ProcessPacket_General(const PacketHeader& Header, char* Buffer);
	bool ProcessPacket_SappServer(const PacketHeader& Header, char* Buffer);
public:
	NetworkClient();
	int Connect();
	int Disconnect();
	bool IsConnected();
	void InitQPC();
	void StartQPC();
	DWORD GetQPC();
	void Ping();
	void GetClientNumber();
	void RebootServer();
	void ListClients();
	void GetIP();
	void VersionCheck();
};

bool WSInit();
void WSShutDown();