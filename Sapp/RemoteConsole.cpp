#include "RemoteConsole.h"

#include "json\json.h"

#include "Addresses.h"
#include "Admins.h"
#include "CommandExecuter.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "LogThread.h"
#include "MiscellaneousFunctions.h"
#include "Prefs.h"
#include "RemoteConsoleNetworking.h"
#include "TextUtilities.h"

using namespace std;

enum RemoteConsoleOpcode { RC_LOGIN = 1, RC_QUERY, RC_QUERY_STATS, RC_QUERY_POS, RC_CIN, RC_COUT, RC_CHAT, RC_PJOIN, RC_PLEAVE, RC_TEAMCHANGE, RC_NEWGAME, RC_SYSTEM_STATUS };

static const char* const RCVersion = "1.2.0.0";

static Json::CharReaderBuilder readerbuilder;
static unique_ptr<Json::CharReader> reader(readerbuilder.newCharReader());

void RemoteConsole::SendRemoteOutput(const char* text)
{
	Json::Value root;
	root["opcode"] = RC_COUT;
	root["text"] = text;

	Json::FastWriter writer;
	SendDataOne(writer.write(root), client_executing);
}

void RemoteConsole::SendRemoteChat(size_t PlayerNumber, size_t Type, const wstring& message)
{
	Json::Value root;
	root["opcode"] = RC_CHAT;
	root["index"] = PlayerNumber;
	root["type"] = Type;
	root["message"] = WToStr(message);

	Json::FastWriter writer;
	SendDataAll(writer.write(root));
}

void RemoteConsole::Login(const std::string& username, const std::string& hash, size_t ClientIndex, int level)
{
	if (level < 0)
	{
		wstring wusername(username.begin(), username.end());

		HaloThreadLock.Lock();
		level = GlobalAdminList.IsAdmin(wusername, hash);
		HaloThreadLock.Unlock();

		if (level > -1)
		{
			clients[ClientIndex]->level = level;
			clients[ClientIndex]->name = wusername;
		}

		BYTE* RemoteIP = clients[ClientIndex]->IPasBytes;
		WriteLog(L"REMOTE", FormatWArgs(L"Login %ls, ClientIndex: %u  Username: %ls  level: %d  IP: %hhu.%hhu.%hhu.%hhu", level > -1 ? L"successful" : L"failed", ClientIndex, wusername.c_str(), level, RemoteIP[0], RemoteIP[1], RemoteIP[2], RemoteIP[3]));
	}

	Json::Value resp;
	resp["opcode"] = RC_LOGIN;
	resp["level"] = level;

	Json::FastWriter writer;
	SendDataOne(writer.write(resp), ClientIndex);
}

void RemoteConsole::ConsoleInput(const std::string& command, size_t ClientIndex, int level)
{
	int ret;
	{
		Lock_Object guard(HaloThreadLock);
		client_executing = ClientIndex;

		wstring wcommand(command.begin(), command.end());
		CommandEnviroment env(Enviroment::REMOTE, -1, level);
		ret = ExecuteSingleCommand(wcommand, env);

		WriteLog(L"REMOTE", FormatWArgs(L"Client #%u executed the following command: %ls, result: %d", ClientIndex, wcommand.c_str(), ret));
	}

	Json::Value resp;
	resp["opcode"] = RC_CIN;
	resp["ret"] = ret;

	Json::FastWriter writer;
	SendDataOne(writer.write(resp), ClientIndex);
}

static void GetPlayerDataJson(size_t PlayerIndex, Json::Value& player)
{
	player["index"] = IndexToNumber(PlayerIndex) + 1;
	player["name"] = WToStr(PlayerTable[PlayerIndex].name);
	player["team"] = (size_t)PlayerTable[PlayerIndex].Team;
	player["kills"] = PlayerTable[PlayerIndex].kills;
	player["assists"] = PlayerTable[PlayerIndex].assists;
	player["deaths"] = PlayerTable[PlayerIndex].deaths;
	player["betrays"] = (size_t)PlayerTable[PlayerIndex].tk_number;
	player["suicides"] = PlayerTable[PlayerIndex].suicides;
	player["score"] = GetPlayerScore(PlayerIndex);

	auto& color = player["color"];
	color["r"] = LastColor[PlayerIndex][0];
	color["g"] = LastColor[PlayerIndex][1];
	color["b"] = LastColor[PlayerIndex][2];
}

