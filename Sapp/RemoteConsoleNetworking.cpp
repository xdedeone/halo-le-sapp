#include "RemoteConsoleNetworking.h"

#include <iomanip>

#include "Addresses.h"
#include "LogThread.h"
#include "MiscellaneousFunctions.h"
#include "TextUtilities.h"
#include "Threads.h"

using namespace std;

bool RemoteClient::SendData(const string& data)
{
	Lock_Object guard(SendLock);

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

void RemoteClient::CloseSocket()
{
	closesocket(s);
	connected = false;
}

void RemoteConsoleNetworking::SendDataOne(const string& text, size_t ClientIndex)
{
	clients[ClientIndex]->SendData(text);
}

void RemoteConsoleNetworking::SendDataAll(const string& text)
{
	for (auto& it : clients)
	{
		if (it->connected && it->level > -1)
		{
			it->SendData(text);
		}
	}
}

void RemoteConsoleNetworking::CloseSocket(size_t ClientIndex)
{
	clients[ClientIndex]->CloseSocket();

	if (ClientIndex == clients.size() - 1)
	{
		auto& it = clients.rbegin();
		while (it != clients.rend() && !(*it)->connected)
			++it;

		clients.erase(it.base(), clients.end());
	}

	WriteLog(L"REMOTE", FormatWArgs(L"Client disconnected, ClientIndex: %u", ClientIndex));
}

void RemoteConsoleNetworking::RemoteConsoleThreadProc(shared_ptr<work> &hThread)
{
	while (!hThread->ExitFlag)
	{
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(server, &fds);

		for (auto& it : clients)
		{
			if (it->connected)
			{
				FD_SET(it->s, &fds);
			}		
		}

		if (select(NULL, &fds, NULL, NULL, NULL) != SOCKET_ERROR)
		{
			if (FD_ISSET(server, &fds))
			{
				sockaddr_in addr;
				int len = sizeof(sockaddr_in);
				SOCKET new_client = accept(server, (sockaddr*)&addr, &len);
				if (new_client != INVALID_SOCKET)
				{
					clients.emplace_back(make_unique<RemoteClient>(new_client, addr.sin_addr.s_addr, -1));
				}
			}
			else
			{
				for (size_t i = 0; i < clients.size(); i++)
				{
					if (clients[i]->connected && FD_ISSET(clients[i]->s, &fds))
					{
						char buff[4096];
						int ret = recv(clients[i]->s, buff, 4095, NULL);
						if (ret > 0)
						{
							buff[ret] = 0;
							if (!ProcessData(i, buff, ret))
							{
								CloseSocket(i);
							}
						}
						else
						{
							CloseSocket(i);
						}
					}
				}
			}
		}
		else
		{
			Sleep(1000);
		}
	}

	for (auto& it : clients)
	{
		closesocket(it->s);
	}

	clients.clear();

	WriteLog(L"REMOTE", L"Remote Client has been stopped.");
}

void RemoteConsoleNetworking::StartThread(shared_ptr<work> &hThread)
{
	RemoteConsoleNetworking* This = static_cast<RemoteConsoleNetworking*>(hThread->GetParams());
	This->RemoteConsoleThreadProc(hThread);
}

bool RemoteConsoleNetworking::StartRemoteConsole()
{
	if (auto sp = hRemoteConsoleThread.lock())
	{
		return false;
	}
	else
	{
		SOCKADDR_IN addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(RemotePort);
		addr.sin_addr.s_addr = endian_swap(IP);

		server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (server == INVALID_SOCKET)
		{
			return false;
		}

		if (::bind(server, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR ||
			listen(server, 16) == SOCKET_ERROR)
		{
			closesocket(server);
			return false;
		}

		WriteLog(L"REMOTE", FormatWArgs(L"Remote Client has been started, listening on port %hu.", RemotePort));

		hRemoteConsoleThread = ThreadPool.NewWork(&RemoteConsoleNetworking::StartThread, this);
		return true;
	}
}

bool RemoteConsoleNetworking::StopRemoteConsole()
{
	if (auto sp = hRemoteConsoleThread.lock())
	{
		sp->ExitFlag = true;
		closesocket(server);
		return true;
	}
	else
	{
		return false;
	}
}

std::wostringstream RemoteConsoleNetworking::StreamOutRemoteClients()
{
	std::wostringstream os;

	if (!clients.empty())
	{
		os << left << setw(4) << "ID" << setw(12) << L"Username" <<  setw(8) << "Level" << "IP"  << endl;
		for (size_t i = 0; i < clients.size(); i++)
		{
			if (clients[i]->connected)
			{
				os << left << setw(4) << i << setw(12) << clients[i]->name << setw(8) << clients[i]->level << inet_ntoa(*(in_addr*)&clients[i]->ip) << endl;
			}
		}
	}
	else
	{
		os << L"There are no remote clients connected." << endl;
	}

	return os;
}