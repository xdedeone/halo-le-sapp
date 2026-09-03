#include "UserList.h"

#include <iomanip>

#include "Addresses.h"
#include "HaloUtilities.h"
#include "MiscellaneousFunctions.h"
#include "TextUtilities.h"

using namespace std;

int UserList::GetPlayerPrivilegeLevel(size_t PlayerIndex)
{
	int PlayerNumber = IndexToNumber(PlayerIndex);
	if(PlayerNumber == -1 || !PlayerPresent(PlayerIndex))
		return -1;

	string Hash = GetPlayerHash(PlayerNumber);

	for(size_t i = 0; i < Users.size(); i++)
	{
		if (Users[i].hash == Hash)
		{
			if (Users[i].AllowedIPRanges.empty())
				return Users[i].PrivilegeLevel;
			else
			{
				string StringIP = GetPlayerIP(PlayerNumber);
				size_t colon = StringIP.find_first_of(":");
				if(colon == string::npos)
					return -1;

				StringIP.erase(StringIP.begin() + colon, StringIP.end());

				int PlayerIP = inet_addr(StringIP.c_str());

				for (size_t j = 0; j < Users[i].AllowedIPRanges.size(); j++)
				{
					IPRange range = Users[i].AllowedIPRanges[j];
					if ((range.IP & range.MaskRange) == (PlayerIP & range.MaskRange))
						return Users[i].PrivilegeLevel;
				}
				return -1;
			}
		}
	}
	return -1;
}

