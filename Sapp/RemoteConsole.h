#pragma once

#include <string>

#include "RemoteConsoleNetworking.h"

class RemoteConsole final : public RemoteConsoleNetworking
{
	size_t client_executing = 0;

	bool ProcessData(size_t ClientIndex, const char* Message, size_t Length);
	bool ProcessJson(size_t ClientIndex, const char* Begin, const char* End);
	void Login(const std::string& username, const std::string& hash, size_t ClientIndex, int level);
	void ConsoleInput(const std::string& command, size_t ClientIndex, int level);
	void SendQuery(size_t ClientIndex);
	void SendQueryStats(size_t ClientIndex);
	void SendQueryPos(size_t ClientIndex);
	void SendQuerySystemStatus(size_t ClientIndex);

public:
	void SendRemoteOutput(const char* text);
	void SendRemoteChat(size_t PlayerNumber, size_t Type, const std::wstring& message);
	void SendPlayerJoin(size_t PlayerIndex);
	void SendPlayerLeave(size_t PlayerIndex);
	void SendTeamChange(size_t PlayerIndex);
	void SendNewGame();
};

extern RemoteConsole GlobalRemoteConsole;