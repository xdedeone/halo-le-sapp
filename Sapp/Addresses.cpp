#include "Addresses.h"

#include "MapLoader.h"
#include "NetworkHook.h"
#include "Patcher.h"

int PlayerIndexes[16];
int MachineIndexes[16];

unsigned short RemotePort;

ServerInfoStruct** ServerPointer = nullptr;
ServerInfoStruct* Server = nullptr;

Static_Player_Header* PlayerHeader;
Player_Table_Item* PlayerTable;

Player_Hash_List* PlayerHashList;

Object_Table_Header* ObjectTableHeader;
Object_Table_Array* ObjectTable;

Tag_Table_Header* TagTableHeader;
Tag_Entry* TagTable;

Update_Server_Queues_Table_Header** pUpdateServerQueuesTableHeader;

DWORD BaseAddr;

DWORD HaloPrintAddress;
DWORD HaloRconPrintAddress;
DWORD HaloExecuteAddress;

DWORD Version;

DWORD IP;
DWORD Port;

DWORD GameTicksAddress;
DWORD* GameTicks;

DWORD* ptrMSData;

char* HaloVersionStr;
char* HaloDNSStr;

char*** CurrentGTPointer;

ULONGLONG* UnlockConsole = nullptr;

int* ScoreLimit; // score limit for all gametypes (both ffa and team games) except ctf and oddball
Slayer_Scores* SlayerScores;
CTF_Scores* CTFScores;
Oddball_Scores* OddballScores;
King_Scores* KingScores;
Race_Scores* RaceScores;

int* ScoreType;

char* GametypesFolder;
char* Banlist;
char* RconPassword;
bool* TeamGame;
int* MapcycleTimeout;
char* Motd;

int* Timelimit;

float* GameSpeed;
float* Gravity;

DWORD VarBspIntersect;

DWORD ADDR_OBJECTBASE;
DWORD ADDR_GAMESTATE;
DWORD FUNC_BUILDPACKET;
DWORD FUNC_BUILDPACKET_PROJWRAPPER;
DWORD FUNC_ADDPACKETTOQUEUE;
DWORD FUNC_ADDPACKETTOPQUEUE;
DWORD FUNC_DOINVIS;
DWORD FUNC_INITPROJ;
DWORD FUNC_CREATEOBJECT;
DWORD FUNC_CREATEOBJECTQUERY;
DWORD FUNC_DESTROYOBJECT;
DWORD FUNC_ENTERVEHICLE;
DWORD FUNC_EJECTVEHICLE;
DWORD FUNC_PLAYERASSIGNWEAPON;
DWORD FUNC_NOTIFY_WEAPONPICKUP;
DWORD FUNC_DROPWEAPON_1;
DWORD FUNC_DROPWEAPON_2;
DWORD FUNC_ONPLAYERDEATH;
DWORD FUNC_ACTIONDEATH_1;
DWORD FUNC_ACTIONDEATH_2;
DWORD FUNC_ACTIONDEATH_3;
DWORD FUNC_UPDATEAMMO;
DWORD FUNC_INTERACTWITHPOWERUP;

DWORD MD5_Update;
DWORD MD5_Final;

TimeLeftFunc GetTimeLeft;
ReloadGametypesFunc ReloadGametypes;

const unsigned char nops[9] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

