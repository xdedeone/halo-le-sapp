#include "AliasThread.h"

#include <map>
#include <queue>

#include "Addresses.h"
#include "HaloUtilities.h"
#include "SuperHeader.h"
#include "TextUtilities.h"
#include "Threads.h"

using namespace std;

wstring alias_file;

size_t NewAliases = 0;

map <string, set <wstring, WStringICmp>, StringICmp> Aliases;

queue <unique_ptr<AliasStr>> AliasList;

weak_ptr<work> hAliasThread;

Locker AliasLock;
Locker AliasQueueLock;

Locker& GetAliasLock()
{
	return AliasLock;
}

const set <wstring, WStringICmp> emptyset;
const set <wstring, WStringICmp>& GetAliasNoLock(const string& hash)
{
	auto it = Aliases.find(hash);
	if (it != Aliases.end())
		return it->second;
	else
		return emptyset;
}

void AddAlias(string&& hash, wstring&& name)
{
	unique_ptr<AliasStr> NewAlias = make_unique<AliasStr>(move(hash), move(name));

	AliasQueueLock.Lock();

	AliasList.push(move(NewAlias));

	AliasQueueLock.Unlock();

	if (auto sp = hAliasThread.lock())
		sp->Boop();
}

void ProcessAlias(string&& hash, wstring&& name)
{
	Lock_Object guard(AliasLock);
	auto it = Aliases.find(hash);
	if(it == Aliases.end())
	{
		auto ret = Aliases.emplace(move(hash), set <wstring, WStringICmp>());
		ret.first->second.emplace(move(name));
		NewAliases++;
	}
	else
	{
		auto ret = it->second.emplace(move(name));
		if (ret.second)
			NewAliases++;
	}
}

void LoadAliases()
{
	FILE *AliasFile;
	_wfopen_s(&AliasFile, alias_file.c_str(), L"r, ccs=UTF-16LE");

	if(!AliasFile)
	{
		PrintConsole(12, "Failed to open Alias file!");
		return;
	}

	wchar_t buff[4096];

	while(fgetws(buff, 4096, AliasFile))
	{
		size_t n;
		wstring line = buff;

		if((n = line.find_first_not_of(L" \t")) != wstring::npos)
		{
			if(line[n] == ';')
				continue;
		}

		TrimSpaces(line);

		vector <wstring> line_items;
		if (!Split(line_items, line, L':') || line_items.size() < 2 || line_items[0].size() != 32)
			continue;

		string Hash = WToStr(line_items[0]);

		Lock_Object guard(AliasLock);

		auto ret = Aliases.emplace(move(Hash), set <wstring, WStringICmp>());
		
		for(size_t i = 1; i < line_items.size(); i++)
			ret.first->second.emplace(move(line_items[i]));
	}

	fclose(AliasFile);
}

void SaveAliases()
{
	FILE* AliasFile;
	if (_wfopen_s(&AliasFile, alias_file.c_str(), L"w, ccs=UTF-16LE") || !AliasFile)
	{
		//PrintConsole(12, "Error: Failed to open Alias file!");
		//aliasExitFlag = true;
		return;
	}
	
	AliasLock.Lock();

	for(auto& it : Aliases)
	{	
		fwprintf_s(AliasFile, L"%hs", it.first.c_str());
		for(auto& jt : it.second)
			fwprintf_s(AliasFile, L":%ls", jt.c_str());

		fwprintf_s(AliasFile, L"\n");
	}

	AliasLock.Unlock();

	NewAliases = 0;

	fclose(AliasFile);
}

void AliasThreadProc(shared_ptr<work> &hThread)
{
	alias_file = FormatWArgs(L"%hs\\sapp\\alias.txt", GametypesFolder);
	crFile(alias_file);

	LoadAliases();

	while (!hThread->ExitFlag)
	{
		hThread->Wait(INFINITE);

		unique_ptr<AliasStr> AliasEntry;

		AliasQueueLock.Lock();
		if (!AliasList.empty())
		{
			AliasEntry = move(AliasList.front());
			AliasList.pop();
		}
		AliasQueueLock.Unlock();

		if (AliasEntry != nullptr)
		{
			ProcessAlias(move(AliasEntry->Hash), move(AliasEntry->Name));
		}

		if (NewAliases > 15 || (hThread->ExitFlag && NewAliases))
			SaveAliases();
	}
}

bool StartAliasThread()
{
	if (auto sp = hAliasThread.lock())
	{
		return false;
	}
	else
	{
		hAliasThread = ThreadPool.NewWork(AliasThreadProc);
		return true;
	}
}
bool StopAliasThread()
{
	if (auto sp = hAliasThread.lock())
	{
		sp->ExitFlag = true;
		sp->Boop();
		return true;
	}
	else
	{
		return false;
	}
}

void ClearAliases()
{
	Aliases.clear();
}

/*
心 = kokoro = heart = ♥
愛 = ai = love = <3
魔法 = mahou = magic
肉茎 = nikukei = cock
真婿 = manko = pussy
お尻 = oshiri = ass
微乳 = binyuu = small boobs
爆乳 = bakunyuu = big boobs
顔射 = gansha = ejaculation onto partner's face
秘具 = higu = sex toy
セクシー = seukushii = sexy
*/