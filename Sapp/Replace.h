#pragma once

#include <sstream>

#include "AimbotScan.h"
#include "Addresses.h"
#include "CDKeyCheck.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "MiscellaneousFunctions.h"
#include "OnlineAdmins.h"
#include "TextUtilities.h"

enum CustomVariableFlags : unsigned char { global_wstring, global_int, global_float, player_wstring, player_int, player_float, var_type_count };

bool AddCustomVar(const std::wstring &name, CustomVariableFlags type);
bool DelCustomVar(const std::wstring &name);
bool SetCustomVar(const std::wstring &name, const std::wstring &val, size_t PlayerIndex);
bool ConvertCustomVar(const std::wstring &name);

std::wostringstream StreamOutVariables(bool custom);

bool StrReplaceOne(size_t PlayerIndex, const std::string &line, std::string &arg);
bool WStrReplaceOne(size_t PlayerNumber, const std::wstring &line, std::wstring &arg);
bool WStrReplaceNum(size_t PlayerIndex, const std::wstring &line, float &number);
void WStrReplace(size_t PlayerNumber, std::wstring &line);

void LoadReplacements();
void ClearReplacements();

class Replace {
public: 
	CustomVariableFlags type;
	bool custom;
	Replace(CustomVariableFlags type, bool custom = false) : type(type), custom(custom) {}
	virtual void ReplacementStr(size_t PlayerIndex, std::string &arg) = 0;
	virtual void ReplacementWStr(size_t PlayerIndex, std::wstring &arg) = 0;
	virtual void ReplacementNum(size_t PlayerIndex, float &number) {}
	virtual void SetVar(const std::wstring &val, size_t Index) {}
	virtual ~Replace() {}
};

class SappReplaceInt : public Replace
{
public:
	SappReplaceInt(CustomVariableFlags type, bool custom = false) : Replace(type, custom) {}
	virtual void ReplacementStr(size_t PlayerIndex, std::string &arg)
	{
		int num;
		ReplacementInt(PlayerIndex, num);
		arg = ToStr(num);
	}
	virtual void ReplacementWStr(size_t PlayerIndex, std::wstring &arg)
	{
		int num;
		ReplacementInt(PlayerIndex, num);
		arg = ToWStr(num);	
	}
	virtual void ReplacementNum(size_t PlayerIndex, float &number)
	{
		int num;
		ReplacementInt(PlayerIndex, num);
		number = (float)num;
	}
	virtual float ToFloat(size_t PlayerIndex)
	{
		int num;
		ReplacementInt(PlayerIndex, num);
		return (float)num;
	}
	virtual void ReplacementInt(size_t PlayerIndex, int &number) = 0;
	virtual ~SappReplaceInt() {}
};

class SappReplaceFloat : public Replace
{
public:
	SappReplaceFloat(CustomVariableFlags type, bool custom = false) : Replace(type, custom) {}
	virtual void ReplacementStr(size_t PlayerIndex, std::string &arg)
	{
		float num;
		ReplacementFloat(PlayerIndex, num);
		arg = ToStr(num);
	}
	virtual void ReplacementWStr(size_t PlayerIndex, std::wstring &arg)
	{
		float num;
		ReplacementFloat(PlayerIndex, num);
		arg = ToWStr(num);
	}
	virtual void ReplacementNum(size_t PlayerIndex, float &number)
	{
		float num;
		ReplacementFloat(PlayerIndex, num);
		number = num;
	}
	virtual int ToInt(size_t PlayerIndex)
	{
		float num;
		ReplacementFloat(PlayerIndex, num);
		return (int)num;
	}
	virtual void ReplacementFloat(size_t PlayerIndex, float &number) = 0;
	virtual ~SappReplaceFloat() {}
};

class GVarWStrReplace final : public Replace
{
	std::wstring replacement;
public:
	GVarWStrReplace() : Replace(global_wstring, true) {}
	void ReplacementStr(size_t PlayerIndex, std::string &arg)
	{
		arg = WToStr(replacement);
	}
	void ReplacementWStr(size_t PlayerIndex, std::wstring &arg)
	{
		arg = replacement;
	}
	void SetVar(const std::wstring &val, size_t Index)
	{
		replacement = val;
	}
};

class GVarIntReplace final : public SappReplaceInt
{
	int replacement;
public:
	GVarIntReplace(int r = 0) : SappReplaceInt(global_int, true), replacement(r) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		number = replacement;
	}
	void SetVar(const std::wstring &val, size_t Index)
	{
		ProcessOperator(val, replacement);
	}
};

class GVarFloatReplace final : public SappReplaceFloat
{
	float replacement;
public:
	GVarFloatReplace(float r = 0) : SappReplaceFloat(global_float, true), replacement(r) {}
	void ReplacementFloat(size_t PlayerIndex, float &number)
	{
		number = replacement;
	}
	void SetVar(const std::wstring &val, size_t Index)
	{
		ProcessOperator(val, replacement);
	}
};

