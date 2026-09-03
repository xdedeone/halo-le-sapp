#include <ws2tcpip.h>

#include "NetworkClient.h"

#include "7z_dec.h"
#include "Addresses.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "MapLoader.h"
#include "MiscellaneousFunctions.h"
#include "NetworkThread.h"
#include "Prefs.h"
#include "TextUtilities.h"

using namespace std;

const char* const addresses[] = {"halo.isimaginary.com", "xhalo.tk", "sapp.haloanticheat.com"};
const wchar_t* const Files[] = { L"sapp_updater.dll", L"sapp.dat" };

size_t selected = 0;
bool downloading = false;

Enviroment MapDownload_Env = Enviroment::SCRIPT;
DWORD MapDownload_PlayerIndex = -1;
time_t MapDownload_LastQuery = 0;

vector <MapDownloadEntry> QueryedMaps;

bool UpdateAvalibe = false;
bool FilesReceived[3] = {};

void GetMap(const PacketHeader& Header, const char* Buffer)
{
	CommandEnviroment env(MapDownload_Env, MapDownload_PlayerIndex);

	if(Header.Size && Buffer)
	{
		HaloThreadLock.Lock();
		env.Print(FormatArgs("Map %hs downloaded, extracting...", QueryedMaps[selected].Name).c_str());
		HaloThreadLock.Unlock();

		bool success = false;

		wstring file_path = FormatWArgs(L"maps\\%hs.7z", QueryedMaps[selected].Name);

		HANDLE h = CreateFileW(file_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
		if (h != INVALID_HANDLE_VALUE)
		{
			DWORD written;
			WriteFile(h, Buffer, Header.Size, &written, NULL);
			SetFilePointer(h, NULL, NULL, FILE_BEGIN);

			success = !Extract7zFile(h, L"maps\\");

			CloseHandle(h);
		}
		else
		{
			HaloThreadLock.Lock();
			env.Print(FormatArgs("Failed to extract %hs.7z!", QueryedMaps[selected].Name).c_str());
			HaloThreadLock.Unlock();
		}

		if(success)
		{
			HaloThreadLock.Lock();
			env.Print(FormatArgs("Map %hs extracted, loading...", QueryedMaps[selected].Name).c_str());
			HaloThreadLock.Unlock();

			DWORD Sum;
			if(Checksum((char*)QueryedMaps[selected].Name, Sum) == ChecksumResult::CHECKSUM_SUCCESS)
			{
				HaloThreadLock.Lock();
				AddMap((char*)QueryedMaps[selected].Name, Sum);
				env.Print(FormatArgs("Successfully loaded %hs.map!", QueryedMaps[selected].Name).c_str());
				HaloThreadLock.Unlock();
			}
			else
			{
				HaloThreadLock.Lock();
				env.Print(FormatArgs("Failed to read %hs.map!", QueryedMaps[selected].Name).c_str());
				HaloThreadLock.Unlock();
			}
		}
		else
		{
			HaloThreadLock.Lock();
			env.Print(FormatArgs("Failed to extract %hs.7z!", QueryedMaps[selected].Name).c_str());
			HaloThreadLock.Unlock();
		}
	}
	else
	{
		HaloThreadLock.Lock();
		env.Print(FormatArgs("Downloading map %hs failed, please try again later.", QueryedMaps[selected].Name).c_str());
		HaloThreadLock.Unlock();
	}

	MapDownload_PlayerIndex = -1;
	QueryedMaps.clear();
	downloading = false;
}

void GetFile(const PacketHeader& Header, const char* Buffer)
{
	WORD hFlags = Header.Flags;
	if (hFlags > 1)
		hFlags--;

	if (hFlags < 2 && Buffer)
	{
		FILE* File;
		if (!_wfopen_s(&File, Files[hFlags], L"wb") && File)
		{
			fwrite(Buffer, Header.Size, 1, File);
			fclose(File);
			FilesReceived[hFlags] = true;
			if (UpdateAvalibe && FilesReceived[0] && FilesReceived[1])
			{
				LoadLibraryW(sapp_updater_file);
			}
		}
	}
}

bool NetworkClient::ProcessPacket_General(const PacketHeader& Header, const char* Buffer)
{
	bool ret = false;
	switch (Header.SubType)
	{
	case 0: // Ping / HB
	{
		break;
	}
	case 1: // Broadcasted Message
	{
		if(Header.Flags == 2501 && Buffer)
		{
			HaloThreadLock.Lock();
			PrintConsole(FormatArgs("%hs", Buffer).c_str());
			sv_say(FormatWArgs(L"%hs", Buffer).c_str());
			HaloThreadLock.Unlock();
		}
		break;
	}
	default:
		ret = true;
	}
	return ret;
}

bool NetworkClient::ProcessPacket_SappServer(const PacketHeader& Header, const char* Buffer)
{
	bool ret = false;
	switch (Header.SubType)
	{
	case 0: // Get IP
	{
		if(Buffer)
		{
			MyServerStats.IP = *(DWORD*)Buffer;

			HaloThreadLock.Lock();
			PrintConsole(FormatArgs("External IP Received: %hs:%u", inet_ntoa(*(in_addr*)&MyServerStats.IP), Port).c_str());
			HaloThreadLock.Unlock();
		}
		break;
	}
	case 1: // SAPP Version
	{
		if(Buffer && *(DWORD*)Buffer > CurrentVersion)
		{
			UpdateAvalibe = true;
			if (GlobalPrefs->AutoUpdate)
			{
				PacketHeader PHeader = {};
				PHeader.Type = 1;
				PHeader.SubType = 2;
				sendHeader(PHeader); // Get sapp_updater.dll
			#ifndef HPC
				PHeader.Flags = 1;
			#else
				PHeader.Flags = 2;
			#endif
				sendHeader(PHeader); // Get sapp.dll
			}
		}
		break;
	}
	case 2: // Get Files
	{
		if(Header.Flags > 2)
		{
			if(Header.Flags == 0xFFFF)
				GetMap(Header, Buffer);
		}
		else
			GetFile(Header, Buffer);
		break;
	}
	case 3: // Stats OK
	{
			
		break;
	}
	case 4: // Get Map Query response
	{
		CommandEnviroment env(MapDownload_Env, MapDownload_PlayerIndex);

		HaloThreadLock.Lock();
		if(Header.Size)
		{
			env.Print("ID  Name");

			size_t size = Header.Size / sizeof(MapDownloadEntry);
			for(size_t i = 0; i < size; i++)
			{
				env.Print(FormatArgs("%2u  %hs", i, ((MapDownloadEntry*)Buffer)[i].Name).c_str());
				QueryedMaps.emplace_back(((MapDownloadEntry*)Buffer)[i]);
			}
		}
		else
		{
			env.Print("No map matches this expression!");
		}
		HaloThreadLock.Unlock();

		break;
	}
	case 5: // AC login
	{
		if(Buffer)
		{
			ServerID = *(DWORD*)Buffer;
			if (ServerID != 0)
			{
				AntiCheat = true;
			}
			else
			{
				GlobalPrefs->AntiCheat = false;
			}
		}
		break;
	}
	default:
		ret = true;
	}
	return ret;
}

bool NetworkClient::ProcessPacket_HAC(const PacketHeader& Header, const char* Buffer)
{
	bool ret = false;
	switch (Header.SubType)
	{
		case event_ping: // Ping
		{

			break;
		}
		case event_get_rc6: // Get RC6 Key and IV
		{

			break;
		}
		case event_map_checksum:
		{
			if (Buffer)
			{
				for (size_t i = 0; i < AnticheatPlayers.size(); i++)
				{
					if (*((DWORD*)Buffer) == AnticheatPlayers[i].SessionKey)
					{
						sv_say(FormatWArgs(L"Player %ls has a modified map and will be kicked in 5 seconds.", PlayerTable[i].name).c_str());
						size_t id = NumberToIndex(AnticheatPlayers[i].MachineID);
						if (id < 16)
						{
							ValidationTimer[id] = 149;
						}					
						break;
					}
				}				
			}
			else
			{
				ret = true;
			}
			break;
		}
		case event_join_server:
		{
			if (Buffer)
			{
				for (size_t i = 0; i < AnticheatPlayers.size(); i++)
				{
					if (*((DWORD*)Buffer) == AnticheatPlayers[i].SessionKey)
					{
						size_t id = NumberToIndex(AnticheatPlayers[i].MachineID);
						if (id < 16)
						{
							ValidationTimer[id] = 0;
						}					
						break;
					}
				}				
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

bool NetworkClient::ProcessPacket(const PacketHeader& Header, const char* Buffer)
{
	bool ret = false;
	switch (Header.Type)
	{
	case 0: // General
		ret = ProcessPacket_General(Header, Buffer);
		break;
	case 1: // Sapp Server
		ret = ProcessPacket_SappServer(Header, Buffer);
		break;
	case 2: // HAC
		ret = ProcessPacket_HAC(Header, Buffer);
		break;
	//case 2: // Server Query

	//	break;	
	//case 3: // Remote Console

	//	break;
	default:
		ret = true;
	}
	return ret;
}

void NetworkClient::ClientThreadProc(shared_ptr<work>& hThread)
{
	while(!hThread->ExitFlag)
	{
		if (!connected)
		{
			while (!Connect(hThread))
			{
				hThread->Wait(60000);
			}
		}
		else
		{
			bool ConnectionOK = true;
			PacketHeader Header;
			int size = recv(s, (char*)&Header, 8, NULL);
			if (size == 8)
			{
				std::unique_ptr<char[]> packetBuffer;
				if (Header.Size) // Packet has payload
				{
					if (Header.Size > 0x0FFFFFFF) // Max 256MB
						ConnectionOK = false;
					else
					{
						packetBuffer = unique_ptr<char[]>(new char[Header.Size]);
						DWORD Size = 0;
						while (Size < Header.Size)
						{
							int Recived = recv(s, packetBuffer.get() + Size, Header.Size - Size, NULL);
							if (Recived > 0)
							{
								Size += Recived;
							}
							else
							{
								ConnectionOK = false;
								break;
							}
						}
					}
				}

				if (ConnectionOK && ProcessPacket(Header, packetBuffer.get()))
					ConnectionOK = false;
			}
			else // Corrupted Header / DC
				ConnectionOK = false;

			if (!ConnectionOK)
			{
				connected = false;
				closesocket(s);
			}
		}
	}
}

void NetworkClient::StartThread(shared_ptr<work> &hThread)
{
	NetworkClient* This = static_cast<NetworkClient*>(hThread->GetParams());
	This->ClientThreadProc(hThread);
}

void NetworkClient::Start()
{
	ThreadPool.NewWork(&NetworkClient::StartThread, this);
}

void NetworkClient::Stop()
{
	shutdown(s, SD_BOTH);
}

bool NetworkClient::sendNoLock(const void* data, int len)
{
	char *ptr = (char*)data;

	while (len > 0)
	{
		int sent = send(s, ptr, len, 0);
		if (sent > 0)
		{
			ptr += sent;
			len -= sent;
		}
		else return false;
	}

	return true;
}

bool NetworkClient::sendHeader(const PacketHeader& Header)
{
	Lock_Object guard(sendLock);
	return sendNoLock(&Header, sizeof(PacketHeader));
}

bool NetworkClient::sendData(const void* data, int len)
{
	Lock_Object guard(sendLock);
	return sendNoLock(data, len);
}

bool NetworkClient::sendDataWithHeader(const PacketHeader& Header, const void* data, int len)
{
	Lock_Object guard(sendLock);
	return sendNoLock(&Header, sizeof(PacketHeader)) && sendNoLock(data, len);
}

bool NetworkClient::Connect(const char* addr)
{
	addrinfo hints = {};

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	PADDRINFOA Address;
	if(getaddrinfo(addr, "2501", &hints, &Address) != 0)
	{
		return false;
	}

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(s == INVALID_SOCKET)
	{
		freeaddrinfo(Address);
		return false;
	}

	sockaddr_in local = {};
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = endian_swap(IP);

	if (::bind(s, (sockaddr*)&local, sizeof(local)) != 0)
	{
		closesocket(s);
		freeaddrinfo(Address);
		return false;
	}

	if(connect(s, Address->ai_addr, Address->ai_addrlen) == SOCKET_ERROR)
	{
		closesocket(s);
		freeaddrinfo(Address);
		return false;
	}

	freeaddrinfo(Address);

	MapDownload_PlayerIndex = -1;
	QueryedMaps.clear();
	downloading = false;
	connected = true;

	return true;
}

bool NetworkClient::Connect(shared_ptr<work>& hThread)
{
	for(size_t i = 0; i < sizeof(addresses)/sizeof(char*); i++)
	{
		if(Connect(addresses[i]))
		{
			GetIP();
			if (GlobalPrefs->AntiCheat)
				ACLogin();
			return true;
		}
		if (hThread->ExitFlag)
			return true;
	}
	return false;
}

bool NetworkClient::IsConnected()
{
	return connected;
}

void NetworkClient::GetIP()
{
	PacketHeader Header = {};
	Header.Type = 1;
	sendHeader(Header);
}

void NetworkClient::VersionCheck()
{
	PacketHeader Header = {};
	Header.Type = 1;
	Header.SubType = 1;
	sendHeader(Header);
}

void NetworkClient::SendStats()
{
	if(ServerRunning())
	{
		wcscpy_s(MyServerStats.ServerName, 64, Server->ServerName);
		wcscpy_s(MyServerStats.Map, 32, CurrentMapW.c_str());
		wcscpy_s(MyServerStats.Mode, 32, CurrentModeW.c_str());
		MyServerStats.Port = (WORD)Port;
		MyServerStats.MaxPlayers = Server->MaxPlayers;
		MyServerStats.Players = (byte)GetPlayerNumber();
		MyServerStats.Password = !!Server->Password[0];
	#ifdef HPC
		MyServerStats.HaloVerType = 0;
	#else
		MyServerStats.HaloVerType = 1;
	#endif
		MyServerStats.NoLead = 0;
		MyServerStats.HAC = GlobalPrefs->AntiCheat;
		MyServerStats.Version = Version;
		MyServerStats.SappVersion = CurrentVersion;
		memcpy(MyServerStats.VersionString, HaloVersionStr, 16);
		strcpy_s(MyServerStats.GameType, 32, CurrentGT.c_str());
		PacketHeader Header = {};
		Header.Size = sizeof(ServerStats);
		Header.Type = 1;
		Header.SubType = 3;		
		sendDataWithHeader(Header, &MyServerStats, sizeof(ServerStats));
	}
}

void NetworkClient::DownloadMap(DWORD ID)
{
	downloading = true;
	selected = ID;

	PacketHeader Header;
	Header.Size = 2;
	Header.Type = 1;
	Header.SubType = 2;
	Header.Flags = 0xFFFF;
	sendDataWithHeader(Header, &QueryedMaps[ID].ID, 2);
}

void NetworkClient::DownloadMapQuery(const string& name)
{
	PacketHeader Header;
	Header.Size = name.size();
	Header.Type = 1;
	Header.SubType = 4;
	Header.Flags = 0;
	sendDataWithHeader(Header, name.c_str(), name.size());
}

void NetworkClient::ACLogin()
{
	PacketHeader Header = {};
	Header.Size = 4;
	Header.Type = 1;
	Header.SubType = 5;
	DWORD version = CurrentVersion ^ 0x736568E9;
	sendDataWithHeader(Header, &version, 4);
}

void NetworkClient::SendSmallEvent(event_type type)
{
	PacketHeader Header = {};
	Header.Type = 2;
	Header.SubType = type;
	sendHeader(Header);
}

void NetworkClient::SendBigEvent(event_type type, char* buff, size_t size)
{
	PacketHeader Header = {};
	Header.Size = size;
	Header.Type = 2;
	Header.SubType = type;
	sendDataWithHeader(Header, buff, size);
}

#ifdef HALORANK
void NetworkClient::SendHREvent(HR_Event* Event, size_t size)
{
	PacketHeader Header = {};
	Header.Type = 2;
	Header.SubType = 1;
	Header.Size = size;
	sendDataWithHeader(Header, Event, size);
}
#endif

NetworkClient GlobalSAPPServer;