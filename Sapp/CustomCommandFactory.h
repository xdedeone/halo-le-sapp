#pragma once

#include <string>
#include <vector>

void LoadCustomCommands();

void SaveCustomCommand(const std::wstring& name, const std::wstring& cmd, const std::vector <std::wstring>& args, int level);