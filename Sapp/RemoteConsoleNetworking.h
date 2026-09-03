#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "Locker.h"
#include "SystemMonitor.h"
#include "Threads.h"

class RemoteClient final
{
	friend class RemoteConsoleNetworking;
	friend class RemoteConsole;

	unsigned int s;
	union
	{
		struct
		{
			unsigned long ip;
		};
		struct
		{
			unsigned char IPasBytes[4];
		};
	};
	int level;
	std::wstring name;
	std::string pending_json;
	bool connected = true;

	Locker SendLock;
	SystemMonitor SystemLoad;

public:
	RemoteClient(unsigned int socket, unsigned long ip, int level) : s(socket), ip(ip), level(level) {}
	bool SendData(const std::string& data);
	void CloseSocket();
};

class RemoteConsoleNetworking
{
	std::weak_ptr<work> hRemoteConsoleThread;

	unsigned int server;

protected:
	std::vector <std::unique_ptr<RemoteClient>> clients;

	virtual bool ProcessData(size_t ClientIndex, const char* Message, size_t Length) = 0;
	void CloseSocket(size_t ClientIndex);

	void RemoteConsoleThreadProc(std::shared_ptr<work> &hThread);
	static void StartThread(std::shared_ptr<work> &hThread);

public:
	void SendDataOne(const std::string& text, size_t ClientIndex);
	void SendDataAll(const std::string& text);
	bool StartRemoteConsole();
	bool StopRemoteConsole();
	std::wostringstream StreamOutRemoteClients();
	virtual ~RemoteConsoleNetworking() {}
};