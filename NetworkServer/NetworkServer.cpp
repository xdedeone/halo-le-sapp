#include "NetworkServer.h"

#pragma warning(disable: 4146) // warning C4146: unary minus operator applied to unsigned type, result still unsigned

#define SystemFunction036 NTAPI SystemFunction036
#include <NTSecAPI.h>
#undef SystemFunction036

#include <fstream>

#include "CustomLoad.h"
#include "globals.h"
#include "TextUtilities.h"

using namespace std;
using namespace std::placeholders;

#ifndef DEBUG
const char* const Files[] = { "cortana.dll", "sapp_updater.dll", "sapp_ce.dll", "sapp_pc.dll", "strings_updater.dll", "strings.dll" };
#else
const char* const Files[] = { "debugcortana.dll", "sapp_updater.dll", "sapp_ce.dll", "sapp_pc.dll", "strings_updater.dll", "strings.dll" };
#endif;

boost::asio::async_result<boost::asio::handler_type<boost::asio::yield_context&, void()>::type>::type
SappServer::async_anything(const std::function <void()> &f)
{
	async_completion completion(std::forward<boost::asio::yield_context&>(Myield));

	unique_ptr<async_work> w = make_unique<async_work>(f, completion.handler);

	GlobalServer->ThreadPool.post(move(w));

	return completion.result.get();
}

void SappServer::WriteLog(const string &str)
{
	GlobalServer->AnticheatLog.Log(logname, str);
}

void SappServer::WriteGlobalLog(const string &str)
{
	//GlobalServer->AnticheatLog.Log("global.log", str);
}

void SappServer::Terminate(const char* reason)
{	
#ifndef DEBUG
	SendSmallEvent(event_terminate_client);
	IsAnticheat = false;
	SessionKey = 0;
#endif
	string log_str = FormatArgs("The client has been terminated. Reason: %hs", reason);
	WriteLog(log_str);
#ifndef DEBUG
	WriteGlobalLog(log_str);
#endif
}

void SappServer::Ban(const char* reason)
{
#ifndef DEBUG
	SendSmallEvent(event_terminate_client);
#endif
	string log_str = FormatArgs("The user has been banned. Reason: %hs", reason);
	WriteLog(log_str);
#ifndef DEBUG
	WriteGlobalLog(log_str);
	async_anything([&] { GlobalServer->MySQL.Query(FormatArgs("UPDATE ac_users SET play_allowed = 0, ban_count = ban_count + 1, ban_reason = '%hs', ban_time = CURRENT_TIMESTAMP WHERE userid = %u", reason, UserID)); });
	async_anything([&] { GlobalServer->MySQL.Query(FormatArgs("INSERT INTO ac_ipbans (ip, userid) VALUES((SELECT reg_ip FROM ac_users WHERE userid = %u), %u)", UserID, UserID)); });
	async_anything([&] { GlobalServer->MySQL.Query(FormatArgs("INSERT INTO ac_ipbans (ip, userid) VALUES((SELECT last_ip FROM ac_users WHERE userid = %u), %u)", UserID, UserID)); });
	for (size_t i = 0; i < MAC_Addresses.size(); i++)
	{
		BYTE* MAC = (BYTE*)&MAC_Addresses[i];
		async_anything([&] { GlobalServer->MySQL.Query(FormatArgs("INSERT INTO ac_macbans (mac, userid) VALUES('%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X', %u)", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5], MAC[6], MAC[7], UserID)); });
	}
	IsAnticheat = false;
	SessionKey = 0;
#endif
}

void SappServer::SendFile(WORD Flags)
{
	shared_lock<shared_mutex> lock(GlobalServer->FileCacheLock);

	size_t index = Flags + 1;
	size_t size = GlobalServer->FileSizes[index];

	PacketHeader Header;
	Header.Size = size;
	Header.Type = 1;
	Header.SubType = 2;
	Header.Flags = Flags;

	if (IsAnticheat)
	{
		char* buff = new char[size];
		headerCrypt.rc6_block_encrypt((WORD*)&Header, (WORD*)&Header);
		myCrypt.rc6_cbc_encrypt((ULONGLONG*)GlobalServer->FileBuffs[index].get(), (ULONGLONG*)buff, size);
		send(&Header, 8);
		send(buff, size);
	}
	else
	{
		send(&Header, 8);
		send(GlobalServer->FileBuffs[index].get(), size);
	}
}

void SappServer::SendMap(WORD Index)
{
	auto it = GlobalServer->MapTable.find(Index);
	if (it == GlobalServer->MapTable.end())
		return;

	size_t file_lenght = 0;
	unique_ptr<char[]> buff;
	async_anything([&]
	{
		string name = FormatArgs("C:\\Halo CE\\map_7z\\%hs.7z", it->second.c_str());
	
		HANDLE hFile = CreateFile(name.c_str(), GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			file_lenght = GetFileSize(hFile, NULL);
			CloseHandle(hFile);
		}

		if (file_lenght)
		{
			FILE* File;
			if (!fopen_s(&File, name.c_str(), "rb") && File)
			{
				buff = make_unique<char[]>(file_lenght);
				fread(buff.get(), file_lenght, 1, File);
				fclose(File);
			}
		}
	});

	if (buff)
	{
		PacketHeader Header;		
		Header.Type = 1;
		Header.SubType = 2;
		Header.Flags = 0xFFFF;
		Header.Size = file_lenght;
		send(&Header, 8);
		send(buff.get(), Header.Size);
	}
	else
	{
		PacketHeader Header;
		Header.Size = 0;
		Header.Type = 1;
		Header.SubType = 2;
		Header.Flags = 0xFFFF;
		send(&Header, 8);
	}
}

