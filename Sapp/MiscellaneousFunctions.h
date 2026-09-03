#pragma once

#include <string>

#include "SuperHeader.h"
#include "SystemMonitor.h"

extern SystemMonitor GlobalSystemMonitor;

bool ParseIP(const std::string &MaskedIP, size_t &ip, size_t &Mask, size_t &MaskRange);
float Get3DDistance(const vect3f &v1, const vect3f &v2);
float GetCameraAngle(const vect3f &v1, const vect3f &v2);
int GetHaloVersion();
DWORD endian_swap(const DWORD x);
DWORD GetRand(size_t min, size_t max);
void MD5_Hash(const char* code, size_t len, unsigned char* hash);
void __stdcall ProccessorBrandString(char *BStr);
bool FastValidate(int PlayerIndex, int level, const wchar_t* text, size_t len);