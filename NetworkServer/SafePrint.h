#pragma once

#include <Windows.h>

#include <memory>
#include <queue>
#include <string>
#include <thread>

#include "Locker.h"

class Safe_Print
{
	HANDLE hPrintSemaphore;

	std::unique_ptr<std::thread> my_thread;

	Locker ConsoleLock;

	std::queue <std::string> ConsolePrintQueue;
	Locker ConsolePrintQueueLock;

	bool ShutDown = false;

	void PrintThread()
	{
		std::string msg;

		while (!ShutDown)
		{
			WaitForSingleObject(hPrintSemaphore, INFINITE);

			ConsolePrintQueueLock.Lock();
			if (!ConsolePrintQueue.empty())
			{
				msg = ConsolePrintQueue.front();
				ConsolePrintQueue.pop();
			}
			ConsolePrintQueueLock.Unlock();

			if (!msg.empty())
			{
				ConsoleLock.Lock();
				printf("%hs\n", msg.c_str());
				ConsoleLock.Unlock();
			}
		}
	}

public:

	Safe_Print() : hPrintSemaphore(CreateSemaphoreW(NULL, 0, 0x7FFFFFFF, NULL))
	{
	}

	~Safe_Print()
	{
		ShutDown = true;
		ReleaseSemaphore(hPrintSemaphore, 1, 0);
		my_thread->join();
		CloseHandle(hPrintSemaphore);
	}

	void Start()
	{
		my_thread = std::make_unique<std::thread>(&Safe_Print::PrintThread, this);
	}

	void Print(const std::string &text)
	{
		ConsolePrintQueueLock.Lock();
		ConsolePrintQueue.push(text);
		ConsolePrintQueueLock.Unlock();

		ReleaseSemaphore(hPrintSemaphore, 1, 0);
	}

	void ManualLockConsole()
	{
		ConsoleLock.Lock();
	}

	void ManualUnlockConsole()
	{
		ConsoleLock.Unlock();
	}
};