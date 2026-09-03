#pragma once

#include "NetworkStructs.h"
#include "Events\ACEvents.h"
#include "DynamicCodeXOR.h"
#include "sha256.h"
#include "sha256_file.h"
#include "RC6\rc6_64.h"
#include "RC6\rc6_16.h"
#include "PerfomanceCounter.h"

class NetworkClient
{
	SOCKET s;
	HANDLE mySemaphore;
	Locker sLock;
	PerfomanceCounter PC;
	bool Crypted;
	RC6_16 headerCrypt;
	RC6_64 myCrypt;
	static void StartThread(void* ClassPointer);
	void ClientThread();
	bool ProcessPacket(const PacketHeader& Header, char* Buffer);
	bool ProcessPacket_General(const PacketHeader& Header, char* Buffer);
	bool ProcessPacket_SappServer(const PacketHeader& Header, char* Buffer);
	bool ProcessPacket_HAC(const PacketHeader& Header, char* Buffer);
	bool sendData(void* data, int len);
public:	
	NetworkClient(SOCKET s) : s(s), mySemaphore(CreateSemaphore(NULL, 0, 1, NULL)), Crypted(0) {}
	~NetworkClient()
	{
		CloseHandle(mySemaphore);
	}
	bool StartWait(DWORD time);
	void EndWait();
	bool Start();
	bool Connect(const char* addr);
	bool Connect();
	void Disconnect();
	void SendSmallEvent(event_type type, WORD flags = 0);
	void SendBigEvent(event_type type, char* buff, size_t size);
	//void SendFile();
};

extern NetworkClient* myClient;