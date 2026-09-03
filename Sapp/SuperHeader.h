#pragma once

//#define HPC

#include <Windows.h>

#include <ctime>
#include <string>

#include "NetworkStructs.h"
#include "Vector3f.h"

enum class Enviroment { INIT, CONSOLE, RCON, CHAT, SCRIPT, REMOTE };

struct JoinRequestStruct
{
	DWORD total_requests;
	time_t last_attempt;
};

struct SpawnedVehicle
{
	DWORD ObjectID;
	vect3f Coords;
	float Rotation;
	size_t PlayerIndex;
	SpawnedVehicle(DWORD ObjectID, const vect3f &Coords, float Rotation, size_t PlayerIndex) : ObjectID(ObjectID), Coords(Coords), Rotation(Rotation), PlayerIndex(PlayerIndex) {}
};

struct TimerStructBase
{
	HANDLE hCommandTimer;
	DWORD GroupID;
	DWORD DueTime;
	TimerStructBase(DWORD GroupID, DWORD DueTime) : GroupID(GroupID), DueTime(DueTime) {}
	virtual ~TimerStructBase() {};
};

struct LogStr
{
	std::wstring Type;
	std::wstring LogString;
	LogStr(const wchar_t *t, std::wstring &&s) : Type(t), LogString(std::move(s)) {}
};

struct AliasStr
{
	std::string Hash;
	std::wstring Name;
	AliasStr(std::string &&hash, std::wstring &&name) : Hash(std::move(hash)), Name(std::move(name)) {}
};

struct PL
{
	time_t left;
	time_t joined;
	size_t afk;
	DWORD AimbotScores;
	int LoginFails;
	int RconFails;
	int LoggedInLvl;
	DWORD RespawnTimer;
	DWORD tk_number;
	DWORD tk_timer;
	long Kills;
	long Assists;
	WORD Deaths;
	bool IsAFK;
	bool Lead;
	DWORD CustomLead;
	bool stfu;
};

struct PL_Stats
{
	long Kills;
	long Assists;
	short Deaths;
	short RaceLastLap;
	short RaceScore;
	short RaceFastestLap;
	DWORD RaceLapStart;
	int SlayerScores;
	int OddballScores;
	DWORD RaceFirstFlag;
	DWORD RaceFlags;
};

struct Player_Stats
{
	time_t Joined;
	long Kills;
	long Assists;
	WORD Deaths;
};

struct ObjectUsage
{
	bool Red = false;
	bool Blue = false;
	bool Players[16] = {};
};

#pragma pack(push, 1)

struct MD5_Context
{
	DWORD state[4] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 };
	DWORD count[2] = { 0, 0 };
	unsigned char buffer[64];
};

struct ChatData
{
    DWORD type;
    DWORD player;
    const wchar_t* msg;
};

struct Ping_Table_Item
{
	DWORD PlayerIndex;
	DWORD Ping;
};

struct Tag_Entry
{
	DWORD Class0;
	DWORD Class1;
	DWORD Class2;
	DWORD TagID;
	char* TagName;
	DWORD TagStruct;
	BYTE Unknown[8];
};

struct Tag_Table_Header
{
	Tag_Entry* TagTableEntryPointer;
	DWORD FirstTagID;
	DWORD MapID;
	DWORD TagCount;
	DWORD VerticesCount;
	DWORD VerticesOffset;
	DWORD IndicesCount;
	DWORD IndicesOffset;
	DWORD ModelDataSize;
	DWORD Signature;
};

struct BSPChunkMeta
{
	DWORD offset;
	DWORD size;
};

//Just made this up
struct SceneryTag
{
	DWORD sections; //BSP count?
	DWORD address;
};

struct MapEntry
{
	char* name;
	DWORD id; // 0x00-0x12 : stock maps, 0x13 : CE maps
	DWORD checked; // BYTE (0x01) // BAADF001
#ifndef HPC
	DWORD checksum;
#endif
};