//bool SappServer::ProcessEventStart(char* serverkey)
//{
//	MYSQL_RES* result = GlobalServer->MySQL.QueryResult(FormatArgs("SELECT serverid FROM serverkeys WHERE serverid = %s", serverkey).c_str());
//	if(result == NULL)
//		return true;
//	else
//		return false;
//}
//
//void SappServer::ProcessEventGameStart(CurrentGame* cg)
//{
//	memcpy(&Game, cg, sizeof(CurrentGame));
//}
//
//void SappServer::ProcessEventGameEnd()
//{
//	for(deque <HR_Event*>::iterator it = Events.begin(); it != Events.end(); ++it)
//	{
//		if((*it)->Type != HR_EVENT_GAME_START)
//		{
//			delete *it;
//			Events.pop_front();
//		}
//		else
//			break;
//	}
//	if(Events.size())
//	{
//		map <DWORD, PlayerStats> Players;
//		for(deque <HR_Event*>::iterator it = Events.begin(); it != Events.end(); ++it)
//		{
//			DWORD Player = (*it)->IP;
//			switch((*it)->Type)
//			{
//			case HR_EVENT_CDKEY_VALIDATION:
//			{
//				Players[Player].ValidKey = ((HREventCDKeyValidation*)(*it))->Value;
//				break;
//			}
//			case HR_EVENT_PLAYER_JOIN:
//			{
//				memcpy(Players[Player].Name, ((HREventJoin*)(*it))->Name, 24);
//				memcpy(Players[Player].IP, ((HREventJoin*)(*it))->IPstr, 32);
//				memcpy(Players[Player].Hash, ((HREventJoin*)(*it))->Hash, 32);
//				Players[Player].Team = ((HREventJoin*)(*it))->Team;
//				Players[Player].Left = 0;
//				break;
//			}
//			case HR_EVENT_PLAYER_LEAVE:
//			{
//				Players[Player].Left = 1;
//				break;
//			}
//			case HR_EVENT_TEAM_SWITCH:
//			{
//				Players[Player].Team = ((HREventTeamSwitch*)(*it))->NewTeam;
//				break;
//			}
//			case HR_EVENT_KILL:
//			{
//				Players[Player].Deaths++;
//				DWORD Killer = ((HREventKill*)(*it))->Killer;
//				if(Killer)
//				{
//					if(Player == Killer)
//					{
//						Players[Killer].Suicides++;
//					}
//					else if(Players[Player].Team == Players[Killer].Team)
//					{
//						 Players[Killer].Betrays++;
//					}
//					else
//					{
//						Players[Killer].Kills++;
//						Players[Killer].Experince += 4;
//					
//						if(((HREventKill*)(*it))->Multi > 1)
//						{
//							Players[Killer].Experince += 2;
//						}
//						if((((HREventKill*)(*it))->Spree % 5) == 0)
//						{
//							Players[Killer].Experince += 5;
//						}
//					}
//				}
//				break;
//			}
//			case HR_EVENT_ASSIST:
//			{
//				Players[Player].Assists++;
//				Players[Player].Experince += 6;
//				break;
//			}
//			}
//		}
//		if(!Players.empty())
//		{
//			ULONGLONG id = GlobalServer->MySQL.QueryID(FormatArgs("INSERT INTO gamestable (ip, servername, map, gametype, mode, teamgame) VALUES ('%s:%d', '%s', '%s', '%s', '%s', '%d'", inet_ntoa(*(in_addr*)&Stats.IP), Stats.Port, WToStr(Stats.ServerName).c_str(), Game.MapName, Game.GTName, Game.GameType, Game.TeamGame).c_str());
//			for(map <DWORD, PlayerStats>::iterator it = Players.begin(); it != Players.end(); ++it)
//			{
//				GlobalServer->MySQL.Query(FormatArgs("INSERT INTO playerstable (gameid, name, ip, hash, valid, exp, kills, assists, deaths, suicides, betrays, left) VALUES ('%llu', '%s', '%s', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d'", id, WToStr(it->second.Name).c_str(), it->second.IP, it->second.Hash, it->second.ValidKey, it->second.Experince, it->second.Kills, it->second.Assists, it->second.Deaths, it->second.Suicides, it->second.Betrays, it->second.Left).c_str());
//				if(it->second.ValidKey == 1)
//				{
//					MYSQL_RES* res = GlobalServer->MySQL.QueryResult(FormatArgs("SELECT username FROM userstable WHERE hash = %s", it->second.Hash).c_str());
//					if(res != NULL)
//					{
//						GlobalServer->MySQL.Query(FormatArgs("UPDATE userstable SET exp = exp + %d, kills = kills + %d, assists = assists + %d, deaths = deaths + %d, suicides = suicides + %d, betrays = betrays + %d WHERE hash = '%s'", id, it->second.Experince, it->second.Kills, it->second.Assists, it->second.Deaths, it->second.Suicides, it->second.Betrays, it->second.Hash).c_str());
//						mysql_free_result(res);
//					}
//				}
//			}
//		}
//	}
//}
//
//bool SappServer::ProcessEvent(HR_Event* Event)
//{
//	HR_Event* NewEvent;
//	bool ret = false;
//	switch(Event->Type)
//	{
//	case HR_EVENT_START:
//		NewEvent = new HREventStart(*(HREventStart*)Event);
//		ret = ProcessEventStart(((HREventStart*)Event)->ServerKey);
//		break;
//	case HR_EVENT_END:
//		NewEvent = new HREventEnd(*(HREventEnd*)Event);
//		ProcessEventGameEnd();
//		break;
//	case HR_EVENT_GAME_START:
//		NewEvent = new HREventGameStart(*(HREventGameStart*)Event);
//		ProcessEventGameStart((CurrentGame*)((HREventGameStart*)Event)->MapName);
//		break;
//	case HR_EVENT_GAME_END:
//		NewEvent = new HREventGameEnd(*(HREventGameEnd*)Event);
//		ProcessEventGameEnd();
//		break;
//	case HR_EVENT_CDKEY_VALIDATION:
//		NewEvent = new HREventCDKeyValidation(*(HREventCDKeyValidation*)Event);
//		break;
//	case HR_EVENT_PLAYER_JOIN:
//		NewEvent = new HREventJoin(*(HREventJoin*)Event);
//		break;
//	case HR_EVENT_PLAYER_LEAVE:
//		NewEvent = new HREventLeave(*(HREventLeave*)Event);
//		break;
//	case HR_EVENT_TEAM_SWITCH:
//		NewEvent = new HREventTeamSwitch(*(HREventTeamSwitch*)Event);
//		break;
//	case HR_EVENT_KILL:
//		NewEvent = new HREventKill(*(HREventKill*)Event);
//		break;
//	case HR_EVENT_ASSIST:
//		NewEvent = new HREventAssist(*(HREventAssist*)Event);
//		break;
//	default:
//		return true;
//	}
//
//	Events.emplace_back(NewEvent);
//
//	return ret;
//}

bool SappServer::ProcessPacket_RemoteConsole(const PacketHeader& Header, const char* Buffer)
{
	bool ret = false;
	switch (Header.SubType)
	{
	case 0: // Ping
	{
		char buff[8] = {};
		send(buff, 8);
		break;
	}
	case 1: // Client Number
	{
		PacketHeader nHeader = {};
		nHeader.SubType = 1;
		nHeader.Flags = (WORD)GlobalServer->ClientNumber;
		send(&nHeader, 8);
		break;
	}
	case 2: // Reboot
	{
		if(Buffer != NULL && !strncmp(Buffer, "R3b00t", 6))
		{
			PacketHeader nHeader = {};
			nHeader.SubType = 2;
			send(&nHeader, 8);
			system("start Reboot.exe");
		}
		break;
	}
	case 3:
	{
		GlobalServer->ListServersToFile();
		PacketHeader nHeader = {};
		nHeader.SubType = 3;
		send(&nHeader, 8);
		break;
	}
	default:
		ret = true;
	}
	return ret;
}

