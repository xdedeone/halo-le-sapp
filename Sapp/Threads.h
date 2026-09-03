#pragma once

#include <process.h>
#include <Windows.h>

#include <algorithm>
#include <deque>
#include <functional>
#include <memory>
#include <set>
#include <thread>
#include <vector>

#include "Locker.h"
#include "SuperHeader.h"

class work
{
	friend class thread_pool;
	HANDLE hSemaphore;
	std::function<void(std::shared_ptr<work>&)> WorkFunc;
	void* WorkParams;

public:
	bool ExitFlag = false;

	work(const std::function<void(std::shared_ptr<work>&)> &Func, void* Params = nullptr) : hSemaphore(CreateSemaphoreW(NULL, 0, 0x7FFFFFFF, NULL)), WorkFunc(Func), WorkParams(Params)
	{
	}
	~work()
	{
		CloseHandle(hSemaphore);
	}

	void* GetParams() const
	{
		return WorkParams;
	}

	void Wait(DWORD time)
	{
		WaitForSingleObject(hSemaphore, time);
	}

	void Boop()
	{
		ReleaseSemaphore(hSemaphore, 1, 0);
	}
};

class thread_pool
{
	HANDLE wSemaphore = CreateSemaphoreW(NULL, 0, 0x7FFFFFFF, NULL);

	std::vector<HANDLE> threads;

	std::deque <std::shared_ptr<work>> WorkQueue;
	std::set <std::shared_ptr<work>> WorkInProgress;
	Locker WorkLock;

	std::set<std::unique_ptr<TimerStructBase>> TimersRunning;
	Locker TimerLock;

	HANDLE hTimerQueue = INVALID_HANDLE_VALUE;

	bool ShutDown = false;
	
	void ThreadFunc();

	static unsigned __stdcall StartThreadFunc(void* param);

public:

	std::weak_ptr<work> NewWork(const std::function<void(std::shared_ptr<work>&)>& Func, void* Params = nullptr);
	void AddTimer(std::unique_ptr<TimerStructBase>&& TimerParams, WAITORTIMERCALLBACK Callback);
	void UpdateTimer(TimerStructBase* TimerParams, WAITORTIMERCALLBACK Callback);
	void DeleteTimer(TimerStructBase* TimerParams);
	void DeleteTimerGroup(DWORD GroupID);
	void StartAll(size_t count);
	void StopAll();
	bool IsShuttingDown() { return ShutDown; }

	~thread_pool()
	{
		CloseHandle(wSemaphore);
	}
};

extern thread_pool ThreadPool;