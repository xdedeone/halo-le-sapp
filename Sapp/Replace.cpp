#include "Replace.h"

#include <iomanip>
#include <map>
#include <memory>

#include "StringCompare.h"

using namespace std;

map <string, shared_ptr<Replace>, StringICmp> Replacements;
map <wstring, shared_ptr<Replace>, WStringICmp> WReplacements;

bool AddCustomVar(const wstring &name, CustomVariableFlags type)
{
	auto& it = WReplacements.find(name);
	if (it != WReplacements.end())
		return false;

	shared_ptr<Replace> NewVar;
	switch (type)
	{
	case global_wstring:
		NewVar = make_shared<GVarWStrReplace>();
		break;
	case global_int:
		NewVar = make_shared<GVarIntReplace>();
		break;
	case global_float:
		NewVar = make_shared<GVarFloatReplace>();
		break;
	case player_wstring:
		NewVar = make_shared<PVarWStrReplace>();
		break;
	case player_int:
		NewVar = make_shared<PVarIntReplace>();
		break;
	case player_float:
		NewVar = make_shared<PVarFloatReplace>();
		break;
	default:
		return false;
	}

	WReplacements[name] = NewVar;
	Replacements[WToStr(name)] = NewVar;

	return true;
}

bool DelCustomVar(const wstring &name)
{
	auto& it = WReplacements.find(name);
	if (it != WReplacements.end() && it->second->custom)
	{
		WReplacements.erase(it);
	}

	auto& jt = Replacements.find(WToStr(name));
	if (jt != Replacements.end() && jt->second->custom)
	{
		Replacements.erase(jt);
		return true;
	}

	return false;
}

bool SetCustomVar(const wstring &name, const wstring &val, size_t PlayerIndex)
{
	auto& it = WReplacements.find(name);

	if (it != WReplacements.end() && it->second->custom)
	{
		it->second->SetVar(val, PlayerIndex);
		return true;
	}

	return false;
}

bool ConvertCustomVar(const wstring &name)
{
	auto& it = WReplacements.find(name);
	auto& jt = Replacements.find(WToStr(name));
	if (it == WReplacements.end() || jt == Replacements.end() || !it->second->custom)
		return false;

	switch (it->second->type)
	{
	case global_wstring:
	{
		return false;
	}
	case global_int:
	{
		float toreplace = static_cast<GVarIntReplace*>(it->second.get())->ToFloat(0);
		shared_ptr<Replace> NewVar = make_shared<GVarFloatReplace>(toreplace);
		it->second = NewVar;
		jt->second = NewVar;
		return true;
	}
	case global_float:
	{
		int toreplace = static_cast<GVarFloatReplace*>(it->second.get())->ToInt(0);
		shared_ptr<Replace> NewVar = make_shared<GVarIntReplace>(toreplace);
		it->second = NewVar;
		jt->second = NewVar;
		return true;
	}
	case player_wstring:
	{
		return false;
	}
	case player_int:
	{
		float toreplace[16];
		for(size_t i = 0; i < 16; i++)
			toreplace[i] = static_cast<PVarIntReplace*>(it->second.get())->ToFloat(i);
		shared_ptr<Replace> NewVar = make_shared<PVarFloatReplace>(toreplace);
		it->second = NewVar;
		jt->second = NewVar;
		return true;
	}
	case player_float:
	{
		int toreplace[16];
		for (size_t i = 0; i < 16; i++)
			toreplace[i] = static_cast<PVarFloatReplace*>(it->second.get())->ToInt(i);
		shared_ptr<Replace> NewVar = make_shared<PVarIntReplace>(toreplace);
		it->second = NewVar;
		jt->second = NewVar;
		return true;
	}
	default:
		return false;
	}
}

const wchar_t* const var_types[var_type_count] = { L"Global String", L"Global Int", L"Global Float", L"Player String", L"Player Int", L"Player Float" };

wostringstream StreamOutVariables(bool custom)
{
	wostringstream os;
	size_t count = 0;
	os << setw(24) << L"Name" << setw(16) << L"Type" << setw(8) << L"Custom" << endl;
	for (auto& it = WReplacements.begin(); it != WReplacements.end(); ++it)
	{
		if (!custom || it->second->custom)
		{
			os << setw(24) << it->first.substr(1) << setw(16) << var_types[it->second->type] << setw(8) << (it->second->custom ? L"True" : L"False") << endl;
			count++;
		}
	}
	if (!count)
	{
		os << L"There are no custom variables." << endl;
	}
	return os;
}

bool StrReplaceOne(size_t PlayerIndex, const std::string &line, string &arg)
{
	auto& it = Replacements.find(line);
	if (it != Replacements.end())
	{
		it->second->ReplacementStr(PlayerIndex, arg);
		return true;
	}
	return false;
}

