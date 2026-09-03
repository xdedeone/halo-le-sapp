#pragma once

#include <map>
#include <string>

#include "SuperHeader.h"

extern std::map <std::string, ObjectUsage> DisabledObjects;
extern std::map <DWORD, ObjectUsage> ObjectCache;

void RemoveIfEmpty(std::map <std::string, ObjectUsage>::iterator it);
void InstallObjectInteractionHandler();