#pragma once

#include <Windows.h>

void PerformBaseRelocation(unsigned char *codeBase, PIMAGE_NT_HEADERS headers, size_t base);

void CopySections(const unsigned char *data, unsigned char *codeBase, PIMAGE_NT_HEADERS headers);

void UnCopySections(unsigned char *data, const unsigned char *codeBase, PIMAGE_NT_HEADERS headers);

void XORFuncs(unsigned char* code, PIMAGE_NT_HEADERS header, DWORD magic);