static void GetPlayerPosJson(size_t PlayerIndex, Json::Value& player)
{
	vect3f pos3f;
	GetPlayerPosition(PlayerIndex, pos3f);

	player["index"] = IndexToNumber(PlayerIndex) + 1;
	player["x"] = ToStr(pos3f.x);
	player["y"] = ToStr(pos3f.y);
	player["z"] = ToStr(pos3f.z);
}

void RemoteConsole::SendPlayerJoin(size_t PlayerIndex)
{
	Json::Value resp;
	resp["opcode"] = RC_PJOIN;
	GetPlayerDataJson(PlayerIndex, resp);

	Json::FastWriter writer;
	SendDataAll(writer.write(resp));
}

void RemoteConsole::SendPlayerLeave(size_t PlayerIndex)
{
	Json::Value resp;
	resp["opcode"] = RC_PLEAVE;
	resp["index"] = IndexToNumber(PlayerIndex) + 1;

	Json::FastWriter writer;
	SendDataAll(writer.write(resp));
}

void RemoteConsole::SendTeamChange(size_t PlayerIndex)
{
	Json::Value resp;
	resp["opcode"] = RC_TEAMCHANGE;
	resp["index"] = IndexToNumber(PlayerIndex) + 1;
	resp["team"] = (size_t)PlayerTable[PlayerIndex].Team;

	Json::FastWriter writer;
	SendDataAll(writer.write(resp));
}

void RemoteConsole::SendNewGame()
{
	Json::Value resp;
	resp["opcode"] = RC_NEWGAME;
	resp["map"] = CurrentMap;
	resp["mode"] = CurrentMode;
	resp["gametype"] = CurrentGT;
	resp["teams"] = *TeamGame;

	Json::FastWriter writer;
	SendDataAll(writer.write(resp));
}

void RemoteConsole::SendQuery(size_t ClientIndex)
{
	Json::Value resp;
	string SappVersion = GlobalPrefs->AppVersion;
	SappVersion += " ";
	SappVersion += GlobalPrefs->AppHaloType;
	resp["opcode"] = RC_QUERY;
	resp["version"] = RCVersion;
	resp["sapp_version"] = SappVersion;
	resp["no-lead"] = GlobalPrefs->NoLead;
	resp["anticheat"] = GlobalPrefs->AntiCheat;
	if (ServerRunning())
	{	
		resp["running"] = true;
		resp["servername"] = WToStr(Server->ServerName);
		resp["map"] = CurrentMap;
		resp["mode"] = CurrentMode;
		resp["gametype"] = CurrentGT;
		resp["maxplayers"] = (size_t)Server->MaxPlayers;
		bool teams = *TeamGame;
		resp["teams"] = teams;
		if (teams)
		{
			resp["red_score"] = GetTeamScore(0);
			resp["blue_score"] = GetTeamScore(1);
		}
		size_t numplayers = 0;
		for (size_t i = 0; i < 16; i++)
		{
			if (PlayerPresent(i))
			{
				auto& player = resp["players"][numplayers++];
				GetPlayerDataJson(i, player);
			}
		}
	}
	else
	{
		resp["running"] = false;
	}

	Json::FastWriter writer;
	SendDataOne(writer.write(resp), ClientIndex);
}