struct MapHeader
{
	long Signature0; // head = "daeh"
	short Game; // Xbox = 5 / Trial = 6 / PC = 7 / CE = 0x261
	short Unknown0;
	long MapSize;
	char Unknown1[4];
	long IndexOffset;
	long MetaSize;
	char Unknown3[8];
	char MapName[14];
	char Unknown4[18];
	char Build[12]; // PC = 01.00.00.0564 / CE = 01.00.00.0609
	char Unknown5[20];
	char MapType; // SP = 0 / MP = 1 / UI = 2
	char Unknown6;
	short Unknown7;
	int checksum;
	char Unknown8[1940];
	long Signature1; // foot = "toof"
};

struct objInfo
{
	DWORD tagType;
	char* tagName;
	DWORD mapObjId;
};

struct ObjectCreationQuery
{
	DWORD map_id;
	char unk1[4];
	union
	{
		DWORD ObjectIndex;
		struct
		{
			WORD ObjectIndexNum;
			WORD ObjectID;
		};
	};
	DWORD parent;
	char unk2[8];
	vect3f pos;
	char unk3[16];
	float RotationVectorX;
	float RotationVectorY;
	float RotationVectorZ;
	float Pitch;
	float Yaw;
	float Roll;
	char unk4[60];
};

// by Oxide
struct s_intersection_output
{
	BYTE hit; // 2 (BSP hit) 3 (unit hit) 0xFF (no hit)
	char unk1[0x17];
	vect3f hit_pos;
	char unk2[0x14];
	union
	{
		DWORD hit_obj;
		struct
		{
			uint16_t ObjectIndexNum;
			uint16_t ObjectID;
		};
	};
	char unk3[0x28];
};

struct Player_Vectors
{
	vect3f Pos;
	vect3f Mov;
	float Pitch;
	float Yaw;
	float Roll;
	float ScaleX;
	float ScaleY;
	float ScaleZ;
	float Unknown[8]; // might not all are floats
};

struct Update_Server_Queues_Table
{
	WORD PlayerID;
	WORD pad;
	char unk1[44];
	unsigned char* ptr480b;
	char unk2[8];
	unsigned char* ptr5280b;
	char unk3[36];
};

struct Update_Server_Queues_Table_Header
{
	BYTE TName[32];
	WORD MaxSlots;
	WORD SlotSize;
	DWORD Unknown;
	BYTE Data[4];
	WORD IsInMainMenu;
	WORD SlotsTaken; 
	WORD NextIndex;
	WORD NextID;
	Update_Server_Queues_Table* First;
};

struct Player_Hash_Data
{
	DWORD id;
	char hash[0x20];
	BYTE unk1[0x14];
	DWORD validation;
	BYTE unk2[0x14];
};

struct Player_Hash_List
{
	Player_Hash_Data* data; // 0 for head of list
	Player_Hash_List* next;
	Player_Hash_List* prev;
};

struct PresenceItem
{
	wchar_t name[12];
	union
	{
		ULONGLONG Prescene; // Init to 0xFFFFFFFFFFFFFFFF
		struct
		{
			short color;
			short icon_id;
			char machine_index;
			char player_status;
			char team;
			char player_index;
		};
	};
};

struct NetInfoStruct
{
	union
	{
		DWORD IP;
		BYTE IPasBytes[4];
	};
	WORD Port;
	WORD Pad;
	DWORD* Socket;
};

struct Player_Machine_Info
{
	NetInfoStruct*** IPAddress;
	DWORD Zero1;
	DWORD Zero2;
	WORD PlayerNumber; // Init to 0xFFFF
	WORD seven;
	BYTE unknown4[24];
	float angle_xy;
	float angle_z;
	BYTE unknown0[44];
	DWORD HashID; // Init to 0xFFFFFFFF
#ifndef HPC
	wchar_t name[16];
	char ip[32];  // at 116
	char cd_key_hash[32];
	BYTE unknown[44]; // 172
#endif	
};

