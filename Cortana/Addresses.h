#pragma once

#include "Threads.h"

typedef NTSTATUS(NTAPI *pNtSetInformationThread)(HANDLE, UINT, PVOID, ULONG);
typedef NTSTATUS(WINAPI *pNtQueryInformationProcess)(HANDLE, UINT, PVOID, ULONG, PULONG);

struct ACJoinInfo
{
	DWORD ServerID;
	DWORD IP;
	WORD Port;
	wchar_t Name[12];
};

extern DWORD PrintConsoleFunc;

extern HMODULE hhalo;
extern HMODULE hntdll;
extern HMODULE huser32;
extern HANDLE cProc;
extern pNtSetInformationThread NtSIT;
extern pNtQueryInformationProcess NtQIP;
extern DWORD* NullMe;
extern DWORD* VTable;
extern bool KillMe;
extern float* GameSpeed;
extern float* Gravity;

extern int TimeLeft;

extern unsigned char hHash[32];

extern bool SappServer;
extern bool ACServer;
extern char ACPlayers[16][32];

extern ACJoinInfo Info;

extern DWORD ServerApp;
extern DWORD ServerAppVersion;

extern const float white[4];
extern const float yellow[4];
extern const float red[4];
extern const float green[4];
extern const float blue[4];