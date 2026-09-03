#include "Threads.h"

using namespace std;

weak_ptr<work> thread_pool::NewWork(const function<void(shared_ptr<work>&)>& Func, void* Params)
{
	if (!ShutDown)
	{
		shared_ptr<work> new_work = make_shared<work>(Func, Params);
		WorkLock.Lock();
		WorkQueue.push_back(new_work);
		WorkLock.Unlock();
		ReleaseSemaphore(wSemaphore, 1, NULL);
		return new_work;
	}
	return weak_ptr<work>();
}

void thread_pool::AddTimer(std::unique_ptr<TimerStructBase>&& TimerParams, WAITORTIMERCALLBACK Callback)
{
	Lock_Object guard(TimerLock);
	CreateTimerQueueTimer(&TimerParams->hCommandTimer, hTimerQueue, Callback, TimerParams.get(), TimerParams->DueTime, 0, WT_EXECUTEONLYONCE);
	TimersRunning.emplace(move(TimerParams));
}

void thread_pool::UpdateTimer(TimerStructBase* TimerParams, WAITORTIMERCALLBACK Callback)
{
	DeleteTimerQueueTimer(hTimerQueue, TimerParams->hCommandTimer, NULL);
	CreateTimerQueueTimer(&TimerParams->hCommandTimer, hTimerQueue, Callback, TimerParams, TimerParams->DueTime, 0, WT_EXECUTEONLYONCE);
}

void thread_pool::DeleteTimer(TimerStructBase* TimerParams)
{
	Lock_Object guard(TimerLock);
	DeleteTimerQueueTimer(hTimerQueue, TimerParams->hCommandTimer, NULL);
	auto it = find_if(TimersRunning.begin(), TimersRunning.end(), [&TimerParams](const unique_ptr<TimerStructBase>& it)
	{
		return it.get() == TimerParams;
	});
	TimersRunning.erase(it);
}

void thread_pool::DeleteTimerGroup(DWORD GroupID)
{
	Lock_Object guard(TimerLock);
	auto it = TimersRunning.begin();
	while (it != TimersRunning.end())
	{
		if ((*it)->GroupID == GroupID)
		{
			DeleteTimerQueueTimer(hTimerQueue, (*it)->hCommandTimer, NULL);
			it = TimersRunning.erase(it);
		}
		else ++it;
	}
}

void thread_pool::StartAll(size_t count)
{
	hTimerQueue = CreateTimerQueue();

	for (size_t i = 0; i < count; i++)
	{
		threads.emplace_back((HANDLE)_beginthreadex(0, 0, thread_pool::StartThreadFunc, this, 0, 0));
	}
}

void thread_pool::StopAll()
{
	ShutDown = true;
	
	ReleaseSemaphore(wSemaphore, threads.size(), NULL);

	WorkLock.Lock();
	for (auto &it : WorkInProgress)
	{
		it->ExitFlag = true;
		it->Boop();
	}
	WorkLock.Unlock();

	WaitForMultipleObjects(threads.size(), threads.data(), TRUE, INFINITE);

	for (auto &it : threads)
	{
		CloseHandle(it);
	}
	threads.clear();

	DeleteTimerQueueEx(hTimerQueue, INVALID_HANDLE_VALUE);

	TimersRunning.clear();

	WorkQueue.clear();

	while (WAIT_OBJECT_0 == WaitForSingleObject(wSemaphore, 0));

	ShutDown = false;
}

void thread_pool::ThreadFunc()
{
	while (true)
	{
		WaitForSingleObject(wSemaphore, INFINITE);
		if (!ShutDown)
		{
			WorkLock.Lock();
			std::shared_ptr<work> w = move(WorkQueue.front());
			WorkQueue.pop_front();
			WorkInProgress.emplace(w);
			WorkLock.Unlock();

			w->WorkFunc(w);

			WorkLock.Lock();
			WorkInProgress.erase(w);
			WorkLock.Unlock();
		}
		else break;
	}
}

unsigned __stdcall thread_pool::StartThreadFunc(void* param)
{
	static_cast<thread_pool*>(param)->ThreadFunc();
	_endthreadex(0);
	return 0;
}

thread_pool ThreadPool;