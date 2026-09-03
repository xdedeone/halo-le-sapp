#pragma once

#include <string>
#include <map>

extern std::string NextMap;
extern std::string NextMode;

extern std::map<std::string, std::string> ExtraQueryItems;

void FormatQueryString();

void InstallGSQueryFixHandler();