class PVarWStrReplace final : public Replace
{
	std::wstring replacement[16];
public:
	PVarWStrReplace(): Replace(player_wstring, true) {}
	void ReplacementStr(size_t PlayerIndex, std::string &arg)
	{
		if (PlayerIndex < 16)
		{
			arg = WToStr(replacement[PlayerIndex]);
		}
	}
	void ReplacementWStr(size_t PlayerIndex, std::wstring &arg)
	{
		if (PlayerIndex < 16)
		{
			arg = replacement[PlayerIndex];
		}		
	}
	void SetVar(const std::wstring &val, size_t Index)
	{
		if (Index < 16)
		{
			replacement[Index] = val;
		}
	}
};

class PVarIntReplace final : public SappReplaceInt
{
	int replacement[16];
public:
	PVarIntReplace(int* r = nullptr) : SappReplaceInt(player_int, true)
	{
		if (r == nullptr)
			memset(replacement, 0, sizeof(replacement));
		else
			memcpy(replacement, r, sizeof(replacement));
	}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = replacement[PlayerIndex];
		else
			number = 0;
	}
	void SetVar(const std::wstring &val, size_t Index)
	{
		if (Index < 16)
		{
			ProcessOperator(val, replacement[Index]);
		}
	}
};

class PVarFloatReplace final : public SappReplaceFloat
{
	float replacement[16];
public:
	PVarFloatReplace(float* r = nullptr) : SappReplaceFloat(player_float, true)
	{		
		if (r == nullptr)
			memset(replacement, 0, sizeof(replacement));
		else
			memcpy(replacement, r, sizeof(replacement));
	}
	void ReplacementFloat(size_t PlayerIndex, float &number)
	{
		if (PlayerIndex < 16)
		{
			number = replacement[PlayerIndex];
		}
		else
			number = 0;
	}
	void SetVar(const std::wstring &val, size_t Index)
	{
		if (Index < 16)
		{
			ProcessOperator(val, replacement[Index]);
		}
	}
};

class ReplaceRunning final : public SappReplaceInt
{
public:
	ReplaceRunning() : SappReplaceInt(global_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		number = ServerRunning();
	}
};

class ReplaceMap final : public Replace
{
public:
	ReplaceMap() : Replace(global_wstring) {}
	void ReplacementStr(size_t PlayerIndex, std::string &arg)
	{
		if (ServerRunning())
			arg = CurrentMap;
		else
			arg = "n/a";
	}
	void ReplacementWStr(size_t PlayerIndex, std::wstring &arg)
	{
		if (ServerRunning())
			arg = CurrentMapW;
		else
			arg = L"n/a";
	}
};

class ReplaceMode final : public Replace
{
public:
	ReplaceMode() : Replace(global_wstring) {}
	void ReplacementStr(size_t PlayerIndex, std::string &arg)
	{
		if (ServerRunning())
			arg = CurrentMode;
		else
			arg = "n/a";
	}
	void ReplacementWStr(size_t PlayerIndex, std::wstring &arg)
	{
		if (ServerRunning())
			arg = CurrentModeW;
		else
			arg = L"n/a";
	}
};

class ReplaceGT final : public Replace
{
public:
	ReplaceGT() : Replace(global_wstring) {}
	void ReplacementStr(size_t PlayerIndex, std::string &arg)
	{
		if (ServerRunning())
			arg = CurrentGT;
		else
			arg = "n/a";
	}
	void ReplacementWStr(size_t PlayerIndex, std::wstring &arg)
	{
		if (ServerRunning())
			arg = CurrentGTW;
		else
			arg = L"n/a";
	}
};

class ReplaceSVName final : public Replace
{
public:
	ReplaceSVName() : Replace(global_wstring) {}
	void ReplacementStr(size_t PlayerIndex, std::string &arg)
	{
		if (ServerRunning())
			arg = WToStr(Server->ServerName);
		else
			arg = "n/a";
	}
	void ReplacementWStr(size_t PlayerIndex, std::wstring &arg)
	{
		if (ServerRunning())
			arg = Server->ServerName;
		else
			arg = L"n/a";
	}
};

class ReplaceTicks final : public SappReplaceInt
{
public:
	ReplaceTicks() : SappReplaceInt(global_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		number = *GameTicks;
	}
};

class ReplaceName final : public Replace
{
public:
	ReplaceName() : Replace(player_wstring) {}
	void ReplacementStr(size_t PlayerIndex, std::string &arg)
	{
		if (ServerRunning() && PlayerIndex < 16)
			arg = WToStr(Server->PlayerData[PlayerIndex].name);
		else
			arg.clear();
	}
	void ReplacementWStr(size_t PlayerIndex, std::wstring &arg)
	{
		if (ServerRunning() && PlayerIndex < 16)
			arg = Server->PlayerData[PlayerIndex].name;
		else
			arg.clear();
	}
};

