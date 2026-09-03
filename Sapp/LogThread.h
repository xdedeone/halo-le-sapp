#pragma once

#include <string>

extern std::wstring log_file;
extern std::wstring log_name;

bool StartLogThread();
void ReStartLogThread();
bool StopLogThread();

void WriteLog(wchar_t* type, std::wstring&& message);
