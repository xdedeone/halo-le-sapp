#pragma once

#include "SuperHeader.h"

extern int PlayerIndexes[16];
extern int MachineIndexes[16];

extern unsigned short RemotePort;

extern ServerInfoStruct** ServerPointer;
extern ServerInfoStruct* Server;

extern Static_Player_Header* PlayerHeader;
extern Player_Table_Item* PlayerTable;

extern Player_Hash_List* PlayerHashList;

extern Object_Table_Header* ObjectTableHeader;
extern Object_Table_Array* ObjectTable;

extern Tag_Table_Header* TagTableHeader;
extern Tag_Entry* TagTable;

extern Update_Server_Queues_Table_Header** pUpdateServerQueuesTableHeader;

extern DWORD BaseAddr;

extern DWORD HaloPrintAddress;
extern DWORD HaloRconPrintAddress;
extern DWORD HaloExecuteAddress;

extern DWORD Version;

extern DWORD IP;
extern DWORD Port;

extern DWORD GameTicksAddress;
extern DWORD* GameTicks;

extern DWORD* ptrMSData;

extern char* HaloVersionStr;
extern char* HaloDNSStr;

extern char*** CurrentGTPointer;

extern ULONGLONG* UnlockConsole;

extern int* ScoreLimit;
extern Slayer_Scores* SlayerScores;
extern CTF_Scores* CTFScores;
extern Oddball_Scores* OddballScores;
extern King_Scores* KingScores;
extern Race_Scores* RaceScores;

extern int* ScoreType;

extern char* GametypesFolder;
extern char* Banlist;
extern char* RconPassword;
extern bool* TeamGame;
extern int* MapcycleTimeout;
extern char* Motd;

extern int* Timelimit;

extern float* GameSpeed;
extern float* Gravity;

extern DWORD VarBspIntersect;

extern DWORD ADDR_OBJECTBASE;
extern DWORD ADDR_GAMESTATE;
extern DWORD FUNC_BUILDPACKET;
extern DWORD FUNC_BUILDPACKET_PROJWRAPPER;
extern DWORD FUNC_ADDPACKETTOQUEUE;
extern DWORD FUNC_ADDPACKETTOPQUEUE;
extern DWORD FUNC_DOINVIS;
extern DWORD FUNC_INITPROJ;
extern DWORD FUNC_CREATEOBJECT;
extern DWORD FUNC_CREATEOBJECTQUERY;
extern DWORD FUNC_DESTROYOBJECT;
extern DWORD FUNC_ENTERVEHICLE;
extern DWORD FUNC_EJECTVEHICLE;
extern DWORD FUNC_PLAYERASSIGNWEAPON;
extern DWORD FUNC_NOTIFY_WEAPONPICKUP;
extern DWORD FUNC_DROPWEAPON_1;
extern DWORD FUNC_DROPWEAPON_2;
extern DWORD FUNC_ONPLAYERDEATH;
extern DWORD FUNC_ACTIONDEATH_1;
extern DWORD FUNC_ACTIONDEATH_2;
extern DWORD FUNC_ACTIONDEATH_3;
extern DWORD FUNC_UPDATEAMMO;
extern DWORD FUNC_INTERACTWITHPOWERUP;

extern DWORD MD5_Update;
extern DWORD MD5_Final;

typedef int(*TimeLeftFunc)();
extern TimeLeftFunc GetTimeLeft;

typedef void(*ReloadGametypesFunc)();
extern ReloadGametypesFunc ReloadGametypes;

void FindAddresses();