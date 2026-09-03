#include "NetworkClient.h"

#include "Addresses.h"
#include "MiscFunctions.h"
#include "SendChatMessage.h"

#pragma comment(lib, "ws2_32.lib")

//unsigned char addr1[] = { 0xc9, 0xc9, 0xd1, 0xca, 0xca, 0xd1, 0xce, 0xca, 0xc7, 0xd1, 0xcd, 0xc7, 0xff };
//unsigned char addr2[] = { 0x87, 0x97, 0x9e, 0x93, 0x90, 0xd1, 0x8b, 0x94, 0xff };
//unsigned char addr3[] = { 0x8c, 0x9e, 0x8f, 0x8f, 0xd1, 0x97, 0x9e, 0x93, 0x90, 0x9e, 0x91, 0x8b, 0x96, 0x9c, 0x97, 0x9a, 0x9e, 0x8b, 0xd1, 0x9c, 0x90, 0x92, 0xff };
//unsigned char port_str[] = { 0xcd, 0xca, 0xcf, 0xce, 0xff };
//unsigned char* addresses[] = { addr1, addr2, addr3 };

const wchar_t* const Files[] = { L"strings_updater.dll", L"strings.dat" };
bool FilesReceived[2] = {};

//void GetFile(const PacketHeader &Header, char* Buffer)
//{
//	WORD Flags = Header.Flags - 3;
//	if (Flags < 2)
//	{
//		FILE* File;
//		if (!_wfopen_s(&File, Files[Flags], L"wb") && File)
//		{
//			fwrite(Buffer, Header.Size, 1, File);
//			fclose(File);
//			FilesReceived[Flags] = true;
//			if (FilesReceived[0] && FilesReceived[1])
//			{
//				HMODULE hModule = LoadLibraryW(Files[0]);
//				if (hModule != NULL)
//				{
//					FreeLibrary(hModule);
//					char hash[32];
//					SHA256_File("strings.dll", (unsigned char*)hash);
//					myClient->SendBigEvent(event_update_succeed, hash, 32);
//				}
//				else
//				{
//					myClient->SendSmallEvent(event_update_failed);
//				}
//			}
//		}
//	}
//}

bool NetworkClient::ProcessPacket_General(const PacketHeader& Header, char* Buffer)
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
		ENCRYPT_BEGIN
		if (Header.Flags == 2501 && Buffer != NULL)
		{
			SendChatMessage((wchar_t*)Buffer);
		}
		ENCRYPT_END
		break;
	}
	default:
		ret = true;
	}
	return ret;
}

//bool NetworkClient::ProcessPacket_SappServer(const PacketHeader &Header, char* Buffer)
//{
//	bool ret = false;
//	switch (Header.SubType)
//	{
//	case 2: // Get Files
//	{
//		ENCRYPT_BEGIN
//		GetFile(Header, Buffer);
//		ENCRYPT_END
//		break;
//	}
//	default:
//		ret = true;
//	}	
//	return ret;
//}

bool NetworkClient::ProcessPacket_HAC(const PacketHeader& Header, char* Buffer)
{
	bool ret = false;
	switch (Header.SubType)
	{
	case event_ping: // Ping
	{
		DeathClock = 30;
		break;
	}
	case event_get_rc6: // Get RC6 Key and IV
	{		
		if (Buffer != NULL)
		{		
			ENCRYPT_BEGIN
			headerCrypt.rc6_key_setup((unsigned char*)Buffer, (unsigned char*)Buffer + 32);
			myCrypt.rc6_key_setup((unsigned char*)Buffer, (unsigned char*)Buffer + 32);
			Crypted = true;
			EndWait();
			ENCRYPT_END
		}
		else
		{
			ret = true;
		}	
		break;
	}
	case event_map_checksum:
	{
		ENCRYPT_BEGIN
		SappServer = true;
		if (Header.Flags == 2501)
		{
			ACServer = true;
		}
		else
		{
			ACServer = false;
		}
		ENCRYPT_END
		break;
	}
	case event_join_client:
	{
		if (Buffer != NULL)
		{
			ENCRYPT_BEGIN
			SendChatMessage((wchar_t*)Buffer);
			ENCRYPT_END
		}
		else
		{
			ret = true;
		}
		break;
	}
	case event_join_server:
	{
		if (Header.Size >= sizeof(AnticheatPlayerPacket))
		{
			ENCRYPT_BEGIN
			AnticheatPlayerPacket* Packet = (AnticheatPlayerPacket*)Buffer;
			if (Packet->PlayerIndex < 16)
			{
				memcpy(ACPlayers[Packet->PlayerIndex], Packet->AnticheatName, 32);
			}
			ENCRYPT_END
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
			ENCRYPT_BEGIN
			if (*(DWORD*)Buffer < 16)
			{
				memcpy(ACPlayers[*(DWORD*)Buffer], "---", 4);
			}
			ENCRYPT_END
		}
		else
		{
			ret = true;
		}
		break;
	}
	case event_alias:
	{
		ENCRYPT_BEGIN
		if (Header.Flags)
		{
			PrintConsole("The alias has been successfully set for you!", green);
		}
		else
		{
			PrintConsole("This alias is invalid or already taken by someone!", red);
		}
		ENCRYPT_END
		break;
	}
	default:
		ret = true;
	}
	return ret;
}

bool __declspec(noinline) NetworkClient::ProcessPacket(const PacketHeader& Header, char* Buffer)
{
	bool ret = false;
	switch (Header.Type)
	{
	case 0: // General
		ret = ProcessPacket_General(Header, Buffer);
		break;
	//case 1: // Sapp Server
	//	ret = ProcessPacket_SappServer(Header, Buffer);
	//	break;
	case 2: // HAC
		ret = ProcessPacket_HAC(Header, Buffer);
		break;
		//case 3: // Server Query

		//	break;	
		//case 4: // Remote Console

		//	break;
	default:
		ret = true;
	}
	return ret;
}

