#include "LogThread.h"

#include <queue>

#include "Addresses.h"
#include "HaloUtilities.h"
#include "Locker.h"
#include "Prefs.h"
#include "TextUtilities.h"
#include "Threads.h"

using namespace std;

wstring log_file;
wstring log_name = L"sapp";

ULONG file_lenght;

bool rotating = false;

queue <unique_ptr<LogStr>> LogString;

weak_ptr<work> hLogThread;

Locker LogLock;

void WriteLog(wchar_t* type, wstring&& message)
{
	if (GlobalPrefs->Log)
	{
		unique_ptr<LogStr> NewLog = make_unique<LogStr>(type, move(message));

		LogLock.Lock();

		LogString.push(move(NewLog));

		LogLock.Unlock();

		if (auto sp = hLogThread.lock())
			sp->Boop();
	}
}

bool writelog(const wstring& type, const wstring& string)
{
	FILE* logfile = NULL;
	if (_wfopen_s(&logfile, log_file.c_str(), L"a+, ccs=UTF-16LE") || !logfile)
		return false;

	SYSTEMTIME st;
	GetLocalTime(&st);

	int ret = fwprintf_s(logfile, L"%04hu-%02hu-%02hu %02hu:%02hu:%02hu  %-16ls %ls\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, type.c_str(), string.c_str());

	if (ret > 0)
		file_lenght += ret * sizeof(wchar_t);	

	fclose(logfile);

	return ret > 0;
}

void LogThreadProc(shared_ptr<work>& hThread)
{
	log_file = FormatWArgs(L"%hs\\sapp\\%ls.log", GametypesFolder, log_name.c_str());

	HANDLE hFile = CreateFileW(log_file.c_str(), GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		file_lenght = GetFileSize(hFile, NULL);
		CloseHandle(hFile);
	}
	else
		file_lenght = 2;

	if(rotating)
	{
		rotating = false;
		writelog(L"CONTINUE", L"Log Continued after rotation.");
	}
	else
		writelog(L"OPEN", L"Log Opened");

	while (!hThread->ExitFlag)
	{
		hThread->Wait(INFINITE);

		unique_ptr<LogStr> LogEntry;

		LogLock.Lock();
		if(!LogString.empty())
		{
			LogEntry = move(LogString.front());
			LogString.pop();
		}
		LogLock.Unlock();

		while (LogEntry != nullptr)
		{
			size_t attempts = 25;
			if (writelog(LogEntry->Type, LogEntry->LogString))
			{
				if(GlobalPrefs->LogRotation != 0 && file_lenght > GlobalPrefs->LogRotation)
				{
					hThread->ExitFlag = true;
					rotating = true;
				}
				break;
			}
			else if (--attempts)
			{
				Sleep(200);
			}
			else break;
		}
	}

	writelog(L"CLOSE", L"Log Closed");

	if (rotating)
	{
		SYSTEMTIME st = {};
		GetLocalTime(&st);

		wstring arch_name = FormatWArgs(L"%hs\\sapp\\%ls %04hu-%02hu-%02hu (%02hu_%02hu_%02hu) archive.log", GametypesFolder, log_name.c_str(), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		if (_wrename(log_file.c_str(), arch_name.c_str()))
		{
			GlobalPrefs->LogRotation = 0;
			writelog(L"ROTATION FAIL", L"Failed to rename the log file, rotation is now disabled.");
		}

		hLogThread = ThreadPool.NewWork(LogThreadProc);
	}
}

bool StartLogThread()
{
	if (auto sp = hLogThread.lock())
	{
		return false;
	}
	else
	{
		hLogThread = ThreadPool.NewWork(LogThreadProc);
		return true;
	}
}
bool StopLogThread()
{
	if (auto sp = hLogThread.lock())
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