void RemoteConsole::SendQueryStats(size_t ClientIndex)
{
	Json::Value resp;
	resp["opcode"] = RC_QUERY_STATS;
	if (ServerRunning())
	{
		if (*TeamGame)
		{
			resp["red_score"] = GetTeamScore(0);
			resp["blue_score"] = GetTeamScore(1);
		}
		size_t numplayers = 0;
		for (size_t i = 0; i < 16; i++)
		{
			if (PlayerPresent(i))
			{
				auto& player = resp["players"][numplayers++];
				GetPlayerDataJson(i, player);
			}
		}
	}

	Json::FastWriter writer;
	SendDataOne(writer.write(resp), ClientIndex);
}

void RemoteConsole::SendQueryPos(size_t ClientIndex)
{
	Json::Value resp;
	resp["opcode"] = RC_QUERY_POS;
	if (ServerRunning())
	{
		size_t numplayers = 0;
		for (size_t i = 0; i < 16; i++)
		{
			if (PlayerAlive(i))
			{
				auto& player = resp["players"][numplayers++];
				GetPlayerPosJson(i, player);
			}
		}
	}

	Json::FastWriter writer;
	SendDataOne(writer.write(resp), ClientIndex);
}

void RemoteConsole::SendQuerySystemStatus(size_t ClientIndex)
{
	auto& monitor = clients[ClientIndex]->SystemLoad;

	Json::Value resp;
	resp["opcode"] = RC_SYSTEM_STATUS;
	resp["my_cpu_load"] = monitor.GetCurrentProcessCPULoad();
	resp["cpu_load"] = monitor.GetTotalCPULoad();
	resp["memory_load"] = monitor.GetMemoryLoad();

	Json::FastWriter writer;
	SendDataOne(writer.write(resp), ClientIndex);
}

bool RemoteConsole::ProcessJson(size_t ClientIndex, const char* Begin, const char* End)
{
	try
	{
		Json::Value root;
		string errs;
		reader->parse(Begin, End, &root, &errs);

		int level = clients[ClientIndex]->level;
		int opcode = root.get("opcode", 0).asInt();

		if (opcode == RC_LOGIN || level > -1)
		{
			switch (opcode)
			{
			case RC_LOGIN:
			{
				Login(root.get("username", "").asString(), root.get("password", "").asString(), ClientIndex, level);
				break;
			}
			case RC_QUERY:
			{
				Lock_Object guard(HaloThreadLock);
				SendQuery(ClientIndex);
				break;
			}
			case RC_QUERY_STATS:
			{
				Lock_Object guard(HaloThreadLock);
				SendQueryStats(ClientIndex);
				break;
			}
			case RC_QUERY_POS:
			{
				Lock_Object guard(HaloThreadLock);
				SendQueryPos(ClientIndex);
				break;
			}
			case RC_CIN:
			{
				ConsoleInput(root.get("command", "").asString(), ClientIndex, level);
				break;
			}
			case RC_SYSTEM_STATUS:
			{
				SendQuerySystemStatus(ClientIndex);
				break;
			}
			default:
			{
				return false;
			}
			}
		}
		else
		{
			return false;
		}
	}
	catch (exception&)
	{
		return false;
	}

	return true;
}

bool RemoteConsole::ProcessData(size_t ClientIndex, const char* Message, size_t Length)
{
	size_t begin = 0, end = 0;
	bool found = false;
	string& pending = clients[ClientIndex]->pending_json;
	
	while (end < Length)
	{
		if (Message[end] == '\n')
		{
			found = true;
			if (pending.empty())
			{
				if (!ProcessJson(ClientIndex, Message + begin, Message + end))
					return false;
			}
			else
			{
				pending.append(Message, Message + end);
				if (!ProcessJson(ClientIndex, pending.c_str(), pending.c_str() + pending.size()))
					return false;

				pending.clear();
			}
			begin = end;
		}
		end++;
	}

	if (!found || begin + 1 < Length)
	{
		pending.append(Message + begin, Message + end);
		if (pending.size() > 4095)
			return false;
	}

	return true;
}

RemoteConsole GlobalRemoteConsole;