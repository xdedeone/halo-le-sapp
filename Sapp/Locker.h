#pragma once

#include <Windows.h>

class Locker 
{
	CRITICAL_SECTION g_cs;

public:
	Locker()
	{
		InitializeCriticalSection(&g_cs);
	}

	BOOL TryLock()
	{
		return TryEnterCriticalSection(&g_cs);
	}

	void Lock()
	{		
		EnterCriticalSection(&g_cs);
	}

	void Unlock()
	{	
		LeaveCriticalSection(&g_cs);
	}

	~Locker()
	{
		DeleteCriticalSection(&g_cs);
	}

	Locker(Locker&& other) = delete;
	Locker(const Locker&) = delete;
	Locker& operator=(const Locker&) = delete;
};

class Lock_Object
{
	Locker& lock;
public:
	Lock_Object(Locker& lock) : lock(lock)
	{
		lock.Lock();
	}
	~Lock_Object()
	{
		lock.Unlock();
	}

	Lock_Object(Lock_Object&& other) = delete;
	Lock_Object(const Lock_Object&) = delete;
	Lock_Object& operator=(const Lock_Object&) = delete;
};