void FindAddresses()
{
	Patches.NewPatch(new FunctionAddress("RconPrint function", RconPrintCode, 0, &HaloRconPrintAddress));
	Patches.NewPatch(new FunctionAddress("ExecuteCommand function", ExecuteCode, 0, &HaloExecuteAddress));
	Patches.NewPatch(new FunctionAddress("Intersect function", func_intersect, 0, &VarBspIntersect));
	Patches.NewPatch(new FunctionAddress("BuildPacket function", func_buildpacket, 0, &FUNC_BUILDPACKET));
	Patches.NewPatch(new FunctionAddress("AddPacketToQueue function", func_addpackettoqueue, -56, &FUNC_ADDPACKETTOQUEUE));
	Patches.NewPatch(new FunctionAddress("AddPacketToPQueue function", func_addpackettopqueue, 0, &FUNC_ADDPACKETTOPQUEUE));
	Patches.NewPatch(new FunctionAddress("Invisible function", func_doinvis, 0, &FUNC_DOINVIS));
	Patches.NewPatch(new FunctionAddress("CreateObject function", func_createobject, 0, &FUNC_CREATEOBJECT));
	Patches.NewPatch(new FunctionAddress("CreateObjectQuery function", func_createobjectquery, 0, &FUNC_CREATEOBJECTQUERY));
	Patches.NewPatch(new FunctionAddress("DestroyObject function", func_destroyobject, 0, &FUNC_DESTROYOBJECT));
	Patches.NewPatch(new FunctionAddress("EnterVehicle function", func_entervehicle, 0, &FUNC_ENTERVEHICLE));
	Patches.NewPatch(new FunctionAddress("EjectVehicle function", func_ejectvehicle, 0, &FUNC_EJECTVEHICLE));
	Patches.NewPatch(new FunctionAddress("AssignWeapon function", func_playerassignweapon, 0, &FUNC_PLAYERASSIGNWEAPON));
	Patches.NewPatch(new FunctionAddress("NotifyWeaponPickup function", func_notify_weaponpickup, 0, &FUNC_NOTIFY_WEAPONPICKUP));
	Patches.NewPatch(new FunctionAddress("DropWeapon function 1", func_dropweapon_1, 0, &FUNC_DROPWEAPON_1));
	Patches.NewPatch(new FunctionAddress("DropWeapon function 2", func_dropweapon_2, 0, &FUNC_DROPWEAPON_2));
	Patches.NewPatch(new FunctionAddress("OnPlayerDeath function", func_onplayerdeath, 0, &FUNC_ONPLAYERDEATH));
	Patches.NewPatch(new FunctionAddress("ActionDeath1 function", func_actiondeath_1, 0, &FUNC_ACTIONDEATH_1));
	Patches.NewPatch(new FunctionAddress("ActionDeath2 function", func_actiondeath_2, 0, &FUNC_ACTIONDEATH_2));
	Patches.NewPatch(new FunctionAddress("ActionDeath3 function", func_actiondeath_3, 0, &FUNC_ACTIONDEATH_3));
	Patches.NewPatch(new FunctionAddress("UpdateAmmo function", func_updateammo, -117, &FUNC_UPDATEAMMO));
	Patches.NewPatch(new FunctionAddress("InteractWithPowerup function", func_interactwithpowerup, 0, &FUNC_INTERACTWITHPOWERUP));
	Patches.NewPatch(new FunctionAddress("TimeLeft funtion", TimelimitCode, 0, (DWORD*)&GetTimeLeft));
	Patches.NewPatch(new FunctionAddress("ReloadGametypes funtion", ReloadGametypesCode, 0, (DWORD*)&ReloadGametypes));
	Patches.NewPatch(new FunctionAddress("MD5 Update function", md5_update, 0, &MD5_Update));
	Patches.NewPatch(new FunctionAddress("MD5 Final function", md5_final, 0, &MD5_Final));
	Patches.NewPatch(new FunctionAddress("Base Address", BaseAddressCode, 0, &BaseAddr));	
	Patches.NewPatch(new MemoryAddress("VersionString Address", VersionString, 1, (DWORD*)&HaloVersionStr));
	Patches.NewPatch(new MemoryAddress("DNSString Address", DNSString, 1, (DWORD*)&HaloDNSStr));
	Patches.NewPatch(new MemoryAddress("ObjectBase Address", addr_objectbase, 65, &ADDR_OBJECTBASE));
	Patches.NewPatch(new MemoryAddress("GameState Address", addr_gamestate, 2, &ADDR_GAMESTATE));
	Patches.NewPatch(new MemoryAddress("HashList Address", HashListCode, 6, (DWORD*)&PlayerHashList));
	Patches.NewPatch(new MemoryAddress("GameType Address", GameTypePtr, 1, (DWORD*)&CurrentGTPointer));
	Patches.NewPatch(new MemoryAddress("Version Address", VersionCode, 26, &Version));
	Patches.NewPatch(new MemoryAddress("GameType Folder Address", GTFolderCode, 8, (DWORD*)&GametypesFolder));
	Patches.NewPatch(new MemoryAddress("Banlist File Address", BanlistCode, 1, (DWORD*)&Banlist));
	Patches.NewPatch(new MemoryAddress("RconPassword Address", RconPasswordCode, 1, (DWORD*)&RconPassword));
	Patches.NewPatch(new MemoryAddress("TeamGame Address", TeamGameCode, 8, (DWORD*)&TeamGame));
	Patches.NewPatch(new MemoryAddress("MapcycleTimeout Address", MapcycleTimeoutCode, 2, (DWORD*)&MapcycleTimeout));
#ifndef HPC
	Patches.NewPatch(new MemoryAddress("Motd Address", MotdMessageCode, 8, (DWORD*)&Motd));
#endif
	Patches.NewPatch(new MemoryAddress("Timelimit Address", TimelimitCode, 2, (DWORD*)&Timelimit));
	Patches.NewPatch(new MemoryAddress("GameSpeed Address", GameSpeedCode, 2, (DWORD*)&GameSpeed));
	Patches.NewPatch(new MemoryAddress("Gravity Address", GravityCode, 2, (DWORD*)&Gravity));
	//Patches.NewPatch(new MemoryAddress("MaxPlayers Address", MaxPlayersCode, 2, (DWORD*)&MaxPlayers));
#ifndef HPC
	Patches.NewPatch(new MemoryAddress("UnlockConsoleLog Address", UnConLogCode, 2, (DWORD*)&UnlockConsole));
#endif
	Patches.NewPatch(new MemoryAddress("Score Limit Address", ScoreLimitCode, 2, (DWORD*)&ScoreLimit));
	Patches.NewPatch(new MemoryAddress("SlayerScoreTable Address", SlayerScoreTableCode, 9, (DWORD*)&SlayerScores));
	Patches.NewPatch(new MemoryAddress("CTFScoreTable Address", CTFScoreTableCode, 5, (DWORD*)&CTFScores));
	Patches.NewPatch(new MemoryAddress("OddballScoreTable Address", OddballScoreTableCode, 8, (DWORD*)&OddballScores));
	Patches.NewPatch(new MemoryAddress("KingScoreTable Address", KingScoreTableCode, 8, (DWORD*)&KingScores));
	Patches.NewPatch(new MemoryAddress("RaceScoreTable Address", RaceScoreTableCode, 14, (DWORD*)&RaceScores));
	Patches.NewPatch(new MemoryAddress("ScoreType Address", ScoreTypeCode, 1, (DWORD*)&ScoreType));
	Patches.NewPatch(new MemoryAddress("MapTable Address", MapTableCode, 2, (DWORD*)&MapTableSize));
	Patches.NewPatch(new MemoryAddress("UpdateServerQueues Address", MemoryLeakFixCode, 18, (DWORD*)&pUpdateServerQueuesTableHeader));
	Patches.NewPatch(new MemoryAddress("GS Data ptr", ptrMSDataSign, 16, (DWORD*)&ptrMSData));
	Patches.NewPatch(new MemoryValue("ObjectTable Address", ObjectTableCode, 2, (DWORD*)&ObjectTableHeader));
	Patches.NewPatch(new MemoryValue("IP Address", addr_ip, 2, &IP));
	Patches.NewPatch(new MemoryValue("Port Address", addr_port, 3, &Port));

	Patches.NewPatch(new RelativeMemoryValue("InitializeProjectile function", func_buildpacket_projwrapper, -8, &FUNC_INITPROJ));
	Patches.NewPatch(new RelativeMemoryValue("BuildPacketProjectileWrapper function", func_buildpacket_projwrapper, 12, &FUNC_BUILDPACKET_PROJWRAPPER));

	Patches.NewPatch(new CodePatch("MD5 Patch", md5_patch, 9, 0, nops));

	RemotePort = (unsigned short)Port;

	PlayerHeader = **reinterpret_cast<Static_Player_Header***>(BaseAddr + 1);

	PlayerTable = PlayerHeader->FirstPlayer;

	ObjectTable = ObjectTableHeader->FirstObject;

	ServerPointer = *reinterpret_cast<ServerInfoStruct***>(BaseAddr + 8);

	if (*ServerPointer != nullptr)
		Server = *ServerPointer;

	PlayerHashList = reinterpret_cast<Player_Hash_List*>((DWORD)PlayerHashList + 4);

	TagTableHeader = reinterpret_cast<Tag_Table_Header*>(0x40440000);
	TagTable = TagTableHeader->TagTableEntryPointer;

	DWORD oldProtect;
	VirtualProtect(HaloVersionStr, 16, PAGE_READWRITE, &oldProtect);
	memcpy(HaloVersionStr, "01.00.10.0621", 14);
	VirtualProtect(HaloVersionStr, 16, oldProtect, &oldProtect);

	VirtualProtect(HaloDNSStr, 24, PAGE_READWRITE, &oldProtect);
	memcpy(HaloDNSStr, "s1.master.hosthpc.com", 22);
	VirtualProtect(HaloDNSStr, 24, oldProtect, &oldProtect);

	SetDNS("s1.master.hosthpc.com");

	memset(PlayerIndexes, -1, sizeof(PlayerIndexes));
	memset(MachineIndexes, -1, sizeof(MachineIndexes));	
}