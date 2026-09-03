#pragma once

#include <time.h>

#include <map>
#include <memory>
#include <set>
#include <shared_mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost\asio\buffer.hpp>
#include <boost\asio\io_service.hpp>
#include <boost\asio\ip\tcp.hpp>
#include <boost\asio\handler_invoke_hook.hpp>
#include <boost\asio\read.hpp>
#include <boost\asio\spawn.hpp>
#include <boost\asio\write.hpp>
#include <boost\noncopyable.hpp>

#include "Events\ACEvents.h"
#include "RC6\rc6_64.h"
#include "RC6\rc6_16.h"

#include "Locker.h"
#include "Logger.h"
#include "MySQLServer.h"
#include "SafePrint.h"
#include "structs.h"
#include "thread_pool.h"

//#define DEBUG

class NetworkServer;

class SappServer : private boost::noncopyable, public std::enable_shared_from_this<SappServer>
{
	friend class NetworkServer;

	NetworkServer* GlobalServer;
	std::weak_ptr<SappServer> MyServer;

	HANDLE hThread;

	boost::asio::io_service &service;
	boost::asio::ip::tcp::socket socket;
	boost::asio::yield_context &Myield;

	ServerStats Stats = {};

	Locker SendLock;

	RC6_16 headerCrypt;
	RC6_64 myCrypt;

	DWORD UserID = -1;
	DWORD SessionKey = 0;
	DWORD MyIP;

	time_t LastUpdated;

	std::string ipstr;
	std::string username;
	std::string logname;
	std::string ssname;
	std::string alias;
	std::string lowalias;

	DWORD PlayerIndex = -1;

	bool IsAnticheat = false;
	bool IsACServer = false;
	bool IsServer = false;
	BYTE HaloType = 0;

	bool UpdateOngoing = false;
	bool GotHaloChecksum = false;
	bool GotStringsChecksum = false;

	//deque <HR_Event*> Events;
	std::vector <ULONGLONG> MAC_Addresses;
	std::vector <std::weak_ptr<SappServer>> AnticheatClients;

	struct async_completion
	{
		async_completion(boost::asio::yield_context& token) : handler(std::forward<boost::asio::yield_context&>(token)), result(handler) {}

		boost::asio::handler_type<boost::asio::yield_context&, void()>::type handler;
		boost::asio::async_result<boost::asio::handler_type<boost::asio::yield_context&, void()>::type> result;
	};

	boost::asio::async_result<boost::asio::handler_type<boost::asio::yield_context&, void()>::type>::type
	async_anything(const std::function <void()> &f);

	void ClientThread();

	void WriteLog(const std::string &str);
	void WriteGlobalLog(const std::string &str);

	void Terminate(const char* reason);
	void Ban(const char* reason);

	void SendFile(WORD Flags);
	void SendMap(WORD Index);

	//bool ProcessEvent(HR_Event* Event);
	//bool ProcessEventStart(char* serverkey);
	//void ProcessEventGameStart(CurrentGame* cg);
	//void ProcessEventGameEnd();

	bool ProcessPacket(const PacketHeader& Header, const char* Buffer);
	bool ProcessPacket_RemoteConsole(const PacketHeader& Header, const char* Buffer);
	bool ProcessPacket_SappServer(const PacketHeader& Header, const char* Buffer);
	bool ProcessPacket_HAC(const PacketHeader& Header, const char* Buffer);
	//bool ProcessPacket_HaloRank(const PacketHeader& Header, char* Buffer);

	void CloseConnection();

	void CleanUp();

	void send(const void* data, size_t len);
	void send(const void* data, size_t len, const boost::asio::yield_context &yield);

	void Start();
public:

	SappServer(NetworkServer* NServer, boost::asio::io_service &service, boost::asio::ip::tcp::socket&& MySocket, boost::asio::yield_context &yield)
		: GlobalServer(NServer), service(service), socket(std::move(MySocket)), Myield(yield), LastUpdated(time(0)),
		MyIP(endian_swap(socket.remote_endpoint().address().to_v4().to_ulong())), ipstr(socket.remote_endpoint().address().to_string()) { }
	void SendSmallEvent(event_type type, WORD flags = 0);
	void SendBigEvent(event_type type, void* buffer, size_t size);
};

class NetworkServer : private boost::noncopyable
{
	friend class SappServer;

	boost::asio::io_service service;
	boost::asio::ip::tcp::acceptor acceptor;
	boost::asio::ip::tcp::socket socket;
	boost::asio::deadline_timer timer;
	boost::asio::deadline_timer update_stats_timer;

	MySQLServer MySQL;

	thread_pool ThreadPool;

	std::vector<std::thread> threads;

	std::vector <std::weak_ptr<SappServer>> Clients;
	std::shared_mutex ClientVectorLock;

	DWORD ClientNumber = 0;

	std::map <DWORD, DWORD> SappUpdateCount;
	Locker SappUpdateCountLock;

	std::map <WORD, std::string> MapTable;
	std::map <std::string, std::string> HaloPCHashes;
	std::map <std::string, std::string> HaloCEHashes;

	Safe_Print SafePrint;
	Logger AnticheatLog;

	DWORD FileSizes[6] = {};
	std::unique_ptr<const BYTE[]> FileBuffs[6];
	std::shared_mutex FileCacheLock;

	std::set <std::string> Aliases;
	std::shared_mutex AliasLock;

	bool ShutDown = false;

	void AcceptConnections(boost::asio::yield_context yield);
	void CleanupConnections(boost::asio::yield_context yield);
	void UpdateServerCount(boost::asio::yield_context yield);
	
public:

	NetworkServer(unsigned short port) : acceptor(service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), socket(service), timer(service), update_stats_timer(service), ThreadPool(64), MySQL(&SafePrint)
	{
		LoadHashes();
		LoadMaps();
		CacheFiles();
	}

	void StartServer();
	void StopServer();
	void ListServers();
	void ListServersToFile();
	void LoadMaps();
	void ListMaps();
	void CacheFiles();
	void LoadHashes();
	void LockConsole();
	void UnlockConsole();
	void NewSapp(const std::string& str);
	void BroadcastServers(const std::string &msg);
	void BroadcastACServers(const std::string &msg);
	void BroadcastAC(const std::string &msg);
};