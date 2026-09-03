#pragma once

#include <windows.h>
#include "DynamicCodeXOR.h"

DWORD FindCodeFromHandle(HANDLE hModule, const short* code, size_t length);
DWORD FindCode(DWORD address, size_t size, const short* code, size_t length);