bool SappServer::ProcessPacket_SappServer(const PacketHeader& Header, const char* Buffer)
{
	bool ret = false;
	switch (Header.SubType)
	{
	case 0: // Get IP
	{
		IsServer = true;
		ServerIP IPPacket;
		IPPacket.Size = 4;
		IPPacket.Type = 1;
		IPPacket.SubType = 0;
		IPPacket.IP = MyIP;
		send(&IPPacket, sizeof(ServerIP));
		break;
	}
	case 1: // SAPP Version
	{
		PacketVersion VersionPacket;
		VersionPacket.Size = 4;
		VersionPacket.Type = 1;
		VersionPacket.SubType = 1;
		VersionPacket.Version = SAPPVersion;
		send(&VersionPacket, sizeof(PacketVersion));	
		break;
	}
	case 2: // Send Files
	{
		if (Header.Flags == 0xFFFF)
		{
			if (Buffer != NULL)
				SendMap(*(WORD*)Buffer);
			else
				ret = true;
		}
		else if(Header.Flags <= 3)
		{
			GlobalServer->SappUpdateCountLock.Lock();
			size_t count = ++GlobalServer->SappUpdateCount[MyIP];
			GlobalServer->SappUpdateCountLock.Unlock();
			if (count > 256)
				break;

			SendFile(Header.Flags);
		}
		else
		{
			ret = true;
		}
		break;
	}
	case 3: // Get Stats
	{
		if (Header.Size <= sizeof(ServerStats) && Header.Size >= sizeof(ServerStats) - 32 && Buffer != NULL)
		{
			memcpy(&Stats, Buffer, Header.Size);
			LastUpdated = time(0);
			vector<string> v;
			v.emplace_back(ToStr(Stats.Players));
			v.emplace_back(ToStr(Stats.MaxPlayers));
			v.emplace_back(ToStr(Stats.NoLead));
			v.emplace_back(ToStr(Stats.HAC));
			v.emplace_back(ToStr(Stats.HaloVerType));
			v.emplace_back(ToStr(Stats.Password));
			v.emplace_back(WToStr(Stats.ServerName));
			v.emplace_back(WToStr(Stats.Map));
			v.emplace_back(WToStr(Stats.Mode));
			v.emplace_back(Stats.VersionString);
			v.emplace_back(FormatArgs("%hhu.%hhu.%hhu.%hhu", Stats.V4, Stats.V3, Stats.V2, Stats.V1));
			v.emplace_back(FormatArgs("%hs:%hu", ipstr.c_str(), Stats.Port));
			if (Header.Size == sizeof(ServerStats) && Stats.GameType[0] != 0)
			{
				v.emplace_back(Stats.GameType);
			}
			else
			{
				v.emplace_back("n/a");
			}
			v.emplace_back(ToStr((int)this));
			async_anything([&] { GlobalServer->MySQL.PreparedStatement("UPDATE sapp_servers SET players = ?, maxplayers = ?, nolead = ?, anticheat = ?, halotype = ?, locked = ?, name = ?, map = ?, mode = ?, version = ?, sappversion = ?, ip = ?, gametype = ? WHERE serverid = ?", v); });
		}
		else
		{
			ret = true;
		}
		break;
	}
	case 4: // Map Download Request
	{
		vector <WORD> FoundIDs;

		if(Header.Size > 2 && Buffer != NULL)
		{
			string tmpbuff;
			tmpbuff.resize(Header.Size);
			for (unsigned int i = 0; i < Header.Size; i++)
			{
				if (Buffer[i] >= 'A' && Buffer[i] <= 'Z')
					tmpbuff[i] = Buffer[i] + 32;
				else
					tmpbuff[i] = Buffer[i];
			}

			for(auto it = GlobalServer->MapTable.begin(); it != GlobalServer->MapTable.end(); ++it)
			{
				if (it->second.find(tmpbuff) != string::npos)
				{
					FoundIDs.emplace_back(it->first);
				}
			}
		}

		PacketHeader newHeader = {};
		newHeader.Size = FoundIDs.size() * sizeof(MapDownloadEntry);
		newHeader.Type = 1;
		newHeader.SubType = 4;
		newHeader.Flags = 0;

		send(&newHeader, 8);
		for(size_t i = 0; i < FoundIDs.size(); i++)
		{		
			WORD ID = FoundIDs[i];
			const string& Map = GlobalServer->MapTable[ID];
			MapDownloadEntry newEntry = {};
			newEntry.ID = ID;
			memcpy(newEntry.Name, Map.c_str(), Map.size());
			send(&newEntry, sizeof(MapDownloadEntry));
		}

		break;
	}
	case 5: // ACLogin
	{
		if (Header.Size >= 4 && ((*(DWORD*)Buffer) ^ 0x736568E9) == SAPPVersion)
		{
			IsACServer = true;
			ACLogin IPPacket;
			IPPacket.Size = 4;
			IPPacket.Type = 1;
			IPPacket.SubType = 5;
			IPPacket.ServerID = (DWORD)this;
			send(&IPPacket, sizeof(ACLogin));
		}
		else
		{
			ACLogin IPPacket;
			IPPacket.Size = 4;
			IPPacket.Type = 1;
			IPPacket.SubType = 5;
			IPPacket.ServerID = 0;
			send(&IPPacket, sizeof(ACLogin));
		}
		break;
	}
	default:
		ret = true;
	}
	return ret;
}
bool SappServer::ProcessPacket_HAC(const PacketHeader& Header, const char* Buffer)
{
	bool ret = false;
	switch (Header.SubType)
	{
	case event_ping: // Ping
		{
			LastUpdated = time(0);
			SendSmallEvent(event_ping);
			break;
		}
	case event_get_rc6: // Send RC6 Key and IV
		{
			BYTE buff[64];
			RtlGenRandom(buff, 64);

			headerCrypt.rc6_key_setup(buff, buff + 32);
			myCrypt.rc6_key_setup(buff, buff + 32);

			SendBigEvent(event_get_rc6, buff, 64);

			IsAnticheat = true;
			break;
		}
	case event_login: // Send LoginSessionKey
		{
			if (Header.Size >= sizeof(loginfoStr))
			{
				loginfoStr* pBuff = (loginfoStr*)Buffer;
				PacketHeader newHeader = {};
				newHeader.Type = 2;
				newHeader.SubType = 2;
				newHeader.Flags = 1;

				pBuff->username[31] = 0;

				logname = FormatArgs("logs\\%hs.log", pBuff->username);
				
				WriteLog(FormatArgs("Login attempt, IP: %hs.", ipstr.c_str()));
				
				bool ok = false;
				vector<string> res;
				vector<string> v;
				v.emplace_back(pBuff->username);
				v.emplace_back(ConvertToString(pBuff->password));
				async_anything([&] { GlobalServer->MySQL.QueryResultPreparedStatement("SELECT userid, activated, play_allowed, ban_count FROM ac_users WHERE username = ? AND hash = ?", v, res, 4); });
				if (res.size() == 4)
				{
					size_t tmpUserID = strtoul(res[0].c_str(), NULL, 10);
					size_t activated = strtoul(res[1].c_str(), NULL, 10);
					size_t play_allowed = strtoul(res[2].c_str(), NULL, 10);
					size_t ban_count = strtoul(res[3].c_str(), NULL, 10);

				#ifndef DEBUG
					shared_lock<shared_mutex> lock(GlobalServer->ClientVectorLock);
					for (auto it = GlobalServer->Clients.begin(); it != GlobalServer->Clients.end(); ++it)
					{
						if (auto sp = it->lock())
						{
							if (sp->UserID == tmpUserID)
							{
								sp->CloseConnection();
								break;
							}
						}
					}
					lock.unlock();
				#endif

					UserID = tmpUserID;

					if (activated && !play_allowed)
					{
						res.clear();
						async_anything([&] { GlobalServer->MySQL.QueryResult(FormatArgs("SELECT userid FROM ac_users WHERE userid = %u AND TIMESTAMPDIFF(DAY, (SELECT ban_time FROM ac_users WHERE userid = %u), CURRENT_TIMESTAMP) >= %u", UserID, UserID, ban_count), res, 1); });
						if (res.size())
						{
							async_anything([&] { GlobalServer->MySQL.Query(FormatArgs("UPDATE ac_users SET play_allowed = 1 WHERE userid = %u", UserID)); });
							async_anything([&] { GlobalServer->MySQL.Query(FormatArgs("DELETE FROM ac_ipbans WHERE userid =  %u", UserID)); });
							async_anything([&] { GlobalServer->MySQL.Query(FormatArgs("DELETE FROM ac_macbans WHERE userid = %u", UserID)); });
							play_allowed = 1;
						}
					}
					if (play_allowed)
					{
						res.clear();
						async_anything([&] { GlobalServer->MySQL.Query(FormatArgs("UPDATE ac_users SET last_ip = '%hs' WHERE userid = %u", ipstr.c_str(), UserID)); });
						async_anything([&] { GlobalServer->MySQL.QueryResult(FormatArgs("SELECT userid FROM ac_ipbans WHERE ip = '%hs'", ipstr.c_str()), res, 1); });
						if (res.size())
						{
							Ban("User is on the IP banlist.");
							newHeader.Flags = 3;
						}
						else
						{
							ssname = FormatArgs("screenshots\\%hs", pBuff->username);	
							async_anything([&]
							{
								CreateDirectoryA(ssname.c_str(), NULL);
							});
							username = pBuff->username;
							alias = pBuff->username;
							WriteLog("Login successfull.");
							newHeader.Size = 4;
							newHeader.Flags = 0;
							ok = true;
						}	
					}
					else if (!activated)
					{
						WriteLog("Login failed, reason: not activated.");
						newHeader.Flags = 2;
					}
					else
					{
						WriteLog("Login failed, reason: banned.");
						newHeader.Flags = 3;
					}
				}
				else
				{
					WriteLog("Login failed, reason: invalid password (or username).");
				}
					
				headerCrypt.rc6_block_encrypt((WORD*)&newHeader, (WORD*)&newHeader);				
				send(&newHeader, 8);
				if (ok)
				{
					RtlGenRandom(&SessionKey, sizeof(DWORD));
					size_t count = 16;
					bool mysqlret = false;
					async_anything([&] { mysqlret = GlobalServer->MySQL.Query(FormatArgs("UPDATE ac_users SET session_key = %d WHERE userid = %u", SessionKey, UserID)); });
					while (!mysqlret && count)
					{
						count--;
						SessionKey++;
						async_anything([&] { mysqlret = GlobalServer->MySQL.Query(FormatArgs("UPDATE ac_users SET session_key = %d WHERE userid = %u", SessionKey, UserID)); });
					}
					send(&SessionKey, sizeof(DWORD));
				}
			}
			else
			{
				ret = true;
			}
			break;
		}
	case event_setup_cortana: // Send Cortana
		{
			if (SessionKey)
			{
				HaloType = (BYTE)Header.Flags;

				shared_lock<shared_mutex> lock(GlobalServer->FileCacheLock);

				size_t buff_size = GlobalServer->FileSizes[0];
				const BYTE* buff = GlobalServer->FileBuffs[0].get();
				
				PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)buff;
				PIMAGE_NT_HEADERS old_header = (PIMAGE_NT_HEADERS)&buff[dos_header->e_lfanew];
				size_t image_size = old_header->OptionalHeader.SizeOfImage;

				SendBigEvent(event_setup_cortana, &image_size, 4);

				unique_ptr<BYTE[]> packetBuff = make_unique<BYTE[]> (buff_size);
				memcpy(packetBuff.get(), buff, buff_size);

				lock.unlock();

				unique_ptr<BYTE[]> dllBuff = make_unique<BYTE[]>(image_size);

				dos_header = (PIMAGE_DOS_HEADER)packetBuff.get();
				old_header = (PIMAGE_NT_HEADERS)&packetBuff.get()[dos_header->e_lfanew];

				CopySections(packetBuff.get(), dllBuff.get(), old_header);

				size_t base = 0;
				boost::system::error_code ec;
				boost::asio::async_read(socket, boost::asio::buffer(&base, 4), Myield[ec]);
				if (!ec)
				{
					PerformBaseRelocation(dllBuff.get(), old_header, base);

					XORFuncs(dllBuff.get(), old_header, SessionKey);

					UnCopySections(packetBuff.get(), dllBuff.get(), old_header);

					old_header->OptionalHeader.ImageBase = base;

					SendBigEvent(event_load_cortana, packetBuff.get(), buff_size);

					IsAnticheat = false;

					WriteLog("Sent cortana.");
				}
				else
				{
					WriteLog("Failed to receive the base of cortana.");
					ret = true;
				}
			}
			else
			{
				ret = true;
			}	
			break;
		}
	case event_load_cortana:
	{
		WriteLog("Cortana loaded.");
		break;
	}
	case event_mac_addresses:
		{
			if (Buffer != NULL)
			{
				for (size_t i = 0; i < Header.Size / 8; i++)
				{	
					if (((ULONGLONG*)Buffer)[i] == 0 || ((ULONGLONG*)Buffer)[i] == 0x797A)
						continue;

					BYTE* MAC = (BYTE*)&((ULONGLONG*)Buffer)[i];
					if (MAC[0] == 0x00 && MAC[1] == 0x50 && MAC[2] == 0x56 && MAC[3] == 0xC0 && MAC[4] == 0x00 && (MAC[5] == 0x01 || MAC[5] == 0x08))
						continue;

					WriteLog(FormatArgs("MAC address received: %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5], MAC[6], MAC[7]));
					vector <string> res;
					async_anything([&] { GlobalServer->MySQL.QueryResult(FormatArgs("SELECT userid FROM ac_macbans WHERE mac = '%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X'", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5], MAC[6], MAC[7]), res, 1); });
					if (res.size())
					{
						Ban("User is on the MAC banlist.");
					#ifndef DEBUG
						ret = true;
					#endif
						break;
					}
					MAC_Addresses.emplace_back(((ULONGLONG*)Buffer)[i]);
				}
			}
			else
			{
				WriteLog("No MAC address received.");
			}

			if (!GotHaloChecksum || !GotStringsChecksum)
			{
				Terminate("Not all checksums received.");
			#ifndef DEBUG
				ret = true;
			#endif
			}
			break;
		}
	case event_terminate_client:
	{
		WriteLog("Client terminated.");
	#ifndef DEBUG
		ret = true;
	#endif
		break;
	}
	case event_debugger_attached:
	{
		Ban("Debugger Attached.");	
	#ifndef DEBUG
		ret = true;
	#endif
		break;
	}
	case event_timing_attack:
	{
		Terminate("Timing attack.");
	#ifndef DEBUG
		ret = true;
	#endif
		break;
	}
	case event_anti_debugger_detect:
	{
		Ban("Anti Debugger-Detection found.");
	#ifndef DEBUG
		ret = true;
	#endif
		break;
	}
	case event_sem_wait_timeout:
	{
		Terminate("Wait timeout.");
	#ifndef DEBUG
		ret = true;
	#endif
		break;
	}
	case event_debug_object:
	{
		Ban("Debug object found.");
	#ifndef DEBUG
		ret = true;
	#endif
		break;
	}
	case event_no_debug_inherit:
	{
		Ban("No debug inherit found.");
	#ifndef DEBUG
		ret = true;
	#endif
		break;
	}
	case event_rasterise_change:
	{
		Ban("Rasterising modified.");
	#ifndef DEBUG
		ret = true;
	#endif
		break;
	}
	case event_illegal_dx_hook:
	{
		Ban(FormatArgs("Illegal DirectX hook detected (%hu).", Header.Flags).c_str());
	#ifndef DEBUG
		ret = true;
	#endif
		break;
	}
	case event_illegal_dxcreate_hook:
	{
		Ban("Illegal DirectXCreate9 hook detected.");
	#ifndef DEBUG
		ret = true;
	#endif
		break;
	}
	case event_wrong_checksum:
	{
		if (!UpdateOngoing)
		{
			Ban("Halo code modified.");
		#ifndef DEBUG
			ret = true;
		#endif
		}
		break;
	}
//	case event_wrong_tags_hash:
//	{
//		Terminate("Wrong tags table hash.");
//#ifndef DEBUG
//		ret = true;
//#endif
//		break;
//	}
//	case event_update_failed:
//	{
//		UpdateOngoing = false;
//		Terminate("Strings update failed.");
//	#ifndef DEBUG
//		ret = true;
//	#endif
//		break;
//	}
//	case event_update_succeed:
//	{
//		UpdateOngoing = false;
//		WriteLog("Strings update succeed.");
//		break;
//	}
	case event_halo_checksum:
		{
			if (Buffer != NULL)
			{
				string hash_str = FormatArgs("%08X%08X%08X%08X%08X%08X%08X%08X", endian_swap(((DWORD*)Buffer)[0]), endian_swap(((DWORD*)Buffer)[1]), endian_swap(((DWORD*)Buffer)[2]), endian_swap(((DWORD*)Buffer)[3]), endian_swap(((DWORD*)Buffer)[4]), endian_swap(((DWORD*)Buffer)[5]), endian_swap(((DWORD*)Buffer)[6]), endian_swap(((DWORD*)Buffer)[7]));				
				if (HaloType == 1)
				{
					WriteLog(FormatArgs("Halo PC Hash: %hs", hash_str.c_str()));
					map <string, string>::iterator it = GlobalServer->HaloPCHashes.find("halo");
					if (it != GlobalServer->HaloPCHashes.end())
					{
						if (it->second != hash_str)
						{
							Terminate("Wrong Halo PC Checksum.");
						#ifndef DEBUG
							ret = true;
						#endif
						}
					}
					GotHaloChecksum = true;
				}
				else if (HaloType == 2)
				{
					WriteLog(FormatArgs("Halo CE Hash: %hs", hash_str.c_str()));
					map <string, string>::iterator it = GlobalServer->HaloCEHashes.find("halo");
					if (it != GlobalServer->HaloCEHashes.end())
					{
						if (it->second != hash_str)
						{
							Terminate("Wrong Halo CE Checksum.");
					#ifndef DEBUG
							ret = true;
					#endif
						}
					}
					GotHaloChecksum = true;
				}
				else
				{
					ret = true;
				}
			}
			else
			{
				ret = true;
			}
			break;
		}
	case event_strings_checksum:
		{
			if (Buffer != NULL)
			{
				string hash_str = FormatArgs("%08X%08X%08X%08X%08X%08X%08X%08X", endian_swap(((DWORD*)Buffer)[0]), endian_swap(((DWORD*)Buffer)[1]), endian_swap(((DWORD*)Buffer)[2]), endian_swap(((DWORD*)Buffer)[3]), endian_swap(((DWORD*)Buffer)[4]), endian_swap(((DWORD*)Buffer)[5]), endian_swap(((DWORD*)Buffer)[6]), endian_swap(((DWORD*)Buffer)[7]));
				WriteLog(FormatArgs("Strings's Hash: %hs", hash_str.c_str()));
				if (HaloType == 1)
				{
					map <string, string>::iterator it = GlobalServer->HaloPCHashes.find("strings");
					if (it != GlobalServer->HaloPCHashes.end())
					{
						if (it->second != hash_str)
						{
							//UpdateOngoing = true;
							//SendFile(3);
							//SendFile(4);
							//Terminate("Wrong Halo PC Strings Checksum.");
					#ifndef DEBUG
							//ret = true;
					#endif
						}
					}
					GotStringsChecksum = true;
				}
				else if (HaloType == 2)
				{
					map <string, string>::iterator it = GlobalServer->HaloCEHashes.find("strings");
					if (it != GlobalServer->HaloCEHashes.end())
					{
						if (it->second != hash_str)
						{
							//UpdateOngoing = true;
							//SendFile(3);
							//SendFile(4);
							//Terminate("Wrong Halo CE Strings Checksum.");
						#ifndef DEBUG
							//ret = true;
						#endif
						}
					}
					GotStringsChecksum = true;
				}
				else
				{
					ret = true;
				}
			}
			else
			{
				ret = true;
			}
			break;
		}
	case event_loader_checksum:
	{
		ret = true;
		break;
	}
	case event_map_checksum:
		{
			if (Buffer != NULL)
			{
				const_cast<char*>(Buffer)[31] = 0;
				string hash_str = FormatArgs("%08X%08X%08X%08X%08X%08X%08X%08X", endian_swap(((DWORD*)Buffer)[8]), endian_swap(((DWORD*)Buffer)[9]), endian_swap(((DWORD*)Buffer)[10]), endian_swap(((DWORD*)Buffer)[11]), endian_swap(((DWORD*)Buffer)[12]), endian_swap(((DWORD*)Buffer)[13]), endian_swap(((DWORD*)Buffer)[14]), endian_swap(((DWORD*)Buffer)[15]));
				WriteLog(FormatArgs("Map: %hs Hash: %hs", Buffer, hash_str.c_str()));
				if (HaloType == 1)
				{
					map <string, string>::iterator it = GlobalServer->HaloPCHashes.find(Buffer);
					if (it != GlobalServer->HaloPCHashes.end())
					{
						if (it->second != hash_str)
						{
							if (auto sp = MyServer.lock())
							{
								if (sp->IsACServer)
								{
									PacketHeader Head;
									Head.Size = 4;
									Head.Type = 2;
									Head.SubType = event_map_checksum;
									Head.Flags = 0;
									sp->send(&Head, 8, Myield);
									sp->send((char*)&SessionKey, 4, Myield);
								}
							}
							Terminate("Wrong Halo PC Map Checksum.");
						}
					}
				}
				else if (HaloType == 2)
				{
					map <string, string>::iterator it = GlobalServer->HaloCEHashes.find(Buffer);
					if (it != GlobalServer->HaloCEHashes.end())
					{
						if (it->second != hash_str)
						{
							if (auto sp = MyServer.lock())
							{
								if (sp->IsACServer)
								{
									PacketHeader Head;
									Head.Size = 4;
									Head.Type = 2;
									Head.SubType = event_map_checksum;
									Head.Flags = 0;
									sp->send(&Head, 8, Myield);
									sp->send((char*)&SessionKey, 4, Myield);
								}
							}
							Terminate("Wrong Halo CE Map Checksum.");
						}
					}
				}
				else
				{
					ret = true;
				}
			}
			else
			{
				ret = true;
			}
			break;
		}
	case event_join_client:
		{
			if (Header.Size >= sizeof(ACJoinInfo) && IsAnticheat)
			{
				ACJoinInfo* info = (ACJoinInfo*)Buffer;
				//bool found = false;
				if (info->ServerID != 0)
				{
					shared_lock<shared_mutex> lock(GlobalServer->ClientVectorLock);
					for (auto it = GlobalServer->Clients.begin(); it != GlobalServer->Clients.end(); ++it)
					{
						if (auto sp = it->lock())
						{
							if (&(*sp) == info->ServerID)
							{
								MyServer = sp;
								//if (MyServer->IsACServer)
								//{
								//	found = true;
								//	break;
								//}
								break;
							}
						}
					}
				}

				string log_str = FormatArgs("%hs (alias %hs) joined to server %hhu.%hhu.%hhu.%hhu:%hu with name %ls", username.c_str(), alias.c_str(), (info->IP >> 24) & 0xFF, (info->IP >> 16) & 0xFF, (info->IP >> 8) & 0xFF, info->IP & 0xFF, info->Port, info->Name);

				WriteLog(log_str);
				WriteGlobalLog(log_str);
				
				//if (found)
				//{
				//	wchar_t buff[48] = L"~ Cortana ~ This server is using anticheat!";
				//	SendBigEvent(event_join_client, buff, 96);
				//}
				//else
				//{			
				//	wchar_t buff[48] = L"~ Cortana ~ This server is NOT using anticheat!";
				//	SendBigEvent(event_join_client, buff, 96);
				//}
			}
			else
			{
				ret = true;
			}
			break;
		}
	case event_join_server:
	{
		if (Header.Size >= sizeof(SappAnticheatPlayer))
		{	
			SappAnticheatPlayer* ACPlayer = (SappAnticheatPlayer*)Buffer;
			shared_ptr<SappServer> sp;
			shared_lock<shared_mutex> lock(GlobalServer->ClientVectorLock);
			for (auto it = GlobalServer->Clients.begin(); it != GlobalServer->Clients.end(); ++it)
			{
				if (sp = it->lock())
				{
					if (sp->IsAnticheat && sp->SessionKey == ACPlayer->SessionKey)
					{
						sp->PlayerIndex = ACPlayer->PlayerIndex;
						break;
					}
				}
			}
			lock.unlock();

			if (sp != nullptr)
			{
				sp->MyServer = shared_from_this();

				if (IsACServer)
				{
					SendBigEvent(event_join_server, &ACPlayer->SessionKey, 4);
				}				

				PacketHeader Head;
				Head.Size = 0;
				Head.Type = 2;
				Head.SubType = event_map_checksum;
				Head.Flags = IsACServer ? 2501 : 0;			
				sp->headerCrypt.rc6_block_encrypt((WORD*)&Head, (WORD*)&Head);
				sp->send(&Head, 8, Myield);

				Head.Size = sizeof(AnticheatPlayerPacket);
				Head.Type = 2;
				Head.SubType = event_join_server;		
				Head.Flags = 0;
				for (size_t i = 0; i < AnticheatClients.size(); i++)
				{
					if (auto sp_ac = AnticheatClients[i].lock())
					{
						AnticheatPlayerPacket Packet = { sp_ac->PlayerIndex };					
						memcpy(Packet.AnticheatName, sp_ac->alias.c_str(), sp_ac->alias.size());
						PacketHeader tmpHeader;
						AnticheatPlayerPacket tmpPacket = {};
						sp->headerCrypt.rc6_block_encrypt((WORD*)&Head, (WORD*)&tmpHeader);
						sp->myCrypt.rc6_cbc_encrypt((ULONGLONG*)&Packet, (ULONGLONG*)&tmpPacket, sizeof(AnticheatPlayerPacket));
						sp->send(&tmpHeader, 8, Myield);
						sp->send(&tmpPacket, sizeof(AnticheatPlayerPacket), Myield);
					}
				}

				AnticheatClients.emplace_back(sp);

				for (size_t i = 0; i < AnticheatClients.size(); i++)
				{
					if (auto sp_ac = AnticheatClients[i].lock())
					{
						AnticheatPlayerPacket Packet = { ACPlayer->PlayerIndex };
						memcpy(Packet.AnticheatName, sp->alias.c_str(), sp->alias.size());
						PacketHeader tmpHeader;
						AnticheatPlayerPacket tmpPacket = {};
						sp_ac->headerCrypt.rc6_block_encrypt((WORD*)&Head, (WORD*)&tmpHeader);
						sp_ac->myCrypt.rc6_cbc_encrypt((ULONGLONG*)&Packet, (ULONGLONG*)&tmpPacket, sizeof(AnticheatPlayerPacket));
						sp_ac->send(&tmpHeader, 8, Myield);
						sp_ac->send(&tmpPacket, sizeof(AnticheatPlayerPacket), Myield);
					}
				}
			}
		}
		else
		{
			ret = true;
		}
		break;
	}
	case event_leave_server:
	{
		if (Header.Size >= 4)
		{	
			for (size_t i = 0; i < AnticheatClients.size(); i++)
			{
				if (auto sp_ac = AnticheatClients[i].lock())
				{
					if (sp_ac->SessionKey == *((DWORD*)Buffer))
					{
						PacketHeader Head;
						Head.Size = 4;
						Head.Type = 2;
						Head.SubType = event_leave_server;

						DWORD tmpPlayerIndex = sp_ac->PlayerIndex;
						for (size_t j = 0; j < AnticheatClients.size(); j++)
						{
							if (auto sp_ac_s = AnticheatClients[j].lock())
							{
								PacketHeader tmpHeader;
								sp_ac_s->headerCrypt.rc6_block_encrypt((WORD*)&Head, (WORD*)&tmpHeader);
								sp_ac_s->send(&tmpHeader, 8, Myield);
								sp_ac_s->send(&tmpPlayerIndex, 4, Myield);
							}
						}
						AnticheatClients.erase(AnticheatClients.begin() + i);
						break;
					}
				}
			}
		}
		else
		{
			ret = true;
		}
		break;
	}
	//case event_screenshot:
	//{
	//	if (Buffer != NULL)
	//	{	
	//		async_anything([&]
	//		{
	//			SYSTEMTIME st = {};
	//			GetLocalTime(&st);

	//			string filename = FormatArgs("%hs\\%hs %04hu-%02hu-%02hu %02hu_%02hu_%02hu.jpg", ssname.c_str(), alias.c_str(), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	//			FILE* screenshot;
	//			if (!fopen_s(&screenshot, filename.c_str(), "wb") && screenshot)
	//			{
	//				fwrite(Buffer, Header.Size, 1, screenshot);
	//				fclose(screenshot);
	//			}
	//		});
	//	}
	//	else
	//	{
	//		ret = true;
	//	}
	//	break;
	//}
	case event_process:
	{
		if (Buffer != NULL)
		{
			string str = (char*)Buffer;
			WriteLog(FormatArgs("Process running: %hs", str.c_str()));

			ToLower(str);
			if (str.find("halo") != string::npos)
			{
				if (str.find("halo.exe") == string::npos && str.find("haloce.exe") == string::npos)
				{
					WriteGlobalLog(FormatArgs("Process running for %hs: %hs", username.c_str(), str.c_str()));
				}			
			}
			else if (str.find("windows") == string::npos && str.find("program") == string::npos && str.find("appdata") == string::npos)
			{
				WriteGlobalLog(FormatArgs("Process running for %hs: %hs", username.c_str(), str.c_str()));
			}
		}
		else
		{
			ret = true;
		}
		break;
	}
	case event_module:
	{
		if (Buffer != NULL)
		{
			string str = (char*)Buffer;
			WriteLog(FormatArgs("Module loaded: %hs", str.c_str()));

			ToLower(str);
			if (str.find("halo") != string::npos)
			{
				if (str.find("halo.exe") == string::npos && str.find("haloce.exe") == string::npos && str.find("strings.dll") == string::npos && str.find("keystone.dll") == string::npos && str.find("ksimeui.dll") == string::npos && str.find("controls.dll") == string::npos && str.find("binkw32.dll") == string::npos && str.find("vorbisfile.dll") == string::npos && str.find("ogg.dll") == string::npos && str.find("msvcr71.dll") == string::npos && str.find("vorbis.dll") == string::npos)
				{
					WriteGlobalLog(FormatArgs("Module loaded for %hs: %hs", username.c_str(), str.c_str()));
				}
			}
			else if (str.find("windows") == string::npos && str.find("program") == string::npos && str.find("appdata") == string::npos)
			{
				WriteGlobalLog(FormatArgs("Module loaded for %hs: %hs", username.c_str(), str.c_str()));
			}
		}
		else
		{
			ret = true;
		}
		break;
	}
	case event_report:
	{
		if (Header.Size >= sizeof(AntcheatReport))
		{			
			AntcheatReport* Report = (AntcheatReport*)Buffer;
			vector<string> res;
			async_anything([&] { GlobalServer->MySQL.QueryResult(FormatArgs("SELECT username, reports FROM ac_users WHERE session_key = %d", Report->Reporter), res, 2); });
			if (res.size() == 2)
			{
				size_t reports = strtoul(res[1].c_str(), NULL, 10);
				if (reports > 0)
				{
					string name = res[0];
					res.clear();
					async_anything([&] { GlobalServer->MySQL.QueryResult(FormatArgs("SELECT username FROM ac_users WHERE session_key = %d", Report->Reported), res, 1); });
					if (res.size())
					{
						Report->Message[63] = 0;
						async_anything([&] { GlobalServer->MySQL.Query(FormatArgs("UPDATE ac_users SET reports = reports - 1 WHERE session_key = %d", Report->Reporter)); });
						GlobalServer->AnticheatLog.Log("report.log", FormatArgs("%hs was reported by %hs, message: %hs", res[0].c_str(), name.c_str(), Report->Message));
						wstring wstr = FormatWArgs(L"The report has been sent, you have %u reports left for today!", reports - 1);
						
						shared_lock<shared_mutex> lock(GlobalServer->ClientVectorLock);
						for (auto it = GlobalServer->Clients.begin(); it != GlobalServer->Clients.end(); ++it)
						{
							if (auto sp = it->lock())
							{
								if (sp->IsAnticheat && sp->SessionKey == Report->Reporter)
								{
									PacketHeader pHeader = {};
									pHeader.SubType = 1;
									pHeader.Flags = 2501;
									pHeader.Size = 128;
									sp->headerCrypt.rc6_block_encrypt((WORD*)&pHeader, (WORD*)&pHeader);
									sp->myCrypt.rc6_cbc_encrypt((ULONGLONG*)wstr.c_str(), 128);
									sp->send(&pHeader, 8, Myield);
									sp->send(wstr.c_str(), 128, Myield);
									break;
								}
							}
						}				
					}
				}
				else
				{
					wchar_t buff[48] = L"You have no more reports left for today!";
					SendBigEvent(event_join_client, buff, 96);
				}
			}
		}
		else
		{
			ret = true;
		}
		break;
	}
	case event_alias:
	{
		if (Header.Size > 0 && Header.Size < 32)
		{
			bool success = true;
			for (size_t i = 0; i < Header.Size; i++)
			{
				if (!((Buffer[i] >= '0' && Buffer[i] <= '9') || (Buffer[i] >= 'a' && Buffer[i] <= 'z') || (Buffer[i] >= 'A' && Buffer[i] <= 'Z')))
				{
					success = false;
					break;
				}
			}

			if (success)
			{
				string tmp_alias(Buffer, Header.Size);
				vector<string> v;
				v.emplace_back(tmp_alias);
				vector<string> res;
				async_anything([&] { GlobalServer->MySQL.QueryResultPreparedStatement("SELECT userid FROM ac_users WHERE username = ?", v, res, 1); });
				if (res.size())
				{
					size_t tmpUserID = strtoul(res[0].c_str(), NULL, 10);
					if (tmpUserID != UserID)
					{
						success = false;
					}
				}

				if (success)
				{
					string tmp2_alias;
					tmp2_alias.resize(tmp_alias.size());
					transform(tmp_alias.begin(), tmp_alias.end(), tmp2_alias.begin(), tolower);

					shared_lock<shared_mutex> lock(GlobalServer->AliasLock);
					bool found = GlobalServer->Aliases.find(tmp2_alias) != GlobalServer->Aliases.end();
					lock.unlock();

					if (!found)
					{
						ssname = FormatArgs("screenshots\\%hs", tmp_alias.c_str());
						async_anything([&]
						{
							CreateDirectoryA(ssname.c_str(), NULL);
						});

						unique_lock<shared_mutex> ulock(GlobalServer->AliasLock);
						GlobalServer->Aliases.erase(lowalias);
						GlobalServer->Aliases.emplace(tmp2_alias);
						ulock.unlock();
						alias = move(tmp_alias);
						lowalias = move(tmp2_alias);
					}
					else
					{
						success = false;
					}
				}
			}

			SendSmallEvent(event_alias, success);
		}
		else
		{
			ret = true;
		}
		break;
	}
	default:
		ret = true;
	}
	return ret;
}

