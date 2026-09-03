#pragma once

#include "SuperHeader.h"

enum class ChecksumResult { CHECKSUM_SUCCESS, INVALID_HANDLE, FILE_SIZE_FAIL, MAPPING_FAIL, MAP_VIEW_FAIL, BAD_OFFSETS };

extern DWORD* MapTableSize;
extern DWORD* MapTableMaxSize;
extern MapEntry** MapTable;
extern MapEntry* SappMapTable;

bool FindMap(const char* name);
ChecksumResult Checksum(const char* filename, unsigned long& store);
void AddMap(const char* name, unsigned long checksum);

void InstallMapTableHandler();
void UninstallMapTableHandler();