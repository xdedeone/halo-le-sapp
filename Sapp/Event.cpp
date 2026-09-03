#include "Event.h"

#include "Addresses.h"
#include "CommandExecuter.h"
#include "LuaHandler.h"
#include "Prefs.h"
#include "Replace.h"
#include "TextUtilities.h"

using namespace std;

void EventHandler::LoadEventsCache()
{
	SappEventsCache[L"custom"] = EVENT_CUSTOM;
	SappEventsCache[L"tick"] = EVENT_TICK;
	SappEventsCache[L"alive"] = EVENT_ALIVE;
	SappEventsCache[L"aenter"] = EVENT_AREA_ENTER;
	SappEventsCache[L"aexit"] = EVENT_AREA_EXIT;
	SappEventsCache[L"prespawn"] = EVENT_PRESPAWN;
	SappEventsCache[L"spawn"] = EVENT_SPAWN;
	SappEventsCache[L"kill"] = EVENT_KILL;
	SappEventsCache[L"die"] = EVENT_DIE;
	SappEventsCache[L"assist"] = EVENT_ASSIST;
	SappEventsCache[L"snap"] = EVENT_SNAP;
	SappEventsCache[L"warp"] = EVENT_WARP;
	SappEventsCache[L"wpickup"] = EVENT_WEAPON_PICKUP;
	SappEventsCache[L"wdrop"] = EVENT_WEAPON_DROP;
	SappEventsCache[L"venter"] = EVENT_VEHICLE_ENTER;
	SappEventsCache[L"vexit"] = EVENT_VEHICLE_EXIT;
	SappEventsCache[L"tk"] = EVENT_BETRAY;
	SappEventsCache[L"suicide"] = EVENT_SUICIDE;
	SappEventsCache[L"score"] = EVENT_SCORE;
	SappEventsCache[L"teamswitch"] = EVENT_TEAM_SWITCH;
	SappEventsCache[L"prejoin"] = EVENT_PREJOIN;
	SappEventsCache[L"join"] = EVENT_JOIN;
	SappEventsCache[L"leave"] = EVENT_LEAVE;
	SappEventsCache[L"camp"] = EVENT_CAMP;
	SappEventsCache[L"login"] = EVENT_LOGIN;
	SappEventsCache[L"start"] = EVENT_GAME_START;
	SappEventsCache[L"end"] = EVENT_GAME_END;
	SappEventsCache[L"reset"] = EVENT_MAP_RESET;
}

void EventHandler::LoadEvents()
{
	event_file = FormatWArgs(L"%hs\\sapp\\events.txt", GametypesFolder);
	crFile(event_file);

	FILE *EventsFile;
	_wfopen_s(&EventsFile, event_file.c_str(), L"r, ccs=UTF-16LE");
	if(!EventsFile)
	{
		PrintConsole(12, "Failed to open Events file!");
		return;
	}

	wchar_t buff[4096];
	while(fgetws(buff, 4096, EventsFile))
	{
		ParseEvent(wstring(buff));
	}

	fclose(EventsFile);
}

void EventHandler::Clear()
{
	for (size_t i = 0; i < SAPP_EVENT_COUNT; i++)
		EventCache[i].clear();

	SappEventQueue.clear();
}

EventHandler::EventHandler()
{
	LoadEventsCache();
}

bool EventHandler::EventRemove(size_t EventIndex)
{
	if(EventIndex >= SappEventQueue.size())
		return false;

	auto& e = SappEventQueue[EventIndex];

	for (auto& it = EventCache[e->type].begin();  it != EventCache[e->type].end(); ++it)
	{
		if (*it == e)
		{
			EventCache[e->type].erase(it);
			break;
		}
	}

	SappEventQueue.erase(SappEventQueue.begin() + EventIndex);

	return true;
}