//bool SappServer::ProcessPacket_HaloRank(const PacketHeader& Header, char* Buffer)
//{
//	bool ret = false;
//	switch (Header.SubType)
//	{
//	case 0: // Ping
//	{
//		char buff[8] = {};
//		send(Socket, buff, 8, NULL);
//		break;
//	}
//	case 1: // Event
//	{
//		if(Buffer != NULL)
//			ret = ProcessEvent((HR_Event*)Buffer);
//		break;
//	}
//	default:
//		ret = true;
//	}
//	return ret;
//}

bool SappServer::ProcessPacket(const PacketHeader& Header, const char* Buffer)
{
	bool ret = false;
	switch (Header.Type)
	{
	case 0: // Remote Console
		ret = ProcessPacket_RemoteConsole(Header, Buffer);
		break;
	case 1: // Sapp Server
		ret = ProcessPacket_SappServer(Header, Buffer);
		break;
	case 2:  // HAC
		ret = ProcessPacket_HAC(Header, Buffer);
		break;	
	//case 3: // Halo Rank
	//	ret = ProcessPacket_HaloRank(Header, Buffer);
	//	break;	
	//case 4: // Server Query
	//	
	//	break;
	default:
		ret = true;
	}
	return ret;
}

void SappServer::ClientThread()
{
	boost::system::error_code ec;
	bool ConnectionOK = true;
	while (!GlobalServer->ShutDown && ConnectionOK)
	{		
		PacketHeader Header;		
		size_t lenght = boost::asio::async_read(socket, boost::asio::buffer(&Header, sizeof(PacketHeader)), Myield[ec]);
		if (!ec && lenght == sizeof(PacketHeader))
		{
			unique_ptr<char[]> MyBuffer;
			if (IsAnticheat)
				headerCrypt.rc6_block_decrypt((WORD*)&Header, (WORD*)&Header);
			if (Header.Size) // Packet has payload
			{
				if (Header.Size > 0x00FFFFFF) // Max 16MB
					ConnectionOK = false;
				else
				{
					MyBuffer = make_unique<char[]>(Header.Size);
					DWORD Size = 0;
					while (Size < Header.Size)
					{
						size_t Recived = boost::asio::async_read(socket, boost::asio::buffer(MyBuffer.get() + Size, Header.Size - Size), Myield[ec]);
						if (!ec && Recived > 0)
							Size += Recived;
						else
						{
							ConnectionOK = false;
							break;
						}
					}
				}
			}

			if (ConnectionOK)
			{
				if (MyBuffer != nullptr && IsAnticheat)
					myCrypt.rc6_cbc_decrypt((ULONGLONG*)MyBuffer.get(), Header.Size);
				if (ProcessPacket(Header, MyBuffer.get()))
					ConnectionOK = false;
			}
		}
		else // Corrupted Header
			ConnectionOK = false;
	}

	WriteLog("Connection terminated.");

	CloseConnection();

	CleanUp();
}

