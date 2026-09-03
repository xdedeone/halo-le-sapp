#pragma once

#include <map>
#include <set>

#include "Prefs.h"

extern std::set <DWORD> DoSList;
extern std::map <DWORD, JoinRequestStruct> JoinReqests;
extern std::map <DWORD, DWORD> IPs;

void InstallNetworkHook();

void SetDNS(const char* dns);

bool Getnameinfo(const char *ip, char *return_dns);