#pragma once

#include <Windows.h>

#include <memory>
#include <queue>
#include <string>
#include <thread>

#include "Locker.h"
#include "TextUtilities.h"

struct Log_Struct
{
	std::string LogName;
	std::string LogString;
	size_t retries = 0;
};

class Logger
{
	HANDLE hLogSemaphore;

	std::unique_ptr<std::thread> my_thread;

	std::queue <Log_Struct*> LogQueue;
	Locker LogQueueLock;

	bool ShutDown = false;

	void LogThread()
	{
		Log_Struct* LogEntry = NULL;

		while (!ShutDown)
		{
			WaitForSingleObject(hLogSemaphore, INFINITE);

			LogQueueLock.Lock();
			if (!LogQueue.empty())
			{
				LogEntry = LogQueue.front();
				LogQueue.pop();
			}
			LogQueueLock.Unlock();

			if (LogEntry != NULL)
			{
				FILE* logfile;
				if (!fopen_s(&logfile, LogEntry->LogName.c_str(), "a+") && logfile)
				{
					SYSTEMTIME st = {};
					GetLocalTime(&st);

					std::string LogStr = FormatArgs("%04hu-%02hu-%02hu %02hu:%02hu:%02hu  %hs", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, LogEntry->LogString.c_str());
					fprintf_s(logfile, "%hs\n", LogStr.c_str());

					fclose(logfile);

					delete LogEntry;
				}
				else
				{
					if(++LogEntry->retries < 5)
						LogAdd(LogEntry);
					else
						delete LogEntry;
				}

				LogEntry = NULL;
			}
		}
	}

	void LogAdd(Log_Struct* LogEntry)
	{
		LogQueueLock.Lock();
		LogQueue.push(LogEntry);
		LogQueueLock.Unlock();

		ReleaseSemaphore(hLogSemaphore, 1, 0);
	}

public:

	Logger() : hLogSemaphore(CreateSemaphoreW(NULL, 0, 0x7FFFFFFF, NULL))
	{		
	}

	~Logger()
	{
		ShutDown = true;
		ReleaseSemaphore(hLogSemaphore, 1, 0);
		my_thread->join();
		CloseHandle(hLogSemaphore);
	}

	void Start()
	{
		my_thread = std::make_unique<std::thread>(&Logger::LogThread, this);
	}

	void Log(const std::string &log_name, const std::string &text)
	{
		Log_Struct* LogEntry = new Log_Struct();
		LogEntry->LogName = log_name;
		LogEntry->LogString = text;

		LogAdd(LogEntry);
	}
};
