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
	void Lock()
	{
		EnterCriticalSection(&g_cs);
	}
	bool TryLock()
	{
		return !!TryEnterCriticalSection(&g_cs);
	}
	void Unlock()
	{
		LeaveCriticalSection(&g_cs);
	}
	~Locker()
	{
		DeleteCriticalSection(&g_cs);
	}
};