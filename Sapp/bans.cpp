#include "bans.h"

#include <iomanip>
#include <time.h>

#include "Addresses.h"
#include "HaloUtilities.h"
#include "MiscellaneousFunctions.h"
#include "TextUtilities.h"

using namespace std;

void IPBanlistClass::BanIP(unsigned long ip, unsigned long mask, unsigned long mask_range, time_t end_time, wstring &&name, wstring &&reason)
{
	for (auto& it = ipbans.begin(); it != ipbans.end(); ++it)
	{
		if ((it->ip & it->mask) == (ip & it->mask))
		{
			if (it->mask >= mask)
				return;
			else
				ipbans.erase(it);
			break;
		}
	}

	ipbans.emplace_back(++ban_id, end_time, move(name), ip, mask, mask_range, move(reason));
}

void IPBanlistClass::Refresh()
{
	time_t Time = time(0);
	for (auto& it = ipbans.begin(); it != ipbans.end();)
	{
		if (it->end_time != 0 && it->end_time < Time)
		{
			it = ipbans.erase(it);
		}
		else ++it;
	}
}

bool IPBanlistClass::IsIPBanned(unsigned long ip)
{
	for (auto& it = ipbans.begin(); it != ipbans.end(); ++it)
	{
		if((it->ip & it->mask) == (ip & it->mask))
		{
			if (it->end_time == 0 || it->end_time > time(0))
			{
				return true;
			}
			else
			{
				ipbans.erase(it);
				return false;
			}
		}
	}

	return false;
}

unsigned long IPBanlistClass::UnbanIP(size_t Index)
{
	unsigned long ret = 0;

	for (auto& it = ipbans.begin(); it != ipbans.end(); ++it)
	{
		if (it->ban_id == Index)
		{
			ret = it->ip;
			ipbans.erase(it);
			break;
		}
	}

	return ret;
}

void IPBanlistClass::LoadIPBans()
{
	ipbans_file = L"sapp\\ipbans.txt";
	crFile(ipbans_file);

	FILE *IPBansFile;
	_wfopen_s(&IPBansFile, ipbans_file.c_str(), L"r, ccs=UTF-16LE");

	if(!IPBansFile)
	{
		PrintConsole(12, "Failed to open IPBans file!");
		return;
	}

	wchar_t buff[4096];
	while(fgetws(buff, 4096, IPBansFile))
	{
		size_t n;
		wstring line = buff;
		
		if((n = line.find_first_not_of(L" \t")) != wstring::npos)
		{
			if(line[n] == ';')
				continue;
		}
		vector <wstring> line_items;
		if (!Split(line_items, line, L':') || line_items.size() < 3)
			continue;

		for(size_t i = 0; i < line_items.size(); i++)
			TrimSpaces(line_items[i]);

		size_t ip;
		size_t mask;
		size_t mask_range;
		if(!ParseIP(WToStr(line_items[1]), ip, mask, mask_range))
			continue;

		time_t end_time;
		if (line_items.size() < 4 || !ToNumber(line_items[3], end_time))
			end_time = 0;
		else if (end_time != 0 && end_time < time(0))
			continue;

		BanIP(ip, mask, mask_range, end_time, move(line_items[0]), move(line_items[2]));
	}

	fclose(IPBansFile);
}

void IPBanlistClass::SaveIPBans()
{
	FILE *IPBansFile;
	_wfopen_s(&IPBansFile, ipbans_file.c_str(), L"w, ccs=UTF-16LE");

	if(!IPBansFile)
	{
		PrintConsole(12, "Failed to open IPBans file!");
		return;
	}

	for (auto& it = ipbans.begin(); it != ipbans.end(); ++it)
	{
		fwprintf_s(IPBansFile, L"%ls:%hs/%u:%ls:%lld\n", it->name.c_str(), inet_ntoa(*(in_addr*)(&it->ip)), it->mask_range, it->ban_reason.c_str(), it->end_time);
	}

	fclose(IPBansFile);
}

void IPBanlistClass::Clear()
{
	ipbans.clear();
}

wostringstream IPBanlistClass::StreamOut()
{
	wostringstream os;
	if(ipbans.empty())
	{
		os << L"There are no IP bans." << endl;
	}
	else
	{
		os << setw(4)  << L"[ID]" << setw(12) << L"[NAME]" << setw(21) << L"[IP]" << L"   [REASON]" << endl;
		for(auto& it = ipbans.begin(); it != ipbans.end(); ++it)
		{
			os << setw(4) << it->ban_id << setw(12) << it->name << setw(18) << inet_ntoa(*(in_addr*)&it->ip) << L"/" << it->mask_range;
			if(it->mask_range > 9)
				os << L"   ";
			else
				os << L"    ";
			os << it->ban_reason << endl;
		}
	}
	return os;
}

