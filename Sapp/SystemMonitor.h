#pragma once

#include <cstdint>

class SystemMonitor final
{
	void* hCurrentProcess;

	size_t numberOfCPUs;

	uint64_t prevBusyCurrent = 0;
	uint64_t prevTotalCurrent = 0;

	uint64_t prevBusyTotal = 0;
	uint64_t prevIdleTotal = 0;

	double CalculateCurrentProcessCPULoad(uint64_t busyTicks, uint64_t totalTicks);
	double CalculateTotalCPULoad(uint64_t busyTicks, uint64_t idleTicks);

public:
	SystemMonitor();
	size_t GetMemoryLoad();
	size_t GetCurrentProcessCPULoad();
	size_t GetTotalCPULoad();
};