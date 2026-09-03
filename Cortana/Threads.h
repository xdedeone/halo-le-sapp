#pragma once

#include <Windows.h>
#include <process.h>

#include "Main.h"

extern int DeathClock;

void DeathClockThread(LPVOID);

void ScanThread(LPVOID);

void EventThread(LPVOID);