class ReplaceNumber final : public SappReplaceInt
{
public:
	ReplaceNumber() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		number = IndexToNumber(PlayerIndex) + 1;
	}
};

class ReplacePlayerNumber final : public SappReplaceInt
{
public:
	ReplacePlayerNumber() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		number = GetPlayerNumber();
	}
};

class ReplaceRedPlayerNumber final : public SappReplaceInt
{
public:
	ReplaceRedPlayerNumber() : SappReplaceInt(global_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		number = GetRedPlayerNumber();
	}
};

class ReplaceBluePlayerNumber final : public SappReplaceInt
{
public:
	ReplaceBluePlayerNumber() : SappReplaceInt(global_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		number = GetBluePlayerNumber();
	}
};

class ReplaceRedScore final : public SappReplaceInt
{
public:
	ReplaceRedScore() : SappReplaceInt(global_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (*TeamGame)
			number = GetTeamScore(0);
		else
			number = 0;
	}
};

class ReplaceBlueScore final : public SappReplaceInt
{
public:
	ReplaceBlueScore() : SappReplaceInt(global_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (*TeamGame)
			number = GetTeamScore(1);
		else
			number = 0;
	}
};

class ReplacePosX final : public SappReplaceFloat
{
public:
	ReplacePosX() : SappReplaceFloat(player_float) {}
	void ReplacementFloat(size_t PlayerIndex, float &number)
	{
		if(PlayerAliveOnly(PlayerIndex))
		{
			vect3f loc;
			GetPlayerPosition(PlayerIndex, loc);
			number = loc.x;
		}
		else
			number = 0;
	}
};

class ReplacePosY final : public SappReplaceFloat
{
public:
	ReplacePosY() : SappReplaceFloat(player_float) {}
	void ReplacementFloat(size_t PlayerIndex, float &number)
	{
		if(PlayerAliveOnly(PlayerIndex))
		{
			vect3f loc;
			GetPlayerPosition(PlayerIndex, loc);
			number = loc.y;
		}
		else
			number = 0;
	}
};

class ReplacePosZ final : public SappReplaceFloat
{
public:
	ReplacePosZ() : SappReplaceFloat(player_float) {}
	void ReplacementFloat(size_t PlayerIndex, float &number)
	{
		if(PlayerAliveOnly(PlayerIndex))
		{
			vect3f loc;
			GetPlayerPosition(PlayerIndex, loc);
			number = loc.z;
		}
		else
			number = 0;
	}
};

class ReplaceHP final : public SappReplaceFloat
{
public:
	ReplaceHP() : SappReplaceFloat(player_float) {}
	void ReplacementFloat(size_t PlayerIndex, float &number)
	{
		if(PlayerAliveOnly(PlayerIndex))
		{
			Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;
			number = Player->Health;
		}
		else
			number = 0;
	}
};

class ReplaceSH final : public SappReplaceFloat
{
public:
	ReplaceSH() : SappReplaceFloat(player_float) {}
	void ReplacementFloat(size_t PlayerIndex, float &number)
	{
		if(PlayerAliveOnly(PlayerIndex))
		{
			Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;
			number = Player->Shield;
		}
		else
			number = 0;
	}
};

class ReplaceStreak final : public SappReplaceInt
{
public:
	ReplaceStreak() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = PlayerTable[PlayerIndex].killstreak;
		else
			number = 0;
	}
};

class ReplaceCombo final : public SappReplaceInt
{
public:
	ReplaceCombo() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = PlayerTable[PlayerIndex].combo;
		else
			number = 0;
	}
};

class ReplaceScore final : public SappReplaceInt
{
public:
	ReplaceScore() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = GetPlayerScore(PlayerIndex);
		else
			number = 0;
	}
};

class ReplaceKills final : public SappReplaceInt
{
public:
	ReplaceKills() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = PlayerTable[PlayerIndex].kills;
		else
			number = 0;
	}
};

class ReplaceAssists final : public SappReplaceInt
{
public:
	ReplaceAssists() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = PlayerTable[PlayerIndex].assists;
		else
			number = 0;
	}
};

class ReplaceDeaths final : public SappReplaceInt
{
public:
	ReplaceDeaths() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = PlayerTable[PlayerIndex].deaths;
		else
			number = 0;
	}
};

class ReplaceTK final : public SappReplaceInt
{
public:
	ReplaceTK() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = PlayerTable[PlayerIndex].tk_number;
		else
			number = 0;
	}
};

class ReplaceSuicides final : public SappReplaceInt
{
public:
	ReplaceSuicides() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = PlayerTable[PlayerIndex].suicides;
		else
			number = 0;
	}
};

