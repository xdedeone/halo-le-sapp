#include "Addresses.h"

#include "StringCrypter.h"

unsigned char ntdll_str[] = { 0x91, 0x8b, 0x9b, 0x93, 0x93, 0xd1, 0x9b, 0x93, 0x93, 0xff }; // "ntdll.dll"
unsigned char user32_str[] = { 0x8a, 0x8c, 0x9a, 0x8d, 0xcc, 0xcd, 0xd1, 0x9b, 0x93, 0x93, 0xff }; // "user32.dll"
unsigned char ntsetinformationthread_str[] = { 0xb1, 0x8b, 0xac, 0x9a, 0x8b, 0xb6, 0x91, 0x99, 0x90, 0x8d, 0x92, 0x9e, 0x8b, 0x96, 0x90, 0x91, 0xab, 0x97, 0x8d, 0x9a, 0x9e, 0x9b, 0xff }; // "NtSetInformationThread"
unsigned char ntqueryinformationprocess_str[] = { 0xb1, 0x8b, 0xae, 0x8a, 0x9a, 0x8d, 0x86, 0xb6, 0x91, 0x99, 0x90, 0x8d, 0x92, 0x9e, 0x8b, 0x96, 0x90, 0x91, 0xaf, 0x8d, 0x90, 0x9c, 0x9a, 0x8c, 0x8c, 0xff }; // "NtQueryInformationProcess"
//unsigned char ollydbg_str[] = { 0x90, 0x93, 0x93, 0x86, 0x9b, 0x9d, 0x98, 0xff }; // "ollydbg"
//unsigned char cheatengine_str[] = { 0x9c, 0x97, 0x9a, 0x9e, 0x8b, 0x9a, 0x91, 0x98, 0x96, 0x91, 0x9a, 0xff }; // "cheatengine"

HMODULE hhalo = GetModuleHandle(0);
HMODULE hntdll = GetModuleHandle(CryptExtractor(StringCrypt(ntdll_str, sizeof(ntdll_str))));
HMODULE huser32 = GetModuleHandle(CryptExtractor(StringCrypt(user32_str, sizeof(user32_str))));
HANDLE cProc = GetCurrentProcess();

pNtSetInformationThread NtSIT = (pNtSetInformationThread)GetProcAddress(hntdll, CryptExtractor(StringCrypt(ntsetinformationthread_str, sizeof(ntsetinformationthread_str))));
pNtQueryInformationProcess NtQIP = (pNtQueryInformationProcess)GetProcAddress(hntdll, CryptExtractor(StringCrypt(ntqueryinformationprocess_str, sizeof(ntqueryinformationprocess_str))));

DWORD PrintConsoleFunc;

DWORD* VTable = NULL;
DWORD* NullMe = NULL;
bool KillMe = false;
float* GameSpeed = NULL;
float* Gravity = NULL;

int TimeLeft = 1;

unsigned char hHash[32];

bool SappServer = false;
bool ACServer = false;
char ACPlayers[16][32];

ACJoinInfo Info;

DWORD ServerApp = 0;
DWORD ServerAppVersion = 0;

const float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
const float yellow[4] = { 1.0f, 1.0f, 1.0f, 0.4f };
const float red[4] = { 1.0f, 1.0f, 0.0f, 0.0f };
const float green[4] = { 1.0f, 0.0f, 1.0f, 0.0f };
const float blue[4] = { 1.0f, 0.0f, 0.4f, 1.0f };