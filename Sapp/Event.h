#pragma once

#include <map>
#include <memory>
#include <vector>

#include "Events.h"

struct EventArgsStruct
{
	unsigned char type;
	unsigned char Var;
	float ArgNum;	
	unsigned char mtype1;
	unsigned char mtype2;
	float modif1;
	float modif2;
	std::wstring Arg1;
	std::wstring Arg2;
};

class SappEvent
{
	friend class EventHandler;

	Sapp_Event type;

	std::vector <EventArgsStruct> EventArgs;
	std::wstring cmd_line;
	std::wstring EventCommand;
public:
	SappEvent(Sapp_Event type, const std::wstring &line, const std::wstring &commmand) : type(type), cmd_line(line), EventCommand(commmand) {}
};

class EventHandler
{
	std::map <std::wstring, Sapp_Event> SappEventsCache;
	std::vector <std::shared_ptr<SappEvent>> SappEventQueue;
	std::vector <std::shared_ptr<SappEvent>> EventCache[SAPP_EVENT_COUNT];
	std::wstring event_file;

public:
	EventHandler();
	void ParseEvent(std::wstring& line);
	bool EventRemove(size_t EventIndex);
	bool ProcessArgument(const std::wstring &EventArg, EventArgsStruct &Args);
	bool ToExecute(size_t PlayerIndex, const std::vector <EventArgsStruct> &Args, const std::vector <std::pair<std::wstring, std::wstring>> &SpecArgs);
	void Execute(size_t PlayerIndex, Sapp_Event EventType, const std::vector <std::pair<std::wstring, std::wstring>> &SpecialArguments);
	void LoadEventsCache();
	void LoadEvents();
	void Clear();
	std::wostringstream StreamOut();
};

extern EventHandler GlobalEventHandler;