struct ServerInfoStruct
{
	DWORD *SomeAddress;
	WORD ServerState;  // 0 - inactive, 1 - game, 2 - game break\results
	WORD SomeBitmask; // 7 ??
	wchar_t ServerName[66];
	char MapName[128];
	wchar_t GameType[24];
#ifndef HPC
	BYTE unknown2[169];
#else
	BYTE unknown2[105];
#endif
	BYTE MaxPlayers;
	WORD SomeOne;
	WORD PlayerNumber;
	PresenceItem PlayerData[16];
	BYTE unknown4[6];
	DWORD GamesPlayed;
	BYTE unknown5[4];
	Player_Machine_Info MachineInfo[16];
	BYTE unknown6[68];
	wchar_t Password[16];
	DWORD EndPad;
};

struct Slayer_Scores
{
	int TeamScores[16]; // slayer: [0]-[15] real scores of the players, team slayer: [0] - reds, [1] - blues
	int PlayerScores[16]; // slayer scores shown in scores table (F1)
};

struct CTF_Scores
{
	int Reds;
	int Blues;
	int Scorelimit;
};

struct Oddball_Scores
{
	int Scorelimit;
	int TeamScores[16]; // oddball: [0]-[15] real scores of the players, team oddball: [0] - reds, [1] - blues
	int PlayerScores[16]; // oddball scores shown in scores table (F1)
	DWORD BallSpawnTimer[16];
	DWORD BallOwnerPlayer[16];
	DWORD SomeTimestamp[16];
};

struct King_Scores
{
	int TeamScores[16];  // king: [0]-[15] hill time of the players, team king: [0] - reds, [1] - blues
	DWORD LastTimeInTheHill[16];
};

struct Race_Scores
{
	DWORD FlagCount; // Bitfield, e.g. 1 flag: 0x01, 2 flags: 0x03, 3 flags: 0x07, 4 flags: 0x0F, etc.
	DWORD FirstFlagtouched[16]; // Bitfield, only in "Any Order" mode
	DWORD FlagsTouched[16]; // Bitfield
	DWORD FlagToTouch; // Bitfield, only in "Rally" mode
	int TeamScores[16]; // race: [0]-[15] laps of the players, team race: [0] - reds, [1] - blues
};

struct Dynamic_Player;

struct Object_Table_Array
{
	WORD ObjectID; // Matches up to Object ID in static player table ( for players )
	BYTE Unknown0;
	BYTE Unknown1;
	WORD Unknown2;
	WORD Size; // Structure size
	Dynamic_Player* Offset; // Pointer to the object data structure
};

struct Object_Table_Header
{
    BYTE TName[32]; // 'object'
    WORD MaxObjects; // Maximum number of objects - 0x800(2048 objects)
    WORD Size; // Size of each object array - 0x0C(12 bytes)
    DWORD Unknown0; // always 1?
    BYTE Data[4]; // '@t@d' - translates to 'data'?
    WORD NextSlot; // Next slot to be taken with Spawn Object
    WORD NumSlots; // Number of slots in the table
	WORD NumObjects; // Number of objects in the table
	WORD NextObjectID; // ID number of the next object to spawn
	Object_Table_Array* FirstObject; // Pointer to the first object in the table    
};

