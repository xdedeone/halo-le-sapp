#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>

#include "NetworkClient.h"

#pragma comment(lib, "ws2_32.lib")

bool WSInit()
{
	WSADATA w; 

	// Fill in WSA info
	if(WSAStartup(0x0202, &w)) // there was an error    
		return false;
	else
		return true;
}

void WSShutDown()
{
	WSACleanup(); //Clean up Winsock
}

bool NetworkClient::ProcessPacket_General(const PacketHeader& Header, char* Buffer)
{
	bool ret = false;
	switch (Header.SubType)
	{
	case 0: // Ping / HB
		{
			printf("Ping: %u ms\n", GetQPC());
			break;
		}
	case 1:
		{
			printf("Client Number: %u\n", Header.Flags);
			break;
		}
	case 2:
		{
			printf("The server has been rebooted.\n");
			break;
		}
	case 3:
		{
			printf("The clients has been listed to the file.\n");
			break;
		}
	default:
		ret = true;
	}
	return ret;
}

bool NetworkClient::ProcessPacket_SappServer(const PacketHeader& Header, char* Buffer)
{
	bool ret = false;
	switch (Header.SubType)
	{
	case 0: // Get IP
		{
			IP = *(DWORD*)Buffer;
			printf("External IP Received: %hs\n", inet_ntoa(*(in_addr*)&IP));
			break;
		}
	case 1: // SAPP Version
		{
			printf("Server Version: %08X\n", *(DWORD*)Buffer);
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
	case 0: // General
		ret = ProcessPacket_General(Header, Buffer);
		break;
	case 1: // Sapp Server
		ret = ProcessPacket_SappServer(Header, Buffer);
		break;
		//case 2: // Server Query

		//	break;	
		//case 3: // Remote Console

		//	break;
		//case 4: // HAC (?)	

		//	break;
	default:
		ret = true;
	}
	return ret;
}

void NetworkClient::ClientThread()
{
	ClientThreadRunning = true;

	char* MyBuffer = NULL;

	bool ConnectionOK = true;
	while(true)
	{
		PacketHeader Header;
		int size = recv(s, (char*)&Header, 8, NULL);
		if(size == 8)
		{
			if(Header.Size) // Packet has payload
			{
				if(Header.Size > 0x0FFFFFFF) // Max 256MB
					ConnectionOK = false;
				else
				{
					MyBuffer = new char[Header.Size];
					DWORD Size = 0;
					while(Size < Header.Size)
					{
						int Recived = recv(s, MyBuffer + Size, Header.Size - Size, NULL);
						if(Recived > 0)
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

			if(ConnectionOK)
				if(ProcessPacket(Header, MyBuffer))
					ConnectionOK = false;

			delete[] MyBuffer;
			MyBuffer = NULL;
		}
		else // Corrupted Header / DC
			ConnectionOK = false;

		if(ShutDown)
			break;

		if(!ConnectionOK)
		{
			connected = false;
			closesocket(s); // Shut down socket

			printf("Global Server has been disconnected, reconnecting...\n");

			size_t Count = 15; 
			while(true)
			{
				size_t i = 0;
				while(i < Count)
				{
					Sleep(1000);

					if(ShutDown)
						break;

					i++;
				}
				if(ShutDown || Connect())
					break;

				if(Count < 900)
					Count += 15;
			}
			printf("Successfully reconnected to the Global Server.\n");
			break;
		}
	}

	if(ShutDown)
		ClientThreadRunning = false;
}

void NetworkClient::StartThread(void* ClassPointer)
{
	NetworkClient* This = (NetworkClient*) ClassPointer;
	This->ClientThread();
}

int NetworkClient::Connect()
{    
	addrinfo hints = {};

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	PADDRINFOA Address;
	if(getaddrinfo("66.55.158.28", "2501", &hints, &Address) != 0)
	{
		return 0;
	}

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket

	if(s == INVALID_SOCKET)
	{
		return 0;
	}

	//Try connect
	if(connect(s, Address->ai_addr, Address->ai_addrlen) == SOCKET_ERROR)
	{
		closesocket(s);
		freeaddrinfo(Address);
		return 0;
	}

	freeaddrinfo(Address);

	ShutDown = false;
	connected = true;

	_beginthread(NetworkClient::StartThread, 0, this);

	return 1;
}

int NetworkClient::Disconnect()
{
	connected = false;
	ShutDown = true;

	closesocket(s); //Shut down socket

	BYTE count = 50;
	do 
	{
		Sleep(100);
		count--;
	} while (ClientThreadRunning && count > 0);	

	return 0;
}

bool NetworkClient::IsConnected()
{
	return connected;
}

void NetworkClient::Ping()
{
	PacketHeader Header = {};
	send(s, (char*)&Header, 8, NULL);
	StartQPC();
}

void NetworkClient::GetClientNumber()
{
	PacketHeader Header = {};
	Header.SubType = 1;
	send(s, (char*)&Header, 8, NULL);
}

void NetworkClient::RebootServer()
{
	PacketHeader Header = {};
	Header.SubType = 2;
	Header.Size = 6;
	send(s, (char*)&Header, 8, NULL);
	send(s, "R3b00t", 6, NULL);
}

void NetworkClient::ListClients()
{
	PacketHeader Header = {};
	Header.SubType = 3;
	send(s, (char*)&Header, 8, NULL);
}

void NetworkClient::GetIP()
{
	PacketHeader Header = {};
	Header.Type = 1;
	send(s, (char*)&Header, 8, NULL);
}

void NetworkClient::VersionCheck()
{
	PacketHeader Header = {};
	Header.Type = 1;
	Header.SubType = 1;
	send(s, (char*)&Header, 8, NULL);
}

void NetworkClient::InitQPC()
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	PCFreq = DWORD(li.QuadPart)/1000;
	QueryPerformanceCounter(&li);
	QPC = li.QuadPart;
}

void NetworkClient::StartQPC()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	QPC = li.QuadPart;
}

DWORD NetworkClient::GetQPC()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return DWORD(li.QuadPart - QPC)/PCFreq;
}

NetworkClient::NetworkClient() : s(0), connected(0), ShutDown(0), ClientThreadRunning(0), IP(0)
{
	InitQPC();
}