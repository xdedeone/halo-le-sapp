#include "NetworkThread.h"

#include "HaloUtilities.h"
#include "LogThread.h"
#include "NetworkClient.h"
#include "Prefs.h"
#include "TextUtilities.h"
#include "Threads.h"

const wchar_t* const sapp_updater_file = L"sapp_updater.dll";

std::weak_ptr<work> hNetworkThread;

void NetworkThreadProc(std::shared_ptr<work>& hThread)
{
	bool remove_failed = false;

	HMODULE hm = GetModuleHandleW(sapp_updater_file);
	if(hm)
	{
		HaloThreadLock.Lock();
		FreeLibrary(hm);
		HaloThreadLock.Unlock();

		size_t count = 60;
		while (!hThread->ExitFlag && _wremove(sapp_updater_file) && errno != ENOENT && --count)
		{
			hThread->Wait(1000);
		}

		if(!count)
		{
			remove_failed = true;
			GlobalPrefs->AutoUpdate = false;
		}
		else if (!hThread->ExitFlag)
		{
			HaloThreadLock.Lock();
			{
				std::wstring updateStr = FormatWArgs(L"SAPP has been updated to Version %hs %hs", GlobalPrefs->AppVersion, GlobalPrefs->AppHaloType);
				sv_say(updateStr.c_str());
				sv_say(L"Visit halo.isimaginary.com for more information.");
				WriteLog(L"UPDATE", move(updateStr));
			}
			HaloThreadLock.Unlock();
		}
	}

	GlobalSAPPServer.Start();

#ifdef HALORANK
	if(GlobalPrefs->HaloRank)
	{
		HREventStart Event;

		Event.Type = HR_EVENT_START;
		Event.IP = 0;
		Event.Time = 0;
		strcpy_s(Event.ServerName, 64, WToStr(Server->ServerName).c_str());

		GlobalSAPPServer.SendHREvent(&Event, sizeof(HREventStart));
	}
#endif

	DWORD Timer = 0;
	while (!hThread->ExitFlag)
	{
		if (!GlobalSAPPServer.IsConnected())
		{
			hThread->Wait(1000);
		}
		else
		{
			if (!Timer)
				GlobalSAPPServer.VersionCheck();

			Timer = (Timer + 1) % 15;

			GlobalSAPPServer.SendStats();

			if (UpdateAvalibe && remove_failed)
			{
				UpdateAvalibe = false;
				HaloThreadLock.Lock();
				PrintConsole(12, "Auto-Update failed; please update SAPP manually from halo.isimaginary.com");
				sv_say(L"Auto-Update failed; please update SAPP manually from halo.isimaginary.com");
				HaloThreadLock.Unlock();
			}

			hThread->Wait(60000);
		}
	}

#ifdef HALORANK
	if(GlobalPrefs->HaloRank)
	{
		HREventEnd Event;

		Event.Type = HR_EVENT_END;
		Event.IP = 0;
		Event.Time = 0;
		Event.Reserved = 0;

		GlobalSAPPServer.SendHREvent(&Event, sizeof(HREventEnd));
	}
#endif

	GlobalSAPPServer.Stop();
}

bool StartNetworkThread()
{
	if (auto sp = hNetworkThread.lock())
	{
		return false;
	}
	else
	{
		hNetworkThread = ThreadPool.NewWork(NetworkThreadProc);
		return true;
	}
}
bool StopNetowrkThread()
{
	if (auto sp = hNetworkThread.lock())
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