void SappServer::Start()
{
	async_anything([&] { GlobalServer->MySQL.Query(FormatArgs("INSERT INTO sapp_servers (serverid) VALUES (%u)", this)); });

	GlobalServer->SafePrint.Print(FormatArgs("Client Connected,    IP:%16hs | Connections: %u", ipstr.c_str(), ++GlobalServer->ClientNumber));

	ClientThread();

	GlobalServer->SafePrint.Print(FormatArgs("Client Disconnected, IP:%16hs | Connections: %u", ipstr.c_str(), --GlobalServer->ClientNumber));

	async_anything([&] { GlobalServer->MySQL.Query(FormatArgs("DELETE FROM sapp_servers WHERE serverid = %u", this)); });
}

void SappServer::CloseConnection()
{
	boost::system::error_code ec;
	socket.close(ec);
}

void SappServer::CleanUp()
{	
	IsAnticheat = false;
	IsServer = false;
	MyServer.reset();
	MAC_Addresses.clear();
	AnticheatClients.clear();
	if (lowalias.size())
	{
		unique_lock<shared_mutex> ulock(GlobalServer->AliasLock);
		GlobalServer->Aliases.erase(lowalias);
	}
}

//void SappServer::DoAsync(const std::function <void()>& f)
//{
//	auto aret = async([&]
//	{
//		f();
//
//		boost::system::error_code ec;
//		while (!timer.cancel(ec))
//			Sleep(1);
//	});
//
//	timer.async_wait(Myield[boost::system::error_code()]);	
//}

