#include "Mapcache.h"

#include <map>
#include <string>

#include "Addresses.h"
#include "Patch.h"

using namespace std;

const short MapCache_LoadChecksum_Sig[] = { 0x81, 0xEC, 0x40, 0x01, 0x00, 0x00, 0x56, 0xBE }; // "81EC4001000056BE";
const short MapCache_BeginChecksum_Sig[] = { 0x83, 0xEC, 0x1C, 0x53, 0x55 }; // "83EC1C5355";
const short MapCache_EndChecksum_Sig[] = { 0x56, 0xFF, 0x15, -1, -1, -1, -1, 0x5F, 0x5E, 0x81, 0xC4, 0x40, 0x01, 0x00, 0x00, 0xC3 }; // "56FF15????????5F5E81C440010000C3";
const short MapCache_GrabChecksum_Sig[] = { 0xFF, 0x15, -1, -1, -1, -1, 0x8B, -1, 0x24, -1, 0x8B, -1, 0x24, -1, 0x89, 0x0A }; // "FF15????????8B??24??8B??24??890A";

map<string, int> Maps;
bool CacheUpdated = false;

string currentMap;

/*
 * Writes checksums to the cache file.
 */
void __stdcall AddChecksum(int checksum)
{
	ENCRYPT_BEGIN
	Maps[currentMap] = checksum;
	CacheUpdated = true;
	ENCRYPT_END
}

/*
 * Run each time Halo loads a map during start-up.
 * The calculated checksum is pushed to the stack and then saved to the cache.
 */
DWORD ContinueChecksum;
void __declspec(naked) AddChecksumCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		mov ecx, dword ptr ss : [esp + 0x14]
		mov dword ptr ds : [edx], ecx
		pushad
		push ecx
		call AddChecksum
		popad
		ENCRYPT_END
		jmp ContinueChecksum
	}
}

/*
 * Writes the checksum hash-map to the cache file.
 */
void __stdcall WriteCache()
{
	ENCRYPT_BEGIN
	FILE* fHandle;
	ChecksumEntry mapChecksum = {};
	if (fopen_s(&fHandle, "cache.hac", "wb") == 0 && fHandle != NULL)
	{
		for(std::map<std::string, int>::iterator iter = Maps.begin(); iter != Maps.end(); ++iter)
		{
			strcpy(mapChecksum.name, iter->first.c_str());
			mapChecksum.checksum = iter->second;
			fwrite(&mapChecksum, sizeof(ChecksumEntry), 1, fHandle);
		}
		fclose(fHandle);
	}
	ENCRYPT_END
}

DWORD WinFindClose;
DWORD BackToLoading;
void __declspec(naked) WriteCacheCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		//Hook code
		pushfd
		pushad
		mov al, CacheUpdated
		test al, al
		jz Continue
		call WriteCache

		//Original code
		Continue:
		popad
		popfd
		push esi
		call WinFindClose
		ENCRYPT_END
		jmp BackToLoading
	}
}

/*
 * Checks to see a checksum is already contained within the hash-map.
 * If a checksum is found, it is loaded from the hash-map and stored within
 * Halo's map table entry that corresponds with char* map.
 */
bool __stdcall FindChecksum(char* map, DWORD* checksum)
{
	ENCRYPT_BEGIN
	bool found = false;
	if ((Maps.find(map)) != Maps.end())
	{
		*checksum = Maps[map];
		found = true;
	}
	else
	{
		currentMap = map;
		Maps[map] = 0;
	}
	ENCRYPT_END
	return found;
}

/*
 * Checks to see whether the cache already contains the checksum for the map.
 * If the checksum exists, it will be loaded into the corresponding table entry
 * for the map and Halo's checksum algorithm will be skipped. Otherwise, we will
 * allow Halo to checksum the map and it will be added to the cache at the end.
 */
DWORD DoChecksum;
void __declspec(naked) LoadFix()
{
	__asm
	{
		ENCRYPT_BEGIN
		//Hook code
		pushfd
		pushad
		push eax
		push ecx
		call FindChecksum
		ENCRYPT_END
		test al, al
		popad
		jnz loadCachedChecksum
		
		//Continue checksum function
		popfd
		sub esp, 0x1C
		push ebx
		push ebp
		jmp DoChecksum
		
		//Bypass checksum function
		loadCachedChecksum:
		popfd
		ret
	}
}

/*
 * Reads the checksum cache file and loads the checksums into a hash-map.
 */
void __stdcall InitCacheMap()
{
	ENCRYPT_BEGIN
	FILE *fHandle;
	if(fopen_s(&fHandle, "cache.hac", "rb") == 0)
	{
		fseek(fHandle, 0L, SEEK_END);
		int numEntries = (ftell(fHandle) / sizeof(ChecksumEntry));

		//Continue loading checksums
		rewind(fHandle);

		ChecksumEntry *entries = new ChecksumEntry[numEntries];
		fread_s(entries, sizeof(ChecksumEntry) * numEntries, sizeof(ChecksumEntry), numEntries, fHandle);
			
		//Add entries to map
		for(int i = 0; i < numEntries; i++)
			Maps[entries[i].name] = entries[i].checksum;

		delete[] entries;

		fclose(fHandle);
	}
	ENCRYPT_END
}

/*
 * Loads the checksum cache out of the cache file and into the hash-map.
 * This codecave is just before the map checksum loop begins.
 */
DWORD BackToMapLoad;
void __declspec(naked) LoadChecksums()
{
	__asm
	{
		ENCRYPT_BEGIN
		sub esp, 0x140
		pushad
		pushfd
		call InitCacheMap
		popfd
		popad
		ENCRYPT_END
		jmp BackToMapLoad
	}
}

void __declspec(noinline) InstallMapCache()
{
	ENCRYPT_BEGIN
	Patches.NewPatch(new MemoryValue(hhalo, MapCache_EndChecksum_Sig, sizeof(MapCache_EndChecksum_Sig), 3, &WinFindClose));
	Patches.NewPatch(new CodeCaveHook(hhalo, MapCache_LoadChecksum_Sig, sizeof(MapCache_LoadChecksum_Sig), 6, 0, &BackToMapLoad, LoadChecksums));
	Patches.NewPatch(new CodeCaveHook(hhalo, MapCache_BeginChecksum_Sig, sizeof(MapCache_BeginChecksum_Sig), 5, 0, &DoChecksum, LoadFix));
	Patches.NewPatch(new CodeCaveHook(hhalo, MapCache_EndChecksum_Sig, sizeof(MapCache_EndChecksum_Sig), 7, 0, &BackToLoading, WriteCacheCC));
	Patches.NewPatch(new CodeCaveHook(hhalo, MapCache_GrabChecksum_Sig, sizeof(MapCache_GrabChecksum_Sig), 6, 10, &ContinueChecksum, AddChecksumCC));
	ENCRYPT_END
}