// players table starts with another 4 bytes
struct Player_Table_Item
{
	WORD PlayerID;            // Starts at 0x70EC
	WORD PlayerID2;            // Local 0 / Not Local -1
	wchar_t name[12];
	DWORD Unknown0; // Always -1 / 0xFFFFFFFF
	DWORD Team;                // 0 = Red / 1 = Blue
	DWORD SwapID;              // ObjectID
	WORD SwapType;           // 8 = Vehicle / 6-7 = Weapon
	short SwapSeat;                    // Warthog - Driver = 0 / Passenger = 1 / Gunner = 2 / Weapon = -1
	DWORD RespawnTimer;        // Counts down when dead, alive = 0
	DWORD RespawnPenalty;
	union 
	{
		uint32_t ObjectIndex;
		struct
		{
			uint16_t ObjectIndexNum;
			uint16_t ObjectID;           // Matches against object table
		};
	};
	DWORD Unknown3;            // Some sort of ID
	DWORD LocationID;          // This is very, very interesting. BG is split into 25 location ID's. 1 -19
	long Unknown4;                     // Always -1 / 0xFFFFFFFF
	DWORD LastBulletTimestamp;         // The current game tick timestamp of the last bullet
	PresenceItem Prescene;
	char unknows3[4];  // 104
	float speed;        // 108
	char unknown10[24]; // 112
	DWORD race_lap_start; // 136
	char unknows4[10];  // 140
	WORD killstreak; // 150
	WORD combo; // 152
	WORD LastKillTimestamp; // 154; truncated, after ~16 min comobos won't be announced
	int kills;         // 156
	long unknown5;      // 160
	int assists;	   // 164
	long unknown11;    //168
	WORD suicides;      // 172
	short deaths;        // 174
	WORD notsuicides; // 176
	char unknown7[18];  // 178
	union // 196
	{
		short king_time;
		short ctf_flag_grabs;
		short race_last_lap;
	};
	union // 198
	{
		short ctf_flag_returns;
		short race_score;
	};
	union // 200
	{
		short ctf_score;
		short race_fastest_lap;
	};
	short unkscore; // 202
	char sfs[16]; // 204
	DWORD ping; // 220
	DWORD tk_number; // 224
	DWORD tk_timer; // 228
	char unknown9[8];
	DWORD LastSync;
	long player_dead;	// 244  -1 if players is dead
	vect3f s_position;  // 248
	char unknown12[96]; // 260
	DWORD LastSyncToOthers; // 356
	char unknown13[20]; // 360
	DWORD LastVehicleSyncToOthers; // 380
	char unknown14[128]; // 384
};

struct Static_Player_Header
{
	BYTE TName[32]; // 'players'
	WORD MaxSlots; // Max number of slots/players possible
	WORD SlotSize; // Size of each Static_Player struct
	DWORD Unknown; // always 1?
	BYTE Data[4]; // '@t@d' - translated as 'data'?
	WORD IsInMainMenu; // 0 = in game 1 = in main menu / not in game
	WORD SlotsTaken; // or # of players
	WORD NextPlayerIndex; // Index # of the next player to join
	WORD NextPlayerID; // ID # of the next player to join
	Player_Table_Item* FirstPlayer; // Pointer to the first static player
};

struct Dynamic_Player
{
	union
	{
		DWORD MetaNumber;
		struct 
		{
			WORD MetaIndex;
			WORD MetaID; // matches against the map's meta table
		};
	};
	DWORD Unknown0;
	bool Sync;
	bool Synced;
	WORD OtherFlags;
	DWORD ExistanceTime;
	DWORD SomeFlags;
	DWORD LastAccess;
	BYTE Unknown00[68];
	Player_Vectors PlayerVectors;
	BYTE Unknown01[8];
	WORD ObjectType; // 0 => Biped, 1 => Vehicle, 2 => Weapon, 3 => Equipment, 4 => Garbage, 5 => Projectile, 6 => Scenery, 7 => Machine, 8 => Control, 9 => LightFixture, 10 => Placeholder, 11 => SoundScenery (Thx to 002)
	BYTE Unknown02[6];
	DWORD DespawnTimer;
	union
	{
		DWORD ParentID;
		struct
		{
			WORD PlayerIndex;
			WORD PlayerID;
		};
	};

