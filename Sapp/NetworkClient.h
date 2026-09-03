#pragma once

#include <string>
#include <vector>

#include "Events\ACEvents.h"

#include "CommandEnviroment.h"
#include "NetworkStructs.h"
#include "SuperHeader.h"
#include "Threads.h"

extern bool downloading;

extern Enviroment MapDownload_Env;
extern DWORD MapDownload_PlayerIndex;
extern time_t MapDownload_LastQuery;

extern std::vector <MapDownloadEntry> QueryedMaps;

extern bool UpdateAvalibe;
extern bool FilesReceived[3];

class NetworkClient
{
	SOCKET s;
	Locker sendLock;
	bool connected;
	static void StartThread(std::shared_ptr<work>& hThread);
	void ClientThreadProc(std::shared_ptr<work>& hThread);
	bool ProcessPacket(const PacketHeader& Header, const char* Buffer);
	bool ProcessPacket_General(const PacketHeader& Header, const char* Buffer);
	bool ProcessPacket_SappServer(const PacketHeader& Header, const char* Buffer);
	bool ProcessPacket_HAC(const PacketHeader& Header, const char* Buffer);
	bool sendNoLock(const void* data, int len);
	bool sendHeader(const PacketHeader& Header);
	bool sendData(const void* data, int len);
	bool sendDataWithHeader(const PacketHeader& Header, const void* data, int len);
	bool Connect(const char* addr);
	bool Connect(std::shared_ptr<work>& hThread);

public:
	NetworkClient() : s(0), connected(0) {}
	void Start();
	void Stop();
	bool IsConnected();
	void GetIP();
	void VersionCheck();
	void SendStats();
	void DownloadMap(DWORD ID);
	void DownloadMapQuery(const std::string& name);
	void ACLogin();
	void SendSmallEvent(event_type type);
	void SendBigEvent(event_type type, char* buff, size_t size);
#ifdef HALORANK
	void SendHREvent(HR_Event* Event, size_t size);
#endif
};

extern NetworkClient GlobalSAPPServer;