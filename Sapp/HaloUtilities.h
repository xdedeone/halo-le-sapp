#pragma once

#include <string>

#include "SuperHeader.h"

bool ServerRunning();
int IndexToNumber(size_t index);
int NumberToIndex(size_t number);
int ToPlayerIndex(size_t PlayerNumber);
bool PlayerInVehicle(size_t playerIndex);
bool PlayerPresent(size_t i);
bool PlayerAlive(size_t i);
bool PlayerAliveOnly(size_t i);
void GetPlayerPosition(size_t PlayerIndex, vect3f &loc);
size_t GetPlayerNumber();
size_t GetRedPlayerNumber();
size_t GetBluePlayerNumber();
void GetPlayers(int &RealPlayerNumber, size_t *TakenSlotIndexes);
void GetReds(int &RedPlayerNumber, size_t *TakenSlotIndexes);
void GetBlues(int &BluePlayerNumber, size_t *TakenSlotIndexes);
int GetPlayerScore(size_t PlayerIndex);
int GetTeamScore(size_t TeamIndex);
std::string GetPlayerHash(size_t MachineID);
std::string GetPlayerIP(size_t MachineID);
void sv_kick(size_t i);
void sv_ban(size_t i, const char* s);
void FormatQueryString();
void crDirectory(const std::wstring &name);
void crFile(const std::wstring &name);
void PrintConsole(const char* buffer);
void PrintConsole(WORD attrib, const char* buffer);
void PrintRcon(size_t PlayerIndex, const char* buffer);
void PrintRconAsm(size_t PlayerIndex, const char* buffer);
DWORD BuildPacket(DWORD arg1, DWORD packettype, DWORD arg3, LPBYTE dataPtr, DWORD arg4, DWORD arg5, DWORD arg6);
DWORD BuildPacketProjectileWrapper(DWORD ObjectIndex);
void AddPacketToGlobalQueue(DWORD packetCode, DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4, DWORD arg5);
void AddPacketToPlayerQueue(DWORD player, DWORD packetCode, DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4, DWORD arg5);
void BroadcastMotd(const char* message);
void MsgSend(size_t PlayerIndex, const char* message, bool admin = false);
void MsgSend(size_t PlayerIndex, const wchar_t* message, bool admin = false);
void sv_say(const char* message, bool admin = false);
void sv_say(const wchar_t* message, bool admin = false);
void Login(size_t PlayerIndex, int level);
void UpdateSJLevel();