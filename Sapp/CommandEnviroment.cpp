#include "CommandEnviroment.h"

#include "CommandFactory.h"
#include "LuaHandler.h"
#include "Prefs.h"
#include "Replace.h"
#include "RemoteConsole.h"

//const char* const DrunkAdmin = "Are you drunk?! Ignoring command: drunk admin";

bool ScriptEcho = false;

void CommandEnviroment::Print(const char* message) const
{
	switch(type)
	{
	case Enviroment::INIT:
	case Enviroment::CONSOLE:
		PrintConsole(message);
		break;
	case Enviroment::RCON:
		PrintRcon(sPlayerIndex, message);
		break;
	case Enviroment::CHAT:
		MsgSend(sPlayerIndex, message);
		break;
	case Enviroment::REMOTE:
	{
		GlobalRemoteConsole.SendRemoteOutput(message);
		break;
	}
	default:
		if (GlobalPrefs->Lua && !GlobalPrefs->ScrimMode)
		{
			std::vector<std::unique_ptr<lua::LuaEventArgBase>> args;
			args.emplace_back(std::make_unique<lua::LuaEventArgString>(message));
			lua::LuaEventCallback(EVENT_ECHO, sPlayerIndex, args);
		}
	}
}

void CommandEnviroment::Print(const wchar_t* message) const
{
	switch(type)
	{
	case Enviroment::INIT:
	case Enviroment::CONSOLE:
		PrintConsole(WToStr(message).c_str());
		break;
	case Enviroment::RCON:
		PrintRcon(sPlayerIndex, WToStr(message).c_str());
		break;
	case Enviroment::CHAT:
		MsgSend(sPlayerIndex, message);
		break;
	case Enviroment::REMOTE:
	{
		GlobalRemoteConsole.SendRemoteOutput(WToStr(message).c_str());
		break;
	}
	default:
		if (GlobalPrefs->Lua && !GlobalPrefs->ScrimMode)
		{
			std::vector<std::unique_ptr<lua::LuaEventArgBase>> args;
			args.emplace_back(std::make_unique<lua::LuaEventArgString>(WToStr(message)));
			lua::LuaEventCallback(EVENT_ECHO, sPlayerIndex, args);
		}
	}
}

void CommandEnviroment::StreamPrint(const std::wostringstream& wos) const
{
	if (type != Enviroment::SCRIPT || ScriptEcho)
	{
		std::wstring wstr = wos.str();
		size_t last_end = 0;
		for (size_t i = 0; i < wstr.length(); i++)
		{
			if (wstr[i] == '\n')
			{
				std::wstring line = wstr.substr(last_end, i - last_end);
				Print(line.c_str());
				last_end = i + 1;
			}
		}
	}
}
