#pragma once

#include <Winbase.h>

class PerfomanceCounter
{
	DWORD PCFreq;
	LONGLONG QPC = 0;
public:
	PerfomanceCounter()
	{
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);
		PCFreq = DWORD(li.QuadPart) / 1000;
	}

	void StartQPC()
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		QPC = li.QuadPart;
	}

	DWORD GetQPC()
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return DWORD(li.QuadPart - QPC) / PCFreq;
	}
};