class ReplaceCampkills final : public SappReplaceInt
{
public:
	ReplaceCampkills() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = PlayerCampKills[PlayerIndex];
		else
			number = 0;
	}
};

class ReplaceBotScore final : public SappReplaceInt
{
public:
	ReplaceBotScore() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = AimbotScores[PlayerIndex];
		else
			number = 0;
	}
};

class ReplaceAFK final : public SappReplaceInt
{
public:
	ReplaceAFK() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = afk_timer[PlayerIndex];
		else
			number = 0;
	}
};

class ReplacePing final : public SappReplaceInt
{
public:
	ReplacePing() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = PlayerTable[PlayerIndex].ping;
		else
			number = 0;
	}
};

class ReplaceInvis final : public SappReplaceInt
{
public:
	ReplaceInvis() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if(PlayerAliveOnly(PlayerIndex))
		{
			Dynamic_Player *Player = ObjectTable[PlayerTable[PlayerIndex].ObjectIndexNum].Offset;
			if(Player->IsInvisible & 0x10)
				number = 1;
			else
				number = 0;
		}
		else
			number = 0;
	}
};

class ReplaceFFA final : public SappReplaceInt
{
public:
	ReplaceFFA() : SappReplaceInt(global_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		number = !(*TeamGame);
	}
};

class ReplaceRandom final : public SappReplaceInt
{
public:
	ReplaceRandom() : SappReplaceInt(global_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		number = GetRand(1, 17);
	}
};


class ReplaceTeam final : public Replace
{
public:
	ReplaceTeam() : Replace(player_wstring) {}
	void ReplacementStr(size_t PlayerIndex, std::string &arg)
	{
		if (PlayerIndex < 16)
			arg = PlayerTable[PlayerIndex].Team == 0 ? "red" : "blue";
		else
			arg.clear();
	}
	void ReplacementWStr(size_t PlayerIndex, std::wstring &arg)
	{
		if (PlayerIndex < 16)
			arg = PlayerTable[PlayerIndex].Team == 0 ? L"red" : L"blue";
		else
			arg.clear();
	}
};

class ReplaceOTeam final : public Replace
{
public:
	ReplaceOTeam() : Replace(player_wstring) {}
	void ReplacementStr(size_t PlayerIndex, std::string &arg)
	{
		if (PlayerIndex < 16)
			arg = PlayerTable[PlayerIndex].Team == 0 ? "blue" : "red";
		else
			arg.clear();
	}
	void ReplacementWStr(size_t PlayerIndex, std::wstring &arg)
	{
		if (PlayerIndex < 16)
			arg = PlayerTable[PlayerIndex].Team == 0 ? L"blue" : L"red";
		else
			arg.clear();
	}
};

class ReplaceLevel final : public SappReplaceInt
{
public:
	ReplaceLevel() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = GlobalOnlineAdminList.GetAdminLevel(PlayerIndex);
		else
			number = -1;
	}
};

class ReplaceValid final : public SappReplaceInt
{
public:
	ReplaceValid() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = valid_cdkey[PlayerIndex];
		else
			number = 0;
	}
};

class ReplaceHash final : public Replace
{
public:
	ReplaceHash() : Replace(player_wstring) {}
	void ReplacementStr(size_t PlayerIndex, std::string &arg)
	{
		if (PlayerPresent(PlayerIndex))
			arg = GetPlayerHash(IndexToNumber(PlayerIndex));
		else
			arg.clear();
	}
	void ReplacementWStr(size_t PlayerIndex, std::wstring &arg)
	{
		if (PlayerPresent(PlayerIndex))
		{
			std::string tmp = GetPlayerHash(IndexToNumber(PlayerIndex));
			arg = std::wstring(tmp.begin(), tmp.end());
		}
		else
			arg.clear();
	}
};

class ReplaceIP final : public Replace
{
public:
	ReplaceIP() : Replace(player_wstring) {}
	void ReplacementStr(size_t PlayerIndex, std::string &arg)
	{
		if (PlayerPresent(PlayerIndex))
			arg = GetPlayerIP(IndexToNumber(PlayerIndex));
		else
			arg.clear();
	}
	void ReplacementWStr(size_t PlayerIndex, std::wstring &arg)
	{
		if(PlayerPresent(PlayerIndex))
		{
			std::string tmp = GetPlayerIP(IndexToNumber(PlayerIndex));
			arg = std::wstring(tmp.begin(), tmp.end());
		}
		else
			arg.clear();
	}
};

class ReplaceApp final : public SappReplaceInt
{
public:
	ReplaceApp() : SappReplaceInt(player_int) {}
	void ReplacementInt(size_t PlayerIndex, int &number)
	{
		if (PlayerIndex < 16)
			number = PlayerApps[PlayerIndex];
		else
			number = 0;
	}
};