#pragma once

#include <set>

#include "Locker.h"
#include "StringCompare.h"

bool StartAliasThread();
bool StopAliasThread();

void AddAlias(std::string&& hash, std::wstring&& name);

void ClearAliases();

Locker& GetAliasLock();
const std::set <std::wstring, WStringICmp>& GetAliasNoLock(const std::string& hash);