// TextBan Class implementation; using CD-Key
void TextBanList::Ban(int PlayerIndex, time_t period)
{
	if(IsBanned(PlayerIndex))
		return;

	int PlayerNumber = IndexToNumber(PlayerIndex);

	string hash = GetPlayerHash(PlayerNumber);
	ToLower(hash);
	
	textbans.emplace(std::piecewise_construct, std::forward_as_tuple(move(hash)), std::forward_as_tuple(++ban_id, period != 0 ? time(0) + period * 60 : 0, PlayerTable[PlayerIndex].name));
}

void TextBanList::Refresh()
{
	time_t Time = time(0);
	for(auto& it = textbans.begin(); it != textbans.end();)
	{
		auto& ban_item = it->second;
		if (ban_item.end_time != 0 && ban_item.end_time < Time)
		{
			it = textbans.erase(it);
		}
		else ++it;
	}
}

bool TextBanList::IsBanned(int PlayerIndex)
{
	int PlayerNumber = IndexToNumber(PlayerIndex);
	if(PlayerNumber == -1)
		return false;

	string hash = GetPlayerHash(PlayerNumber);
	ToLower(hash);

	auto& it = textbans.find(hash);
	if (it != textbans.end())
	{
		auto& ban_item = it->second;
		if (ban_item.end_time == 0 || ban_item.end_time > time(0))
			return true;
		else
		{
			textbans.erase(it);
			return false;
		}
	}

	return false;
}

wstring TextBanList::Unban(size_t Index)
{
	wstring ret;

	for (auto& it = textbans.begin(); it != textbans.end(); ++it)
	{
		if (it->second.ban_id == Index)
		{
			ret = move(it->second.name);
			textbans.erase(it);
			break;
		}
	}

	return ret;
}

void TextBanList::Clear()
{
	textbans.clear();
}

wostringstream TextBanList::StreamOut()
{
	wostringstream os;
	if (textbans.empty())
	{
		os << "There are no textbans." << endl;
	}	
	else
	{
		for (auto& it = textbans.begin(); it != textbans.end(); ++it)
		{
			os << setw(4) << it->second.ban_id << setw(12) << it->second.name << L" | " << FormatWArgs(L"%hs", it->first.c_str()) << endl;
		}
	}
	return os;
}

// Mute Class implementation: using IP
void MuteList::Ban(int PlayerIndex, time_t period)
{
	if(IsBanned(PlayerIndex))
		return;

	int PlayerNumber = IndexToNumber(PlayerIndex);

	NetInfoStruct* info = **Server->MachineInfo[PlayerNumber].IPAddress;
	if (info != NULL)
	{
		mutes.emplace(std::piecewise_construct, std::forward_as_tuple(info->IP), std::forward_as_tuple(++ban_id, period != 0 ? time(0) + period * 60 : 0, PlayerTable[PlayerIndex].name));
	}
}

void MuteList::Refresh()
{
	time_t Time = time(0);
	for (auto& it = mutes.begin(); it != mutes.end();)
	{
		auto& ban_item = it->second;
		if (ban_item.end_time != 0 && ban_item.end_time < Time)
		{
			it = mutes.erase(it);
		}
		else ++it;
	}
}

bool MuteList::IsBanned(int PlayerIndex)
{
	int PlayerNumber = IndexToNumber(PlayerIndex);
	if(PlayerNumber == -1)
		return false;

	NetInfoStruct* info = **Server->MachineInfo[PlayerNumber].IPAddress;
	if (info != NULL)
	{
		auto& it = mutes.find(info->IP);
		if (it != mutes.end())
		{
			auto& ban_item = it->second;
			if (ban_item.end_time == 0 || ban_item.end_time > time(0))
				return true;
			else
			{
				mutes.erase(it);
				return false;
			}
		}
	}
	
	return false;
}

wstring MuteList::Unban(size_t Index)
{
	wstring ret;

	for (auto& it = mutes.begin(); it != mutes.end(); ++it)
	{
		if (it->second.ban_id == Index)
		{
			ret = move(it->second.name);
			mutes.erase(it);
			break;
		}
	}

	return ret;
}

void MuteList::Clear()
{
	mutes.clear();
}

wostringstream MuteList::StreamOut()
{
	wostringstream os;
	if (mutes.empty())
	{
		os << L"There are no mutes." << endl;
	}
	else
	{
		for (auto& it = mutes.begin(); it != mutes.end(); ++it)
		{
			os << setw(4) << it->second.ban_id << setw(12) << it->second.name << L" | " << inet_ntoa(*(in_addr*)&it->first) << endl;
		}
	}
	return os;
}

IPBanlistClass GlobalIPBanlist;
TextBanList GlobalTextBans;
MuteList GlobalMutedPlayers;