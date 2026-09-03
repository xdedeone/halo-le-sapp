#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>

#include "CustomLoad.h"
#include "NetworkClient.h"

#pragma comment(lib, "ws2_32.lib")

const char* addresses[] = { "66.55.158.28", "halo.isimaginary.com", "xhalo.tk", "sapp.haloanticheat.com" };

void WSInit()
{
	WSADATA w;
	WSAStartup(0x0202, &w);
}

void WSShutDown()
{
	WSACleanup(); //Clean up Winsock
}

//bool NetworkClient::ProcessPacket_General(const PacketHeader& Header, char* Buffer)
//{
//	bool ret = false;
//	switch (Header.SubType)
//	{
//	case 0: // Ping / HB
//		{
//			break;
//			}
//	case 1: // Broadcasted Message
//		{
//			break;
//			}
//	default:
//		ret = true;
//	}
//	return ret;
//}
//
//bool NetworkClient::ProcessPacket_SappServer(const PacketHeader& Header, char* Buffer)
//{
//	bool ret = false;
//	switch (Header.SubType)
//	{
//	case 0: // Get IP
//		{
//			if (Buffer != NULL)
//			{
//				MyIP = *(DWORD*)Buffer;
//			}
//			break;
//			}
//	case 1: // Version
//		{
//			if (Buffer != NULL && *(DWORD*)Buffer > CurrentVersion)
//			{
//				
//			}
//			break;
//			}
//	case 2: // Get Files
//		{
//				
//			break;
//		}
//	case 3: // Stats OK
//		{
//
//			break;
//			}
//	case 4: // Get Map Query response
//		{
//
//			break;
//			}
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
	case event_ping:
		{
			break;
		}
	case event_get_rc6: // Get RC6 Key and IV
		{		
			if (Buffer != NULL)
			{
				headerCrypt.rc6_key_setup((unsigned char*)Buffer, (unsigned char*)Buffer + 32);
				myCrypt.rc6_key_setup((unsigned char*)Buffer, (unsigned char*)Buffer + 32);		
				Crypted = true;
				EndWait();
			}
			else
			{
				ret = true;
			}				
			break;
		}
	case event_login: // Get LoginSessionKey
		{
			switch (Header.Flags)
			{
			case 0:
				if (Buffer != NULL)
				{
					LoginSessionKey = *(DWORD*)Buffer;
					SessionState = success;
				}			
				else
					SessionState = invalid_user;
				break;
			case 1:
				SessionState = invalid_user;
				break;
			case 2:
				SessionState = not_activated;
				break;
			case 3:			
				SessionState = banned;
				break;
			default:
				SessionState = invalid_user;
				break;
			}
			EndWait();
			break;
		}
	case event_setup_cortana: // Setup Cortana
		{
			if (Buffer != NULL)
			{
				size_t image_size = *(size_t*)Buffer;

				unsigned char* code = (unsigned char*)VirtualAlloc(NULL, image_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

				sendData(&code, 4);
			}
			else
			{
				ret = true;
			}		
			break;
		}
	case event_load_cortana: // Get Cortana
		{
			if (Buffer != NULL)
			{
				NetworkCoreStruct* core = new NetworkCoreStruct;
				core->s = s;
				core->Key = LoginSessionKey;

				if (!MemoryLoadLibrary(Buffer, (LPVOID)core))
					ret = true;

				delete core;

				ShutDown = true;

				EndWait();
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

bool NetworkClient::ProcessPacket(const PacketHeader& Header, char* Buffer)
{
	bool ret = false;
	switch (Header.Type)
	{
	//case 0: // General
	//	ret = ProcessPacket_General(Header, Buffer);
	//	break;
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
			if (Crypted)
				myCrypt.rc6_cbc_decrypt((ULONGLONG*)MyBuffer, Header.Size);
			if (ConnectionOK && ProcessPacket(Header, MyBuffer))
				ConnectionOK = false;	

			delete[] MyBuffer;
			MyBuffer = NULL;
		}
		else // Corrupted Header / DC
			ConnectionOK = false;
		
		if (ShutDown)
			break;

		if (!ConnectionOK)
		{
			connected = false;
			closesocket(s); // Shut down socket	

			SessionState = connection_terminated;
			EndWait();
			break;
		}
	}
}

unsigned int __stdcall NetworkClient::StartThread(void* ClassPointer)
{
	NetworkClient* This = (NetworkClient*)ClassPointer;
	This->ClientThread();
	return 0;
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

bool NetworkClient::Connect(const char* addr)
{
	addrinfo hints = {};

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	PADDRINFOA Address;
	if (getaddrinfo(addr, "2501", &hints, &Address) != 0)
	{
		return false;
	}

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket

	if (s == INVALID_SOCKET)
	{
		return false;
	}

	//Try connect
	if (connect(s, Address->ai_addr, Address->ai_addrlen) == SOCKET_ERROR)
	{
		closesocket(s);
		freeaddrinfo(Address);
		return false;
	}

	freeaddrinfo(Address);

	ShutDown = false;
	connected = true;

	myThread = (HANDLE)_beginthreadex(0, 0, NetworkClient::StartThread, this, 0, 0);

	return true;
}

bool NetworkClient::Connect()
{
	for (size_t i = 0; i < sizeof(addresses) / sizeof(char*); i++)
	{
		if (Connect(addresses[i]))
			return true;
	}
	return false;
}

void NetworkClient::Disconnect()
{
	connected = false;
	ShutDown = true;

	closesocket(s); //Shut down socket

	WaitForSingleObject(myThread, 2000);
}

bool NetworkClient::IsConnected()
{
	return connected;
}

extern byte halo;
LoginState NetworkClient::GetRC6()
{
	PacketHeader Header = {};
	Header.Type = 2;
	Header.SubType = 1;

	sendData(&Header, 8);

	if (StartWait(10000))
		SessionState = timeout;

	return SessionState;
}
LoginState NetworkClient::Login(loginfoStr &LogInfo)
{
	PacketHeader Header = {};
	Header.Size = 64;
	Header.Type = 2;
	Header.SubType = 2;
	headerCrypt.rc6_block_encrypt((WORD*)&Header, (WORD*)&Header);
	myCrypt.rc6_cbc_encrypt((ULONGLONG*)&LogInfo, 64);
	sendData(&Header, 8);
	sendData(&LogInfo, 64);

	memset(&LogInfo, 0, sizeof(loginfoStr));

	if (StartWait(10000))
		SessionState = timeout;

	return SessionState;
}
LoginState NetworkClient::GetCortana()
{
	//__asm
	//{
	//	mov ecx, dword ptr fs : [0x18]
	//	mov ecx, dword ptr ds : [ecx + 0x30]
	//	cmp byte ptr ds : [ecx + 2], 0
	//	sete al
	//	movzx eax, al
	//	mov ecx, [this]
	//	mov dword ptr ds : [ecx + SessionState], eax
	//}

	PacketHeader Header = {};
	Header.Size = 0;
	Header.Type = 2;
	Header.SubType = 3;
	Header.Flags = halo;
	headerCrypt.rc6_block_encrypt((WORD*)&Header, (WORD*)&Header);
	sendData(&Header, 8);

	if (StartWait(60000))
		SessionState = timeout;

	return SessionState;
}