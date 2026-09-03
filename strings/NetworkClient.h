#pragma once

#include <string>
#include <vector>

#include "Events\ACEvents.h"
#include "RC6\rc6_64.h"
#include "RC6\rc6_16.h"

#include "NetworkStructs.h"

enum LoginState { success, connection_terminated, timeout, invalid_user, not_activated, banned };

class NetworkClient
{
	SOCKET s;
	HANDLE mySemaphore;
	HANDLE myThread;
	bool connected;
	bool ShutDown;
	bool Crypted;
	RC6_16 headerCrypt;
	RC6_64 myCrypt;
	LoginState SessionState; // 0: initalization failed, 1: success, 2: connection terminated, 3: timeout, 4: no login data, 5: invalid login info, 6: wrong user, 7: banned user
	DWORD LoginSessionKey;
	static unsigned int __stdcall StartThread(void* ClassPointer);
	void ClientThread();
	bool ProcessPacket(const PacketHeader& Header, char* Buffer);
	//bool ProcessPacket_General(const PacketHeader& Header, char* Buffer);
	//bool ProcessPacket_SappServer(const PacketHeader& Header, char* Buffer);
	bool ProcessPacket_HAC(const PacketHeader& Header, char* Buffer);
	bool sendData(void* data, int len);
public:
	NetworkClient() : s(0), mySemaphore(CreateSemaphore(NULL, 0, 1, NULL)), myThread(NULL), connected(0), ShutDown(0), Crypted(0), SessionState(success), LoginSessionKey(0) {}
	~NetworkClient()
	{
		CloseHandle(mySemaphore);
	}
	bool StartWait(DWORD time);
	void EndWait();
	bool Connect(const char* addr);
	bool Connect();
	void Disconnect();
	bool IsConnected();
	LoginState GetRC6();
	LoginState Login(loginfoStr &LogInfo);
	LoginState GetCortana();
};

void WSInit();
void WSShutDown();