bool UserList::AddUser(size_t PlayerIndex, int PrivilegeLevel, size_t &UserID)
{
	int PlayerNumber = IndexToNumber(PlayerIndex);
	if(PlayerNumber == -1)
		return false;
	
	string PlayerHash = GetPlayerHash(PlayerNumber);
	for(size_t i = 0; i < Users.size(); i++)
	{
		if(Users[i].hash == PlayerHash)
			return false;
	}

	int NewUserID = 0;
	bool IdUsed = false;
	do 
	{
		IdUsed = false;
		for(size_t i = 0; i < Users.size(); i++)
		{
			if(Users[i].UserID == NewUserID)
			{
				IdUsed = true;
				NewUserID++;
				break;
			}
		}
	}
	while(IdUsed);

	UserID = NewUserID;

	Users.emplace_back(UserID, PrivilegeLevel, PlayerTable[PlayerIndex].name, move(PlayerHash));
	return true;
}
bool UserList::RemoveUser(size_t UserID)
{
	for(size_t i = 0; i < Users.size(); i++)
	{
		if (Users[i].UserID == UserID)
		{
			Users.erase(Users.begin() + i);
			return true;
		}
	}
	return false;
}
bool UserList::UserAddIP(size_t UserID, const string& MaskedIP)
{
	for(size_t i = 0; i < Users.size();i++)
	{
		if(Users[i].UserID == UserID)
		{
			size_t NewIP;
			size_t NewMask;
			size_t NewMaskRange;
			if(ParseIP(MaskedIP, NewIP, NewMask, NewMaskRange))
			{
				Users[i].AllowedIPRanges.emplace_back(NewIP, NewMask, NewMaskRange);
				return true;
			}
		}
	}
	return false;
}
bool UserList::UserChangeLevel(size_t UserID, int lvl)
{
	for(size_t i = 0; i < Users.size();i++)
	{
		if (Users[i].UserID == UserID)
		{
			Users[i].PrivilegeLevel = lvl;		
			return true;
		}
	}
	return false;
}
int UserList::UserGetLevel(size_t UserID)
{
	for(size_t i = 0; i < Users.size();i++)
	{
		if (Users[i].UserID == UserID)
		{
			return Users[i].PrivilegeLevel;
		}
	}
	return -1;
}
void UserList::LoadUsersFile()
{
	users_file = L"sapp\\users.txt";
	crFile(users_file);

	FILE *UsersFile;
	_wfopen_s(&UsersFile, users_file.c_str(), L"r, ccs=UTF-16LE");

	if(!UsersFile)
	{
		PrintConsole(12, "Failed to open Users file!");
		return;
	}

	wchar_t buff[4096];

	while(fgetws(buff, 4096, UsersFile))
	{
		size_t n;
		wstring line = buff;

		if((n = line.find_first_not_of(L" \t")) != wstring::npos)
		{
			if(line[n] == L';')
				continue;
		}
		vector <wstring> line_items;
		if (!Split(line_items, line, L':') || line_items.size() < 5)
			continue;

		for(size_t i = 0; i < line_items.size(); i++)
			TrimSpaces(line_items[i]);

		string hash = WToStr(line_items[2]);
		ToLower(hash);

		bool found = false;
		for(size_t i = 0; i < Users.size(); i++)
		if (Users[i].hash == hash)
		{
			found = true;
			break;
		}

		size_t UserID;
		size_t PrivilegeLevel;
		if(found)
			continue;
		if (!ToNumber(line_items[1], UserID))
			continue;
		if(!ToNumber(line_items[3], PrivilegeLevel))
			continue;

		if(PrivilegeLevel > 4)
			PrivilegeLevel = 4;

		Users.emplace_back(UserID, PrivilegeLevel, move(line_items[0]), move(hash));

		if (!line_items[4].empty())
		{
			vector <wstring> ip_ranges;
			Split(ip_ranges, line_items[4], L' ');

			for (size_t i = 0; i < ip_ranges.size(); i++)
			{
				size_t ip;
				size_t mask;
				size_t mask_range;
				if (ParseIP(WToStr(ip_ranges[i]), ip, mask, mask_range))
				{
					Users.back().AllowedIPRanges.emplace_back(ip, mask, mask_range);
				}
			}
		}
	}

	fclose(UsersFile);
}
void UserList::SaveUsersFile()
{
	FILE *UsersFile;
	_wfopen_s(&UsersFile, users_file.c_str(), L"w, ccs=UTF-16LE");

	if(!UsersFile)
	{
		PrintConsole(12, "Failed to open Users file!");
		return;
	}

	for(size_t i = 0; i < Users.size(); i++)
	{
		string IPList = "";
		for(size_t j = 0; j < Users[i].AllowedIPRanges.size(); j++)
		{
			IPList += FormatArgs("%hs/%u ", inet_ntoa(*(in_addr*)(&Users[i].AllowedIPRanges[j].IP)), Users[i].AllowedIPRanges[j].MaskRange);
		}

		fwprintf_s(UsersFile, L"%ls:%u:%hs:%d:%hs\n", Users[i].UserName.c_str(), Users[i].UserID, Users[i].hash.c_str(), Users[i].PrivilegeLevel, IPList.c_str());
	}

	fclose(UsersFile);
}
void UserList::Clear()
{
	Users.clear();
}

wostringstream UserList::StreamOut()
{
	wostringstream os;
	if (Users.empty())
	{
		os << L"There are no admins." << endl;
	}
	else
	{
		os << setw(4) << L"[ID]" << setw(12) << L"[NAME]" << setw(6) << L"[LVL]    IP List" << endl;
		for (size_t i = 0; i < Users.size(); i++)
		{
			wstring StrIPList = L"";
			for (size_t j = 0; j < Users[i].AllowedIPRanges.size(); j++)
			{
				if (j != 0) StrIPList += L" ";
				string tmp = inet_ntoa(*(in_addr*)(&Users[i].AllowedIPRanges[j].IP));
				StrIPList += wstring(tmp.begin(), tmp.end());
				StrIPList += L"/" + ToWStr((int)Users[i].AllowedIPRanges[j].MaskRange);
			}
			os << setw(4) << Users[i].UserID << setw(12) << Users[i].UserName << setw(6) << Users[i].PrivilegeLevel << L"    " << StrIPList << endl;
		}
	}
	return os;
}

UserList GlobalUserList;