bool WStrReplaceOne(size_t PlayerIndex, const std::wstring &line, wstring &arg)
{
	auto& it = WReplacements.find(line);
	if(it != WReplacements.end())
	{
		it->second->ReplacementWStr(PlayerIndex, arg);
		return true;
	}
	return false;
}

bool WStrReplaceNum(size_t PlayerIndex, const std::wstring &line, float &number)
{
	auto& it = WReplacements.find(line);
	if(it != WReplacements.end())
	{
		it->second->ReplacementNum(PlayerIndex, number);
		return true;
	}
	return false;
}

void WStrReplace(size_t PlayerIndex, wstring &line)
{	
	// need to loop from the back, so it won't replace overlaps such as $n in $name etc.
	auto& it = WReplacements.end();
	do
	{
		size_t name_pos = line.find('$');
		if(name_pos == wstring::npos)
			break;

		--it;

		while((name_pos = line.find(it->first, name_pos)) != wstring::npos)
		{
			wstring replacement;
			it->second->ReplacementWStr(PlayerIndex, replacement);
			line.replace(name_pos, it->first.size(), replacement);
			name_pos++;
		}
	} while (it != WReplacements.begin());
}
 
void LoadReplacements()
{
	WReplacements[L"$running"] = make_shared<ReplaceRunning>();
	WReplacements[L"$map"] = make_shared<ReplaceMap>();
	WReplacements[L"$mode"] = make_shared<ReplaceMode>();
	WReplacements[L"$gt"] = make_shared<ReplaceGT>();
	WReplacements[L"$svname"] = make_shared<ReplaceSVName>();
	WReplacements[L"$ticks"] = make_shared<ReplaceTicks>();
	WReplacements[L"$name"] = make_shared<ReplaceName>();
	WReplacements[L"$team"] = make_shared<ReplaceTeam>();
	WReplacements[L"$oteam"] = make_shared<ReplaceOTeam>();
	WReplacements[L"$n"] = make_shared<ReplaceNumber>();
	WReplacements[L"$pn"] = make_shared<ReplacePlayerNumber>();
	WReplacements[L"$reds"] = make_shared<ReplaceRedPlayerNumber>();
	WReplacements[L"$blues"] = make_shared<ReplaceBluePlayerNumber>();
	WReplacements[L"$redscore"] = make_shared<ReplaceRedScore>();
	WReplacements[L"$bluescore"] = make_shared<ReplaceBlueScore>();
	WReplacements[L"$streak"] = make_shared<ReplaceStreak>();
	WReplacements[L"$combo"] = make_shared<ReplaceCombo>();
	WReplacements[L"$score"] = make_shared<ReplaceScore>();
	WReplacements[L"$kills"] = make_shared<ReplaceKills>();
	WReplacements[L"$assists"] = make_shared<ReplaceAssists>();
	WReplacements[L"$deaths"] = make_shared<ReplaceDeaths>();
	WReplacements[L"$tk"] = make_shared<ReplaceTK>();
	WReplacements[L"$suicides"] = make_shared<ReplaceSuicides>();
	WReplacements[L"$campkills"] = make_shared<ReplaceCampkills>();
	WReplacements[L"$botscore"] = make_shared<ReplaceBotScore>();
	WReplacements[L"$afk"] = make_shared<ReplaceAFK>();
	WReplacements[L"$ping"] = make_shared<ReplacePing>();
	WReplacements[L"$invis"] = make_shared<ReplaceInvis>();
	WReplacements[L"$x"] = make_shared<ReplacePosX>();
	WReplacements[L"$y"] = make_shared<ReplacePosY>();
	WReplacements[L"$z"] = make_shared<ReplacePosZ>();
	WReplacements[L"$hp"] = make_shared<ReplaceHP>();
	WReplacements[L"$sh"] = make_shared<ReplaceSH>();
	WReplacements[L"$ffa"] = make_shared<ReplaceFFA>();
	WReplacements[L"$rand"] = make_shared<ReplaceRandom>();
	WReplacements[L"$lvl"] = make_shared<ReplaceLevel>();
	WReplacements[L"$valid"] = make_shared<ReplaceValid>();
	WReplacements[L"$hash"] = make_shared<ReplaceHash>();
	WReplacements[L"$ip"] = make_shared<ReplaceIP>();
	WReplacements[L"$app"] = make_shared<ReplaceApp>();

	for (auto& it = WReplacements.begin(); it != WReplacements.end(); ++it)
	{
		Replacements[WToStr(it->first)] = it->second;
	}
}

void ClearReplacements()
{
	WReplacements.clear();
	Replacements.clear();
}