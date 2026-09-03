#include "NetworkClient.h"

#include <process.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

SOCKET s;

bool SendData(const string& data)
{
	const char* ptr = data.c_str();
	size_t len = data.size();
	
	while (len > 0)
	{
		int sent = send(s, ptr, len, 0);
		if (sent <= 0)
		{
			closesocket(s);
			return false;
		}
		ptr += sent;
		len -= sent;
	}

	return true;
}

void ProcessData(const char* Message, size_t Length)
{
	printf(Message);
}

unsigned __stdcall ClientThreadProc(void*)
{
	while (true)
	{
		char Buffer[4096] = {};
		int Recived = recv(s, Buffer, 4096, NULL);
		if (Recived > 0)
		{
			ProcessData(Buffer, Recived);
		}
		else
		{
			break;
		}
	}

	printf("Connection terminated!\n");
	return 0;
}

bool Connect(const char* addr)
{
	WSADATA w;
	WSAStartup(0x0202, &w);

	addrinfo hints = {};

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	PADDRINFOA Address;
	if (getaddrinfo(addr, "2501", &hints, &Address) != 0)
	{
		return false;
	}

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
	{
		freeaddrinfo(Address);
		return false;
	}

	if (connect(s, Address->ai_addr, Address->ai_addrlen) == SOCKET_ERROR)
	{
		closesocket(s);
		freeaddrinfo(Address);
		return false;
	}

	freeaddrinfo(Address);

	_beginthreadex(0, 0, ClientThreadProc, 0, 0, 0);

	return true;
}