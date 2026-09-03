#pragma once

#include <map>
#include <memory>

#include "SuperHeader.h"

extern std::map <ULONGLONG, std::unique_ptr<PL>> PlayersLeft;
extern std::map <ULONGLONG, std::unique_ptr<PL_Stats>> PlayersLeftStats;
extern Player_Stats PlayerStats[16];
extern size_t LoginFails[16];
extern bool stfu[16];

void InstallMJoinHandler();

void InstallMLeaveHandler();

void InstallCleanMachinesHandler();

void InstallPJoinHandler();