void SappServer::send(const void* data, size_t len)
{
	SendLock.Lock();
	boost::system::error_code ec;
	boost::asio::async_write(socket, boost::asio::buffer(data, len), Myield[ec]);
	if (ec)
	{
		CloseConnection();
	}
	SendLock.Unlock();
}

void SappServer::send(const void* data, size_t len, const boost::asio::yield_context &yield)
{
	SendLock.Lock();
	boost::system::error_code ec;	
	boost::asio::async_write(socket, boost::asio::buffer(data, len), yield[ec]);	
	if (ec)
	{
		CloseConnection();
	}
	SendLock.Unlock();
}

void SappServer::SendSmallEvent(event_type type, WORD flags)
{	
	PacketHeader Header = {};
	Header.Type = 2;
	Header.SubType = type;
	Header.Flags = flags;
	if (IsAnticheat)
	{
		headerCrypt.rc6_block_encrypt((WORD*)&Header, (WORD*)&Header);
	}
	send(&Header, 8);
}

void SappServer::SendBigEvent(event_type type, void* buff, size_t size)
{
	PacketHeader Header = {};
	Header.Size = size;
	Header.Type = 2;
	Header.SubType = type;
	if (IsAnticheat)
	{
		headerCrypt.rc6_block_encrypt((WORD*)&Header, (WORD*)&Header);
		myCrypt.rc6_cbc_encrypt((ULONGLONG*)buff, size);
	}
	send(&Header, 8);
	send(buff, size);
}