void EventHandler::ParseEvent(wstring& line)
{
	TrimSpaces(line);

	if (line.size() > 8)
	{
		line[5] = ' ';
		ArgContainer args(line);
		line[5] = '_';

		if (args.argc > 2 && args[0] == L"event")
		{
			map <wstring, Sapp_Event>::iterator it = SappEventsCache.find(args[1]);
			if (it != SappEventsCache.end())
			{
				auto EventToAdd = make_shared<SappEvent>(it->second, line, args[args.argc - 1]);
				for (size_t i = 2; i < args.argc - 1; i++)
				{
					EventArgsStruct EventArgs;
					if (ProcessArgument(args[i], EventArgs))
						EventToAdd->EventArgs.emplace_back(EventArgs);
				}

				SappEventQueue.emplace_back(EventToAdd);

				EventCache[it->second].emplace_back(EventToAdd);
			}
		}
	}
}

bool EventHandler::ProcessArgument(const wstring &EventArg, EventArgsStruct &Args)
{
	size_t i = 0;
	size_t rlenght;
	bool ok = false;
	while (i < EventArg.size())
	{
		switch (EventArg[i])
		{
		case '!':
			if (EventArg[i + 1] == ':')
			{
				Args.type = 0;
				rlenght = 2;
				ok = true;
			}
			else if (EventArg[i + 1] == '=')
			{
				Args.type = 6;
				rlenght = 2;
				ok = true;
			}
			break;
		case ':':
			Args.type = 1;
			rlenght = 1;
			ok = true;
			break;
		case '<':
			if (EventArg[i + 1] == '=')
			{
				Args.type = 2;
				rlenght = 2;
				ok = true;
			}
			else
			{
				Args.type = 3;
				rlenght = 1;
				ok = true;
			}
			break;
		case '=':
			Args.type = 7;
			rlenght = 1;
			ok = true;
			break;
		case '>':
			if (EventArg[i + 1] == '=')
			{
				Args.type = 4;
				rlenght = 2;
				ok = true;
			}
			else
			{
				Args.type = 5;
				rlenght = 1;
				ok = true;
			}
			break;
		}
		if (ok)
			break;
		i++;
	}

	if (!ok)
		return false;

	Args.Arg1 = EventArg.substr(0, i);
	Args.Arg2 = EventArg.substr(i + rlenght);

	TrimSpaces(Args.Arg1);
	TrimSpaces(Args.Arg2);

	if (Args.Arg1[0] != '$')
	{
		Args.Var = 2;
		if (Args.type > 1 && !ToNumber(Args.Arg1, Args.ArgNum))
				return false;
	}
	else if (Args.Arg2[0] != '$')
	{
		Args.Var = 1;
		if (Args.type > 1 && !ToNumber(Args.Arg2, Args.ArgNum))
			return false;
	}		
	else
	{
		Args.Var = 0;
	}

	Args.mtype1 = 0;
	Args.mtype2 = 0;

	if (Args.type > 1)
	{
		if (Args.Var == 0 || Args.Var == 1)
		{
			for (size_t j = 0; j < Args.Arg1.size(); j++)
			{
				switch (Args.Arg1[j])
				{
				case '*':
					if (ToNumber(Args.Arg1.substr(j + 1), Args.modif1) && Args.modif1 != 0)
					{
						Args.Arg1.erase(j);
						Args.mtype1 = 1;
					}
					else
					{
						return false;
					}
					break;
				case '+':
					if (ToNumber(Args.Arg1.substr(j + 1), Args.modif1) && Args.modif1 != 0)
					{
						Args.Arg1.erase(j);
						Args.mtype1 = 2;
					}
					else
					{
						return false;
					}
					break;
				case '-':
					if (ToNumber(Args.Arg1.substr(j + 1), Args.modif1) && Args.modif1 != 0)
					{
						Args.Arg1.erase(j);
						Args.mtype1 = 3;
					}
					else
					{
						return false;
					}
					break;
				case '/':
					if (ToNumber(Args.Arg1.substr(j + 1), Args.modif1) && Args.modif1 != 0)
					{
						Args.Arg1.erase(j);
						Args.mtype1 = 4;
					}
					else
					{
						return false;
					}
					break;
				}
			}
		}
		if (Args.Var == 0 || Args.Var == 2)
		{
			for (size_t j = 0; j < Args.Arg2.size(); j++)
			{
				switch (Args.Arg2[j])
				{
				case '*':
					if (ToNumber(Args.Arg2.substr(j + 1), Args.modif2) && Args.modif2 != 0)
					{
						Args.Arg2.erase(j);
						Args.mtype2 = 1;
					}
					else
					{
						return false;
					}
					break;
				case '+':
					if (ToNumber(Args.Arg2.substr(j + 1), Args.modif2) && Args.modif2 != 0)
					{
						Args.Arg2.erase(j);
						Args.mtype2 = 2;
					}
					else
					{
						return false;
					}
					break;
				case '-':
					if (ToNumber(Args.Arg2.substr(j + 1), Args.modif2) && Args.modif2 != 0)
					{
						Args.Arg2.erase(j);
						Args.mtype2 = 3;
					}
					else
					{
						return false;
					}
					break;
				case '/':
					if (ToNumber(Args.Arg2.substr(j + 1), Args.modif2) && Args.modif2 != 0)
					{
						Args.Arg2.erase(j);
						Args.mtype2 = 4;
					}
					else
					{
						return false;
					}
					break;
				}
			}
		}
	}

	return true;
}

