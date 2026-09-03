#include <ws2tcpip.h>

#include "NetworkHook.h"

#include "Addresses.h"
#include "bans.h"
#include "LogThread.h"
#include "Patcher.h"
#include "TextUtilities.h"

std::map <DWORD, JoinRequestStruct> JoinReqests;
std::map <DWORD, DWORD> IPs;
std::set <DWORD> DoSList;

int __stdcall RecvfromEnd(int ret, const char* buf, const sockaddr* from)
{
	if (ret <= 0)
	{
		if (ret == SOCKET_ERROR && WSAGetLastError() == WSAENETRESET)
		{
			WSASetLastError(WSAECONNRESET);
		}
		return ret;
	}

	DWORD client_IP = *(DWORD*)&((sockaddr_in*)from)->sin_addr;

	if (DoSList.find(client_IP) != DoSList.end())
	{
		WSASetLastError(WSAECONNRESET);
		return SOCKET_ERROR;
	}

	if (GlobalPrefs->PacketLimit && ++IPs[client_IP] > GlobalPrefs->PacketLimit)
	{
		DoSList.emplace(client_IP);
		GlobalIPBanlist.BanIP(client_IP, 0xFFFFFFFF, 32, time(0) + 172800, L"<unknown>", L"DOS_ATTACK");
		WriteLog(L"IPBAN", FormatWArgs(L"IP %hs has been banned because the server received more than %u packets in less than 1 second. (DOS_ATTACK)", inet_ntoa(*(in_addr*)&client_IP), GlobalPrefs->PacketLimit));
		WSASetLastError(WSAECONNRESET);
		return SOCKET_ERROR;
	}

	bool block = false;

	bool IsServerQuery = (*(unsigned short*)buf == 0xfdfe) || (*(unsigned char*)(buf) == 0x5C);
	bool IsJoinAttempt = (*(unsigned long*)buf == 0x0001fefe);

	stats::JoinAttempts += IsJoinAttempt;
	stats::ServerQueries += IsServerQuery;

	if (IsJoinAttempt || IsServerQuery || GlobalPrefs->FullIPBan)
	{
		if (GlobalIPBanlist.IsIPBanned(client_IP))
			block = true;
		else if (IsJoinAttempt && GlobalPrefs->AntiHaloFP)
		{
			JoinRequestStruct& thisRequest = JoinReqests[client_IP];
			time_t TmpTime = time(0);
			if (TmpTime - thisRequest.last_attempt < 4)
			{
				thisRequest.total_requests++;
				if (thisRequest.total_requests > 8)
				{
					GlobalIPBanlist.BanIP(client_IP, 0xFFFFFFFF, 32, time(0) + 300, L"<unknown>", L"HALOFP");
					WriteLog(L"IPBAN", FormatWArgs(L"IP %hs has been banned. (HALOFP)", inet_ntoa(*(in_addr*)&client_IP)));
					block = true;
				}
			}
			else
				thisRequest.total_requests = 1;

			thisRequest.last_attempt = TmpTime;
		}
	}

	if (!block)
	{
		return ret;
	}
	else
	{
		WSASetLastError(WSAECONNRESET);
		return SOCKET_ERROR;
	}
}

void __declspec(naked) RecvfromEndCC()
{
	__asm
	{
		lea edx, [esp + 0x10]
		push edx
		lea ecx, [esp + 0x24]
		push ecx
		push eax
		call RecvfromEnd
		mov esi, eax
		cmp esi, -1
		ret
	}
}

void InstallNetworkHook()
{
	Patches.NewPatch(new CodeCaveHook("recvfrom Hook", recvfromEndSign, 5, 0, NULL, RecvfromEndCC, true));
}

void SetDNS(const char* dns)
{
	if (*ptrMSData != NULL)
	{
		PADDRINFOA Address;
		if (!getaddrinfo(dns, NULL, NULL, &Address))
		{
			*(DWORD*)(*ptrMSData + 0xC8) = *(DWORD*)&((sockaddr_in*)Address->ai_addr)->sin_addr;
			freeaddrinfo(Address);
		}
	}
}

bool Getnameinfo(const char *ip, char *return_dns)
{
	sockaddr_in saGNI = {};
	char servInfo[NI_MAXSERV];
	saGNI.sin_family = AF_INET;
	saGNI.sin_addr.s_addr = inet_addr(ip);
	return !getnameinfo((struct sockaddr *) &saGNI, sizeof(struct sockaddr), return_dns, NI_MAXHOST, servInfo, NI_MAXSERV, NI_NAMEREQD);
}