void NetworkServer::AcceptConnections(boost::asio::yield_context yield)
{
	boost::system::error_code ec;
	boost::coroutines::attributes attrib(16384);
	while (!ShutDown)
	{	
		acceptor.async_accept(socket, yield[ec]);
		if (!ec)
		{
			boost::asio::spawn(service, [this, mysocket = move(socket)](boost::asio::yield_context yield) mutable
			{
				auto sp = make_shared<SappServer>(this, service, move(mysocket), yield);
				unique_lock<shared_mutex> lock(ClientVectorLock);
				Clients.emplace_back(sp);
				lock.unlock();
				sp->Start();
			}, attrib);
		}
	}
}

void NetworkServer::StartServer()
{
	ShutDown = false;
#ifndef DEBUG
	MySQL.Query("TRUNCATE TABLE sapp_servers");
	MySQL.Query("UPDATE ac_users SET session_key = NULL");
#endif

	acceptor.listen();

	boost::coroutines::attributes attrib(16384);
	boost::asio::spawn(service, bind(&NetworkServer::AcceptConnections, this, _1), attrib);
	boost::asio::spawn(service, bind(&NetworkServer::CleanupConnections, this, _1), attrib);
	boost::asio::spawn(service, bind(&NetworkServer::UpdateServerCount, this, _1), attrib);

	size_t concurrency = thread::hardware_concurrency() * 2;

	if (!concurrency)
	{
		concurrency = 4;
		SafePrint.Print("Couldn't determine concurrency, using the default value (4).");
	}

	for (size_t i = 0; i < concurrency; i++)
	{
		threads.emplace_back(static_cast<size_t(boost::asio::io_service::*)()>(&boost::asio::io_service::run), &service);
	}

	ThreadPool.start();
	SafePrint.Start();
	AnticheatLog.Start();
}

void NetworkServer::StopServer()
{
	ShutDown = true;

	acceptor.close();

	socket.close();

	shared_lock<shared_mutex> lock(ClientVectorLock);
	for (auto it = Clients.begin(); it != Clients.end(); ++it)
	{
		if (auto sp = it->lock())
		{
			sp->CloseConnection();
		}
	}
	lock.unlock();

	timer.cancel();

	update_stats_timer.cancel();

	service.stop();

	for (auto& t : threads)
	{
		t.join();
	}

	ThreadPool.shutdown();
}

