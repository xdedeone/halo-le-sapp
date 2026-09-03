#pragma once

#include <sstream>
#include <string>
#include <vector>

struct AdminItem {
	std::wstring name;
	std::string pwhash;
	int lvl;
	AdminItem(const std::wstring &name, const std::string &pwhash, int lvl) : name(name), pwhash(pwhash), lvl(lvl) {}
};

class AdminList {
	std::vector <AdminItem> Admins;
	std::wstring admins_file;
public:
	std::string LastError;
	bool AddAdmin(std::wstring &&name, std::string &&password, int lvl);
	int IsAdmin(const std::wstring &name, const std::string &password);
	int IsRconAdmin(const std::wstring &name);
	int GetAdminID(const std::wstring &name);
	bool ChangePassword(size_t Index, const std::string &password);
	bool ChangeLevel(size_t Index, int lvl);
	bool RemoveAdmin(size_t Index);
	void LoadAdmins();
	void SaveAdmins();
	void Clear();
	std::wostringstream StreamOut();
};

extern AdminList GlobalAdminList;