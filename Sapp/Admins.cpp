#include "Admins.h"

#include <iomanip>

#include "HaloUtilities.h"
#include "SuperHeader.h"
#include "Textutilities.h"

using namespace std;

bool AdminList::AddAdmin(wstring &&name, string &&password, int lvl)
{
	for (size_t i = 0; i < Admins.size(); i++)
	{
		if (Admins[i].name == name)
		{
			return false;
		}
	}		
		
	Admins.emplace_back(move(name), move(password), lvl);
	return true;
}
int AdminList::IsAdmin(const wstring &name, const string &password)
{
	for (size_t i = 0; i < Admins.size(); i++)
	{
		if (Admins[i].name == name)
		{
			if (Admins[i].pwhash == password)
			{
				return Admins[i].lvl;
			}
		}
	}

	return -1;
}
int AdminList::IsRconAdmin(const wstring &name)
{
	for (size_t i = 0; i < Admins.size(); i++)
	{
		if (Admins[i].name == name)
		{
			return Admins[i].lvl;
		}
	}

	return -1;
}
int AdminList::GetAdminID(const wstring &name)
{
	for (size_t i = 0; i < Admins.size(); i++)
	{
		if (Admins[i].name == name)
		{
			return i;
		}
	}

	return -1;
}
bool AdminList::ChangePassword(size_t Index, const string &password)
{
	if(Index >= Admins.size())
		return false;

	Admins[Index].pwhash = password;
	return true;
}
bool AdminList::ChangeLevel(size_t Index, int lvl)
{
	if(Index >= Admins.size())
		return false;

	Admins[Index].lvl = lvl;
	return true;
}
bool AdminList::RemoveAdmin(size_t Index)
{
	if(Index >= Admins.size())
		return false;

	Admins.erase(Admins.begin() + Index);
	return true;
}
void AdminList::LoadAdmins()
{
	admins_file = L"sapp\\admins.txt";
	crFile(admins_file);

	FILE *AdminsFile;
	_wfopen_s(&AdminsFile, admins_file.c_str(), L"r, ccs=UTF-16LE");

	if(!AdminsFile)
	{
		PrintConsole(12, "Failed to open Admins file!");
		return;
	}

	wchar_t buff[4096];

	while(fgetws(buff, 4096, AdminsFile))
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

		for (size_t i = 0; i < line_items.size(); i++)
			TrimSpaces(line_items[i]);

		int lvl;
		if(!ToNumber(line_items[2], lvl))
			continue;

		if(lvl > 4)
			lvl = 4;

		AddAdmin(move(line_items[0]), WToStr(line_items[1]), lvl);
	}

	fclose(AdminsFile);
}
void AdminList::SaveAdmins()
{
	FILE *AdminsFile;
	_wfopen_s(&AdminsFile, admins_file.c_str(), L"w, ccs=UTF-16LE");

	if(!AdminsFile)
	{
		PrintConsole(12, "Failed to open Admins file!");
		return;
	}	

	for(size_t i = 0; i < Admins.size();i++)
	{
		fwprintf_s(AdminsFile, L"%ls:%hs:%d\n", Admins[i].name.c_str(), Admins[i].pwhash.c_str(), Admins[i].lvl);
	}

	fclose(AdminsFile);
}
void AdminList::Clear()
{
	Admins.clear();
}

wostringstream AdminList::StreamOut()
{
	wostringstream os;
	if (Admins.empty())
	{
		os << L"There are no admins." << endl;
	}
	else
	{
		os << setw(4) << L"[ID]" << setw(12) << L"[NAME]" << setw(6) << L"[LVL]" << endl;
		for (size_t i = 0; i < Admins.size(); i++)
		{
			os << setw(4) << i << setw(12) << Admins[i].name << setw(6) << Admins[i].lvl << endl;
		}
	}
	return os;
}

AdminList GlobalAdminList;