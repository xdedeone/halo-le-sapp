#pragma once

#include <sstream>
#include <string>
#include <vector>

struct IPRange
{
	unsigned long IP;
	unsigned long Mask;
	unsigned long MaskRange;
	IPRange(unsigned long UserIP, unsigned long Mask, unsigned long MaskRange) : IP(UserIP), Mask(Mask), MaskRange(MaskRange) {}
};

struct UserItem {
	size_t UserID;
	int PrivilegeLevel;
	std::wstring UserName;
	std::string hash;
	std::vector <IPRange> AllowedIPRanges;
	UserItem(size_t UserID, int PrivilegeLevel, const std::wstring &UserName, const std::string &hash) : UserID(UserID), PrivilegeLevel(PrivilegeLevel), UserName(UserName), hash(hash) {}
};

class UserList {
	std::vector <UserItem> Users;
	std::wstring users_file;
public:
	int GetPlayerPrivilegeLevel(size_t PlayerIndex);
	bool AddUser(size_t PlayerIndex, int PrivilegeLevel, size_t &UserID);
	bool UserAddIP(size_t UserID, const std::string& MaskedIP);
	bool UserChangeLevel(size_t UserID, int lvl);
	int UserGetLevel(size_t UserID);
	bool RemoveUser(size_t UserID);
	void LoadUsersFile();
	void SaveUsersFile();
	void Clear();
	std::wostringstream StreamOut();
};

extern UserList GlobalUserList;