	BYTE Unknown1[28];
	float Health;
	float Shield;
	BYTE Unknown2[28];
	union
	{
		DWORD Flags0;
		struct 
		{
			BYTE UnknowFlag0;
			BYTE UnknowFlag1;
			BYTE UnknowFlag2;
			BYTE IsInvincible;
		};
	};
	BYTE Unknown3[4];
	DWORD Exists;
	BYTE Unknown33[4];
	union
	{
		uint32_t VehicleWeaponIndex;
		struct
		{
			uint16_t VehicleWeaponIndexNum;
			uint16_t VehicleWeaponID;
		};
	};
	union
	{
		uint32_t WeaponIndex;
		struct
		{
			uint16_t WeaponIndexNum;
			uint16_t WeaponID;
		};
	};
	union
	{
		uint32_t VehicleIndex;
		struct
		{
			uint16_t VehicleIndexNum;
			uint16_t VehicleID;
		};
	};
	BYTE Unknown4[116];
	vect3f Color;
	BYTE Unknown44[72];
	uint16_t Sizeof4thBlock;
	uint16_t Sizeof1st2ndand3rdBlock;
	uint16_t Sizeof3rdBlock;
	uint16_t Sizeof1stand2ndBlock;
	uint16_t Sizeof2ndBlock;
	uint16_t Sizeof1stBlock;
	// End of dynamic object base
	BYTE UnknownFF[16];
	union
	{
		DWORD Flags1;
		struct 
		{
			BYTE IsInvisible; // dead = 0x40, visible = 0x41, invisible = 0x51 (bitfield?) yes
			BYTE UnkFlag1;
			BYTE UnkFlag2;
			BYTE SuspendUnit;
		};
	};
	union
	{
		DWORD Flags2;
		struct 
		{
			BYTE ControlFlags;      // nothing = 0, crouch = 1, jump = 2, both = 3, flashlight pressed = 0x10, action button pressed = 0x40, melee pressed = 0x80,  (bitfield)
			BYTE ActionFlags;		// 0x08 shooting, 0x30 throwing nade, 0x04 = reload pressed, 0x40 = action button hold (bitfield)
			BYTE UnknownFlag3;
			BYTE UnknownFlag4;
		};
	};
	BYTE Unknown5[36];
	vect3f Camera;
	BYTE Unknown6[88];
	DWORD NadeObjectToThrow;
	BYTE Unknown7[96];
	WORD PrimaryWeaponIndex;
	WORD PrimaryWeaponID;
	WORD SecondaryWeaponIndex;
	WORD SecondaryWeaponID;
	WORD TertiaryWeaponIndex;
	WORD TertiaryWeaponID;
	WORD QuaternaryWeaponIndex;
	WORD QuaternaryWeaponID;
	BYTE Unknown8[22];
	BYTE FragNades;
	BYTE PlasmaNades;
	BYTE Zoomed;
	BYTE ZoomedAgain;
	//BYTE Unknown8[284];
	//DWORD LastDamage;
	//BYTE Unknown8[272];
	BYTE Unknown9[354];
	float MoveUPDown;
	float MoveLeftRight;
	BYTE Unknown10[196];
	float Body[13];
	float LeftThigh[13];
	float RightThigh[13];
	float Pelvis[13];
	float LeftCalf[13];
	float RightCalf[13];
	float Spine[13];
	float LeftClavicle[13];
	float LeftFoot[13];
	float Neck[13];
	float RightClavicle[13];
	float RightFoot[13];
	float Head[10];
	vect3f HeadCoord;
	float LeftUpperArm[13];
	float RightUpperArm[13];
	float LeftLowerArm[13];
	float RightLowerArm[13];
	float LeftHand[13];
	float RightHand[13];
	float AnimationFloats[304];
};

#pragma pack(pop)

struct NoLeadObjectItem
{
	bool Valid = false;
	bool Restored = false;
	size_t Size = 0;
	Dynamic_Player* Object = nullptr;
	NoLeadObjectItem* Weapon = nullptr;
	NoLeadObjectItem* VehicleWeapon = nullptr;
	~NoLeadObjectItem()
	{
		delete Weapon;
		delete VehicleWeapon;
		delete[] (uint8_t*)Object;
	}
};

struct NoLeadObjectItemRoot : NoLeadObjectItem
{
	uint32_t RootIndex;
};