void NetworkClient::ClientThread()
{
	char* MyBuffer = NULL;

	bool ConnectionOK = true;
	while (true)
	{
		PacketHeader Header;
		int size = recv(s, (char*)&Header, 8, NULL);
		if (size == 8)
		{
			ENCRYPT_BEGIN
			if (Crypted)
				headerCrypt.rc6_block_decrypt((WORD*)&Header, (WORD*)&Header);
			if (Header.Size) // Packet has payload
			{
				if (Header.Size > 0x0FFFFFFF) // Max 256MB
					ConnectionOK = false;
				else
				{
					MyBuffer = new char[Header.Size];
					DWORD Size = 0;
					while (Size < Header.Size)
					{
						int Recived = recv(s, MyBuffer + Size, Header.Size - Size, NULL);
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
			
			if (ConnectionOK)
			{
				if (Crypted)
					myCrypt.rc6_cbc_decrypt((ULONGLONG*)MyBuffer, Header.Size);
				if (ProcessPacket(Header, MyBuffer))
					ConnectionOK = false;
			}
			
			delete[] MyBuffer;
			MyBuffer = NULL;
			ENCRYPT_END
		}
		else // Corrupted Header / DC
			ConnectionOK = false;

		if (!ConnectionOK)
			break;
	}	
}

void NetworkClient::StartThread(void* ClassPointer)
{
	ENCRYPT_BEGIN
	NetworkClient* This = (NetworkClient*)ClassPointer;
	This->ClientThread();
	This->Disconnect();
	delete This;
	if (NullMe != NULL)
		*NullMe = NULL;
	ENCRYPT_END
	ExitProcess(0);
}

bool NetworkClient::StartWait(DWORD time)
{
	if (WaitForSingleObject(mySemaphore, time) == WAIT_TIMEOUT)
		return true;

	return false;
}

void NetworkClient::EndWait()
{
	ReleaseSemaphore(mySemaphore, 1, 0);
}

bool NetworkClient::Start()
{
	ENCRYPT_BEGIN
	_beginthread(NetworkClient::StartThread, 0, this);
	PacketHeader Header = {};
	Header.Type = 2;
	Header.SubType = event_get_rc6;
	sendData(&Header, 8);
	ENCRYPT_END
	bool ret = !StartWait(10000);
	if (ret)
	{
		GetHashesAndMACs();
	}
	_beginthread(DeathClockThread, 0, 0);
	return ret;
}

bool NetworkClient::sendData(void* data, int len)
{
	char *ptr = (char*)data;

	while (len > 0)
	{
		int sent = send(s, ptr, len, 0);
		if (sent <= 0)
		{
			Disconnect();
			return false;
		}
		ptr += sent;
		len -= sent;
	}

	return true;
}

//bool NetworkClient::Connect(const char* addr)
//{
//	addrinfo hints = {};
//
//	hints.ai_family = AF_INET;
//	hints.ai_socktype = SOCK_STREAM;
//	hints.ai_protocol = IPPROTO_TCP;
//
//	PADDRINFOA Address;
//	if (getaddrinfo(addr, CryptExtractor(StringCrypt(port_str, sizeof(port_str))), &hints, &Address) != 0)
//	{
//		return false;
//	}
//
//	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket
//
//	if (s == INVALID_SOCKET)
//	{
//		return false;
//	}
//
//	//Try connect
//	if (connect(s, Address->ai_addr, Address->ai_addrlen) == SOCKET_ERROR)
//	{
//		closesocket(s);
//		freeaddrinfo(Address);
//		return false;
//	}
//
//	freeaddrinfo(Address);
//
//	Start();
//
//	return true;
//}

//bool NetworkClient::Connect()
//{
//	for (size_t i = 0; i < sizeof(addresses) / sizeof(char*); i++)
//	{
//		if (Connect(CryptExtractor(StringCrypt(addresses[i], sizeof(addresses[i])))) && GetRC6())
//			return true;
//	}
//	return false;
//}

void __declspec(noinline) NetworkClient::Disconnect()
{
	ENCRYPT_BEGIN
	closesocket(s); //Shut down socket
	WSACleanup();
	ENCRYPT_END
}

void __declspec(noinline) NetworkClient::SendSmallEvent(event_type type, WORD flags)
{
	sLock.Lock();
	ENCRYPT_BEGIN
	PacketHeader Header;
	Header.Size = 0;
	Header.Type = 2;
	Header.SubType = type;
	Header.Flags = flags;
	headerCrypt.rc6_block_encrypt((WORD*)&Header, (WORD*)&Header);	
	sendData(&Header, 8);
	KillMe = (type >= event_debugger_attached && type <= event_wrong_checksum);
	ENCRYPT_END
	sLock.Unlock();
}

void __declspec(noinline) NetworkClient::SendBigEvent(event_type type, char* buff, size_t size)
{
	sLock.Lock();
	ENCRYPT_BEGIN
	PacketHeader Header;
	Header.Size = size;
	Header.Type = 2;
	Header.SubType = type;
	Header.Flags = 0;
	headerCrypt.rc6_block_encrypt((WORD*)&Header, (WORD*)&Header);
	myCrypt.rc6_cbc_encrypt((ULONGLONG*)buff, size);
	sendData(&Header, 8);
	sendData(buff, size);
	ENCRYPT_END
	sLock.Unlock();
}

NetworkClient* myClient = NULL;