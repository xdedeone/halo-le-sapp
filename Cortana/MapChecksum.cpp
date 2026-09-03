#include "MapChecksum.h"

#include <string>

#include "Addresses.h"
#include "Patch.h"
#include "sha256_file.h"
#include "StringCrypter.h"

short MapNameSig[] = { 0xB8, -1, -1, -1, -1, 0x8D, 0x50, 0x01, 0x8A, 0x08, 0x40, 0x84, 0xC9, 0x75, 0xF9, 0x2B, 0xC2, 0x83, 0xF8, 0x1F };
short MapChecksumSig[] = { 0x25, 0xFF, 0xFF, 0x00, 0x00, 0xC1, 0xE0, 0x05, 0x8B, 0x44, 0x10, 0x14, 0x83, 0xC4, 0x04, 0x33, 0xF6 };

char* MapName = NULL;

unsigned char ui_str[] = { 0x8a, 0x96, 0xff }; // ui
unsigned char maps_str[] = { 0x92, 0x9e, 0x8f, 0x8c, 0xa3, 0xff }; // maps\ 
unsigned char dotmap_str[] = { 0xd1, 0x92, 0x9e, 0x8f, 0xff }; // .map

extern bool TagHashReady;

void ChecksumThread(LPVOID)
{
	ENCRYPT_BEGIN
	if (strcmp(MapName, CryptExtractor(StringCrypt(ui_str, sizeof(ui_str)))))
	{
		std::string map_file_name = CryptExtractor(StringCrypt(maps_str, sizeof(maps_str)));
		map_file_name.append(MapName);
		map_file_name.append(CryptExtractor(StringCrypt(dotmap_str, sizeof(dotmap_str))));

		char buff[64] = {};
		memcpy(buff, MapName, 32);

		SHA256_File(map_file_name.c_str(), (unsigned char*)buff + 32);

		myClient->SendBigEvent(event_map_checksum, buff, 64);
	}
	else // quit from the game (to ui.map)
	{
		SappServer = false;
		ACServer = false;
		TagHashReady = false;
		TimeLeft = 1;
		for (size_t i = 0; i < 16; i++)
		{
			memcpy(ACPlayers[i], "---", 4);
		}
	}
	ENCRYPT_END
}

void ChecksumMap()
{
	ENCRYPT_BEGIN
	_beginthread(ChecksumThread, 0, 0);
	ENCRYPT_END
}

void __declspec(naked) MapChecksumCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		pushad
		pushfd
		call ChecksumMap
		popfd
		popad
		and eax, 0x0000ffff
		ENCRYPT_END
		retn
	}
}

void __declspec(noinline) InstallMapChecksumCode()
{
	ENCRYPT_BEGIN
	Patches.NewPatch(new MemoryAddress(hhalo, MapNameSig, sizeof(MapNameSig), 1, (DWORD*)&MapName));
	Patches.NewPatch(new CodeCaveHook(hhalo, MapChecksumSig, sizeof(MapChecksumSig), 5, 0, NULL, MapChecksumCC, true));
	ENCRYPT_END
}