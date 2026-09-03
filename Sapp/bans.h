#pragma once

#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

struct BanItem {
	size_t ban_id;
	time_t end_time;
	std::wstring name;
	BanItem(size_t ban_id, time_t end_time, std::wstring&& name)
		: ban_id(ban_id), end_time(end_time), name(std::move(name)) {}
};

struct IPBanItem : public BanItem {
	unsigned long ip;
	unsigned long mask;
	unsigned long mask_range;
	std::wstring ban_reason;
	IPBanItem(size_t ban_id, time_t end_time, std::wstring&& name, unsigned long ip, unsigned long mask, unsigned long mask_range, std::wstring&& ban_reason)
		: BanItem(ban_id, end_time, std::move(name)), ip(ip), mask(mask), mask_range(mask_range), ban_reason(std::move(ban_reason)) {}
};

class IPBanlistClass {
	std::vector <IPBanItem> ipbans;
	std::wstring ipbans_file;
	size_t ban_id = 0;
public:
	std::string LastError;
	void BanIP(unsigned long ip, unsigned long mask, unsigned long mask_range, time_t end_time, std::wstring &&name, std::wstring &&reason);
	unsigned long UnbanIP(size_t ban_id);
	void Refresh();
	bool IsIPBanned(unsigned long ip);
	void LoadIPBans();
	void SaveIPBans();
	void Clear();
	std::wostringstream StreamOut();
};

class TextBanList {
	std::map <std::string, BanItem> textbans;
	size_t ban_id = 0;
public:
	void Ban(int PlayerIndex, time_t period);
	std::wstring Unban(size_t Index);
	void Refresh();
	bool IsBanned(int PlayerIndex);
	void Clear();
	std::wostringstream StreamOut();
};

class MuteList {
	std::map <unsigned long, BanItem> mutes;
	size_t ban_id = 0;
public:
	void Ban(int PlayerIndex, time_t period);
	std::wstring Unban(size_t Index);
	void Refresh();
	bool IsBanned(int PlayerIndex);
	void Clear();
	std::wostringstream StreamOut();
};

extern IPBanlistClass GlobalIPBanlist;
extern TextBanList GlobalTextBans;
extern MuteList GlobalMutedPlayers;