bool EventHandler::ToExecute(size_t PlayerIndex, const vector <EventArgsStruct> &Args, const vector <pair<wstring, wstring>> &SpecArgs)
{	
	for(size_t i = 0; i < Args.size(); i++)
	{
		if(Args[i].type <= 1)
		{
			wstring Arg1_Replacement, Arg2_Replacement;
			size_t toReplace = (Args[i].Var == 0) ? 2 : 1;

			if(Args[i].Var == 1 || Args[i].Var == 0)
			{
				if (WStrReplaceOne(PlayerIndex, Args[i].Arg1, Arg1_Replacement))
					toReplace--;
				else
				{
					for (auto &it : SpecArgs)
					{
						if (!_wcsicmp(it.first.c_str(), Args[i].Arg1.c_str()))
						{
							Arg1_Replacement = it.second;
							toReplace--;
							break;
						}
					}
				}
			}
			if(Args[i].Var == 2 || Args[i].Var == 0)
			{
				if (WStrReplaceOne(PlayerIndex, Args[i].Arg2, Arg2_Replacement))
					toReplace--;
				else
				{
					for (auto &it : SpecArgs)
					{
						if (!_wcsicmp(it.first.c_str(), Args[i].Arg2.c_str()))
						{
							Arg2_Replacement = it.second;
							toReplace--;
							break;
						}
					}
				}
			}

			if(toReplace)
				return false;

			int result;
			if(Args[i].Var == 1)
				result = _wcsicmp(Arg1_Replacement.c_str(), Args[i].Arg2.c_str());
			else if(Args[i].Var == 2)
				result = _wcsicmp(Args[i].Arg1.c_str(), Arg2_Replacement.c_str());
			else
				result = _wcsicmp(Arg1_Replacement.c_str(), Arg2_Replacement.c_str());
			
			if((Args[i].type == 0 && !result) || (Args[i].type == 1 && result)) // !: and :
				return false;
		}
		else
		{	
			float Arg1_Replacement = 0, Arg2_Replacement = 0;
			size_t toReplace = (Args[i].Var == 0) ? 2 : 1;

			if(Args[i].Var == 1 || Args[i].Var == 0)
			{
				if (Args[i].Var == 1)
					Arg2_Replacement = Args[i].ArgNum;

				if (WStrReplaceNum(PlayerIndex, Args[i].Arg1, Arg1_Replacement))
					toReplace--;
				else
				{
					for (auto &it : SpecArgs)
					{
						if (!_wcsicmp(it.first.c_str(), Args[i].Arg1.c_str()))
						{
							if (ToNumber(it.second, Arg1_Replacement))
								toReplace--;
							break;
						}
					}
				}
			}
			if(Args[i].Var == 2 || Args[i].Var == 0)
			{
				if (Args[i].Var == 2)
					Arg1_Replacement = Args[i].ArgNum;

				if (WStrReplaceNum(PlayerIndex, Args[i].Arg2, Arg2_Replacement))
					toReplace--;
				else
				{
					for (auto &it : SpecArgs)
					{
						if (!_wcsicmp(it.first.c_str(), Args[i].Arg2.c_str()))
						{
							if (ToNumber(it.second, Arg2_Replacement))
								toReplace--;
							break;
						}
					}
				}
			}

			if(toReplace)
				return false;

			switch (Args[i].mtype1)
			{
			case 1:
				Arg1_Replacement *= Args[i].modif1;
				break;
			case 2:
				Arg1_Replacement += Args[i].modif1;
				break;
			case 3:
				Arg1_Replacement -= Args[i].modif1;
				break;
			case 4:
				Arg1_Replacement /= Args[i].modif1;
				break;
			}

			switch (Args[i].mtype2)
			{
			case 1:
				Arg2_Replacement *= Args[i].modif2;
				break;
			case 2:
				Arg2_Replacement += Args[i].modif2;
				break;
			case 3:
				Arg2_Replacement -= Args[i].modif2;
				break;
			case 4:
				Arg2_Replacement /= Args[i].modif2;
				break;
			}

			switch(Args[i].type)
			{
			case 2: // <=
				if(Arg1_Replacement >  Arg2_Replacement)
					return false;
				break;
			case 3: // <
				if(Arg1_Replacement >= Arg2_Replacement)
					return false;
				break;
			case 4: // >=
				if(Arg1_Replacement <  Arg2_Replacement)
					return false;
				break;
			case 5: // >
				if(Arg1_Replacement <= Arg2_Replacement)
					return false;
				break;
			case 6: // !=
				if(Arg1_Replacement == Arg2_Replacement)
					return false;
				break;
			case 7: // =
				if(Arg1_Replacement != Arg2_Replacement)
					return false;
				break;
			default:
				return false;
			}
		}
	}
	return true; // all arguments passed
}

