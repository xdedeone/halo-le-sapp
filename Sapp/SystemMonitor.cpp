#include "SystemMonitor.h"

#include <Windows.h>

static uint64_t FileTimeToUInt64(const FILETIME& ft)
{
	return ((static_cast<uint64_t>(ft.dwHighDateTime)) << 32) | ft.dwLowDateTime;
}

SystemMonitor::SystemMonitor() : hCurrentProcess(GetCurrentProcess())
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	numberOfCPUs = sysInfo.dwNumberOfProcessors;
}

size_t SystemMonitor::GetMemoryLoad()
{
	MEMORYSTATUSEX memStat;
	memStat.dwLength = sizeof(memStat);
	GlobalMemoryStatusEx(&memStat);
	return memStat.dwMemoryLoad;
}

double SystemMonitor::CalculateCurrentProcessCPULoad(uint64_t busyTicks, uint64_t totalTicks)
{
	double busyTicksSinceLastTime = static_cast<double>(busyTicks - prevBusyCurrent);
	double totalTicksSinceLastTime = static_cast<double>(totalTicks - prevTotalCurrent);

	prevBusyCurrent = busyTicks;
	prevTotalCurrent = totalTicks;

	return busyTicksSinceLastTime / totalTicksSinceLastTime / numberOfCPUs;
}

double SystemMonitor::CalculateTotalCPULoad(uint64_t busyTicks, uint64_t idleTicks)
{
	double busyTicksSinceLastTime = static_cast<double>(busyTicks - prevBusyTotal);
	double idleTicksSinceLastTime = static_cast<double>(idleTicks - prevIdleTotal);

	prevBusyTotal = busyTicks;
	prevIdleTotal = idleTicks;

	return 1.0 - idleTicksSinceLastTime / busyTicksSinceLastTime;
}

size_t SystemMonitor::GetCurrentProcessCPULoad()
{
	FILETIME systemTime, kernelTime, userTime;
	GetProcessTimes(hCurrentProcess, &systemTime, &systemTime, &kernelTime, &userTime);
	GetSystemTimeAsFileTime(&systemTime);
	return static_cast<size_t>(CalculateCurrentProcessCPULoad(FileTimeToUInt64(kernelTime) + FileTimeToUInt64(userTime), FileTimeToUInt64(systemTime)) * 100);
}

size_t SystemMonitor::GetTotalCPULoad()
{
	FILETIME idleTime, kernelTime, userTime;
	GetSystemTimes(&idleTime, &kernelTime, &userTime);
	return static_cast<size_t>(CalculateTotalCPULoad(FileTimeToUInt64(kernelTime) + FileTimeToUInt64(userTime), FileTimeToUInt64(idleTime)) * 100);
}