void NetworkServer::CleanupConnections(boost::asio::yield_context yield)
{
	boost::system::error_code ec;
	while (!ShutDown)
	{
		time_t cTime = time(0);

		unique_lock<shared_mutex> lock(ClientVectorLock);

		auto it = Clients.begin();
		while (it != Clients.end())		
		{
			if (auto sp = it->lock())
			{
#ifndef DEBUG
				if ((cTime - sp->LastUpdated) > (sp->IsAnticheat ? 30 : 300))
#else
				if ((cTime - sp->LastUpdated) > 3600)
#endif
				{
					sp->CloseConnection();
				}
				++it;
			}
			else
			{
				it = Clients.erase(it);
				continue;
			}
		}

		lock.unlock();

		timer.expires_from_now(boost::posix_time::minutes(1));
		timer.async_wait(yield[ec]);
	}
}

void NetworkServer::UpdateServerCount(boost::asio::yield_context yield)
{
	boost::system::error_code ec;
	while (!ShutDown)
	{
		update_stats_timer.expires_from_now(boost::posix_time::minutes(15));
		update_stats_timer.async_wait(yield[ec]);

		if (!ShutDown)
		{
			MySQL.Query("INSERT INTO server_count (count) VALUES((SELECT COUNT(serverid) FROM sapp_servers WHERE maxplayers != 0 LIMIT 1))");
			MySQL.Query("INSERT INTO player_count (count) VALUES((SELECT SUM(players) FROM sapp_servers WHERE maxplayers != 0 LIMIT 1))");
		}
	}
}

void NetworkServer::ListServers()
{
	shared_lock<shared_mutex> lock(ClientVectorLock);
	for (auto it = Clients.begin(); it != Clients.end(); ++it)
	{
		if (auto sp = it->lock())
		{
			SafePrint.Print(FormatArgs("Name: %ls\nMap: %ls Mode: %ls Players: %u/%u", sp->Stats.ServerName, sp->Stats.Map, sp->Stats.Mode, sp->Stats.Players, sp->Stats.MaxPlayers));
		}
	}
}

void NetworkServer::ListServersToFile()
{
	FILE* file = NULL;
	_wfopen_s(&file, L"server_stats.txt", L"w");
	if(file)
	{
		shared_lock<shared_mutex> lock(ClientVectorLock);
		for(auto it = Clients.begin(); it != Clients.end(); ++it)
		{
			if (auto sp = it->lock())
			{
				fprintf_s(file, "IP: %hhu.%hhu.%hhu.%hhu:%hu Name: %ls Map: %ls Mode: %ls Players: %u/%u No-Lead: %u Version: %hs SAPP Version: %08X %hs\n", sp->Stats.IP & 0xFF, (sp->Stats.IP >> 8) & 0xFF, (sp->Stats.IP >> 16) & 0xFF, (sp->Stats.IP >> 24) & 0xFF, sp->Stats.Port, sp->Stats.ServerName, sp->Stats.Map, sp->Stats.Mode, sp->Stats.Players, sp->Stats.MaxPlayers, sp->Stats.NoLead, sp->Stats.VersionString, sp->Stats.SappVersion, sp->Stats.HaloVerType == 0 ? "PC" : "CE");
			}
		}

		fclose(file);
	}
}

void NetworkServer::LoadMaps()
{
	WORD lfsr;
	RtlGenRandom(&lfsr, 2);
	if (!lfsr) lfsr = 1;

	const char* path = "C:\\Halo CE\\map_7z\\*.7z";

	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile(path, &FindData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{		
			ToLower(FindData.cFileName);
			string name = FindData.cFileName;
			name.erase(name.end() - 3, name.end());
			lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xB400u);
			MapTable[lfsr] = move(name);
		}
		while(FindNextFile(hFind, &FindData));
		FindClose(hFind);
	}
}

void NetworkServer::ListMaps()
{
	for (auto it = MapTable.begin(); it != MapTable.end(); ++it)
		SafePrint.Print(FormatArgs("%hs\n", it->second.c_str()));

	SafePrint.Print(FormatArgs("Number of maps loaded: %u", MapTable.size()));
}

void NetworkServer::CacheFiles()
{
	unique_lock<shared_mutex> lock(FileCacheLock);
	for (size_t i = 0; i < 6; i++)
	{
		size_t size = 0;
		HANDLE hFile = CreateFile(Files[i], GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			FileSizes[i] = size = GetFileSize(hFile, NULL);
			CloseHandle(hFile);
		}

		FILE* File;
		if (!fopen_s(&File, Files[i], "rb") && File && size)
		{
			FileBuffs[i] = make_unique<const BYTE[]>(size);
			fread(const_cast<BYTE*>(FileBuffs[i].get()), size, 1, File);
			fclose(File);
			SafePrint.Print(FormatArgs("Successfully loaded %hs!", Files[i]));
		}
		else
		{
			SafePrint.Print(FormatArgs("ERROR: Failed to open %hs!", Files[i]));
		}
	}
	SappUpdateCountLock.Lock();
	SappUpdateCount.clear();
	SappUpdateCountLock.Unlock();
}

void NetworkServer::LoadHashes()
{
	fstream HashFile;
	HashFile.open("hpc_hashes.txt", ios::in);
	if (!HashFile.is_open())
	{
		SafePrint.Print("Failed to open Halo PC Hash file!");
	}
	else
	{
		string map, hash;
		while (getline(HashFile, map) && getline(HashFile, hash))
		{
			HaloPCHashes[move(map)] = move(hash);
		}
		HashFile.close();
	}

	HashFile.open("hce_hashes.txt", ios::in);
	if (!HashFile.is_open())
	{
		SafePrint.Print("Failed to open Halo CE Hash file!");
	}
	else
	{
		string map, hash;
		while (getline(HashFile, map) && getline(HashFile, hash))
		{
			HaloCEHashes[move(map)] = move(hash);
		}
		HashFile.close();
	}
}

void NetworkServer::LockConsole()
{
	SafePrint.ManualLockConsole();
}

void NetworkServer::UnlockConsole()
{
	SafePrint.ManualUnlockConsole();
}

void NetworkServer::NewSapp(const string& str)
{
	size_t ver;
	ToHexNumber(str, ver);
	SAPPVersion = ver;
}

void NetworkServer::BroadcastServers(const string &msg)
{	
	boost::asio::spawn(service, [this, message = msg](boost::asio::yield_context yield)
	{
		PacketHeader Header = {};
		Header.SubType = 1;
		Header.Flags = 2501;
		Header.Size = message.size() + 1;

		shared_lock<shared_mutex> lock(ClientVectorLock);
		for (auto it = Clients.begin(); it != Clients.end(); ++it)
		{
			if (auto sp = it->lock())
			{
				if (sp->IsServer)
				{
					sp->send(&Header, 8, yield);
					sp->send(message.c_str(), Header.Size, yield);
				}
			}
		}
	});
}

void NetworkServer::BroadcastACServers(const string &msg)
{
	boost::asio::spawn(service, [this, message = msg](boost::asio::yield_context yield)
	{
		PacketHeader Header = {};
		Header.SubType = 1;
		Header.Flags = 2501;
		Header.Size = message.size() + 1;

		shared_lock<shared_mutex> lock(ClientVectorLock);
		for (auto it = Clients.begin(); it != Clients.end(); ++it)
		{
			if (auto sp = it->lock())
			{
				if (sp->IsACServer)
				{
					sp->send(&Header, 8, yield);
					sp->send(message.c_str(), Header.Size, yield);
				}
			}
		}
	});
}

void NetworkServer::BroadcastAC(const string & msg)
{
	boost::asio::spawn(service, [this, message = msg](boost::asio::yield_context yield)
	{
		wstring wmsg(message.begin(), message.end());

		size_t size = (wmsg.size() + 1) * sizeof(wchar_t);

		PacketHeader Header = {};
		Header.SubType = 1;
		Header.Flags = 2501;
		Header.Size = size;

		shared_lock<shared_mutex> lock(ClientVectorLock);
		for (auto it = Clients.begin(); it != Clients.end(); ++it)
		{
			if (auto sp = it->lock())
			{
				if (sp->IsAnticheat)
				{
					PacketHeader tmpHeader;
					char* buff = new char[size];
					memcpy(buff, wmsg.c_str(), size);
					sp->headerCrypt.rc6_block_encrypt((WORD*)&Header, (WORD*)&tmpHeader);
					sp->myCrypt.rc6_cbc_encrypt((ULONGLONG*)buff, size);
					sp->send(&tmpHeader, 8, yield);
					sp->send(buff, size, yield);
					delete[] buff;
				}
			}
		}
	});
}
