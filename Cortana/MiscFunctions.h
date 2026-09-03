#pragma once

#include <string>

void HashHalo(unsigned char* hash);
void ListModules();
void ListProcess();
void GetHashesAndMACs();
std::wstring FormatWArgs(const wchar_t* fmt, ...);
void PrintConsole(const char* message, const float* color);