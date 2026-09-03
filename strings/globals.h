#pragma once

#include <Windows.h>

#include "Patch.h"
#include "Signatures.h"

typedef int (__stdcall *CHandlerPROC)(const char* command);

extern HMODULE hModule;

extern PatchContainer Patches;

extern DWORD HaloPrintAddress;

void PrintConsoleAsm(char* buffer);

//void FPrintConsoleAsm(char* format, char* buffer);