void EventHandler::Execute(size_t PlayerIndex, Sapp_Event EventType, const vector <pair<wstring, wstring>> &SpecialArguments)
{
	if (GlobalPrefs->Lua && !GlobalPrefs->ScrimMode)
	{
		lua::EventCallback(EventType, PlayerIndex, SpecialArguments);
	}
	for (size_t i = 0; i < EventCache[EventType].size(); i++)
	{
		if (ToExecute(PlayerIndex, EventCache[EventType][i]->EventArgs, SpecialArguments))
		{							
			wstring CCommand = EventCache[EventType][i]->EventCommand;
			WStrReplace(PlayerIndex, CCommand);
			if(!SpecialArguments.empty())
			{
				for(auto &it : SpecialArguments)
				{
					StringReplace(CCommand, it.first, it.second);
				}
			}

			CommandEnviroment env(Enviroment::SCRIPT);
			ExecuteSequence(CCommand, env);

			stats::NumberOfExecutedEvents++;
		}
	}
}

//wstring BuildArgString(const vector <EventArgsStruct> &EventArgs)
//{
//	wstring line;
//	for(size_t i = 0; i < EventArgs.size(); i++)
//	{
//		wstring type;
//		switch (EventArgs[i].type)
//		{
//			case 0: type = L"!:"; break;
//			case 1: type = L":"; break;
//			case 2: type = L"<="; break;
//			case 3: type = L"<"; break;
//			case 4: type = L">="; break;
//			case 5: type = L">"; break;
//			case 6: type = L"!="; break;
//			case 7: type = L"="; break;
//		}
//		line += FormatWArgs(L"%ls%ls%ls ", EventArgs[i].Arg1.c_str(), type.c_str(), EventArgs[i].Arg2.c_str());
//	}
//
//	return line;
//}

wostringstream EventHandler::StreamOut()
{
	wostringstream os;
	if(SappEventQueue.empty())
	{
		os << L"There are no events." << endl;
	}
	else
	{
		for(size_t i = 0; i < SappEventQueue.size(); i++)
		{
			//os << FormatWArgs(L"%2d %ls %ls{%ls}", i, SappEventQueue[i]->Name, BuildArgString(SappEventQueue[i]->EventArgs).c_str(), SappEventQueue[i]->EventCommand.c_str()) << endl;
			os << i << L" " << SappEventQueue[i]->cmd_line << endl;
		}
	}	
	return os;
}

EventHandler GlobalEventHandler;