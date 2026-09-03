#include "Mapcache.h"

#include <map>
#include <string>

#include "globals.h"

std::map<std::string, int> Maps;
bool CacheUpdated = false;

std::string currentMap;

/*
 * Writes checksums to the cache file.
 */
void __stdcall AddChecksum(int checksum)
{
	Maps[currentMap] = checksum;
	CacheUpdated = true;
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
		mov ecx, dword ptr ss : [esp + 0x14]
		mov dword ptr ds : [edx], ecx
		pushad
		push ecx
		call AddChecksum
		popad
		jmp ContinueChecksum
	}
}

/*
 * Writes the checksum hash-map to the cache file.
 */
void __stdcall WriteCache()
{
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
}

DWORD WinFindClose;
DWORD BackToLoading;
void __declspec(naked) WriteCacheCC()
{
	__asm
	{
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
	return found;
}

/*
 * Checks to see whether the cache already contains the checksum for the map.
 * If the checksum exists, it will be loaded into the corresponding table entry
 * for the map and Halo's checksum algorithm will be skipped. Otherwise, we will
 * allow Halo to checksum the map and it will be added to the cache at the end.
 */
DWORD DoChecksum;
void __declspec(naked) LoadFixDedi()
{
	__asm
	{
		//Hook code
		pushfd
		pushad
		mov ecx, dword ptr ss: [esp + 0x28]
		push ecx
		push eax
		call FindChecksum
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
		sub esp, 0x140
		pushad
		pushfd
		call InitCacheMap
		popfd
		popad
		jmp BackToMapLoad
	}
}

void InstallMapCache()
{
	Patches.NewPatch(new MemoryValue("MapCache WinFindCode", MapCache_EndChecksum_Code, 3, &WinFindClose));
	Patches.NewPatch(new CodeCaveHook("MapCache LoadChecksums", MapCache_LoadChecksum_Code, 6, 0, &BackToMapLoad, LoadChecksums));
	Patches.NewPatch(new CodeCaveHook("MapCache BeginChecksums", MapCache_BeginChecksum_Code, 5, 0, &DoChecksum, LoadFixDedi));
	Patches.NewPatch(new CodeCaveHook("MapCache EndChecksums", MapCache_EndChecksum_Code, 7, 0, &BackToLoading, WriteCacheCC));
	Patches.NewPatch(new CodeCaveHook("MapCache GrabChecksums", MapCache_GrabChecksum_Code, 6, 10, &ContinueChecksum, AddChecksumCC));
}