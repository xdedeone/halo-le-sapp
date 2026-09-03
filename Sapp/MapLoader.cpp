#include "MapLoader.h"

#include <memory>

#include "TextUtilities.h"

DWORD* MapTableSize;
DWORD* MapTableMaxSize;
MapEntry** MapTable;
MapEntry* SappMapTable;

bool FindMap(const char* name)
{
	for(size_t i = 0; i < *MapTableSize; i++)
	{
		if(!_stricmp(name, SappMapTable[i].name))
		{
			return true;
		}
	}
	return false;
}

// Code reversed by Btcc22 aka Chaosvex
DWORD xorChunk(DWORD checksum, char* data, DWORD length, DWORD* xorbuff)
{
	for(DWORD i = 0; i < length; i++)
	{
		int byte = data[i];
		byte ^= checksum;
		byte &= 255;
		byte = xorbuff[byte];
		checksum /= 256;
		checksum ^= byte;
	}
	return checksum;
}

// Code reversed by Btcc22 aka Chaosvex
void generateXorBuff(DWORD* buffer)
{
	for(DWORD i = 0, val = 0; i < 256; i++, val = i)
	{
		for(int j = 0; j < 8; j++)
		{
			if(val % 2 == 1)
			{
				val /= 2;
				val ^= 0xEDB88320;
			} 
			else
			{
				val /= 2;
			}
		}
		buffer[i] = val;
	}
}

// Code mostly written by Btcc22 aka Chaosvex
ChecksumResult Checksum(const char* filename, DWORD& store)
{
	//Create memory mapped file
	auto hFile = std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(&CloseHandle)>(CreateFileW(FormatWArgs(L"maps\\%hs.map", filename).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL), CloseHandle);
	
	if(hFile.get() == INVALID_HANDLE_VALUE)
		return ChecksumResult::INVALID_HANDLE;

	LARGE_INTEGER fileSize = {};
	if(GetFileSizeEx(hFile.get(), &fileSize) == 0)
	{
		return ChecksumResult::FILE_SIZE_FAIL;
	}

	auto hmap = std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(&CloseHandle)>(CreateFileMapping(hFile.get(), NULL, PAGE_READONLY, 0, 0, NULL), CloseHandle);

	if(hmap.get() == nullptr)
	{
		return ChecksumResult::MAPPING_FAIL;
	}

	MapHeader* view = static_cast<MapHeader*>(MapViewOfFile(hmap.get(), FILE_MAP_READ, 0, 0, 0));

	if(view == nullptr)
	{
		return ChecksumResult::MAP_VIEW_FAIL;
	}

	DWORD checksum = -1;
	DWORD offset = (DWORD)view;
	DWORD xorBuffer[256];
	generateXorBuff(xorBuffer);

	//Read the index
	MapHeader* pHeader = view;
	Tag_Table_Header *pIndex = reinterpret_cast<Tag_Table_Header*>(pHeader->IndexOffset + offset);

	//Ensure offsets aren't larger than the map file - cannot trust header info
	if(pHeader->IndexOffset > fileSize.QuadPart || pHeader->MetaSize > fileSize.QuadPart)
	{
		return ChecksumResult::BAD_OFFSETS;
	}

	//Calculate 'rebase' value
	DWORD magic = ((DWORD)pIndex->TagTableEntryPointer - pHeader->IndexOffset) - 40;

	//Checksum SBSPs - bless this mess
	Tag_Entry *pTag = reinterpret_cast<Tag_Entry*>(((DWORD)pIndex->TagTableEntryPointer - magic) + offset);
	SceneryTag* pScenery = reinterpret_cast<SceneryTag*>(((pTag->TagStruct - magic) + 0x5A4) + offset);
	
	for(DWORD i = 0; i < pScenery->sections; i++)
	{
		BSPChunkMeta* pBspMeta = reinterpret_cast<BSPChunkMeta*>(((pScenery->address - magic) + (i * 32)) + offset);
		checksum = xorChunk(checksum, (char*)(pBspMeta->offset  + offset), pBspMeta->size, xorBuffer);
	}

	//Checksum model data
	checksum = xorChunk(checksum, (char*)(pIndex->VerticesOffset + offset), pIndex->ModelDataSize, xorBuffer);

	//Checksum tag index
	checksum = xorChunk(checksum, (char*)(pHeader->IndexOffset + offset), pHeader->MetaSize, xorBuffer);

	//Clean up
	store = checksum;

	return ChecksumResult::CHECKSUM_SUCCESS;
}

void AddMap(const char* name, DWORD checksum)
{
	size_t nsize = strlen(name) + 1;
	char* mapname = (char*)GlobalAlloc(0, nsize);

	memcpy(mapname, name, nsize);

	ToLower(mapname);

	if(*MapTableSize == *MapTableMaxSize)
	{
		size_t size = *MapTableSize + (*MapTableSize % 16) + 16;

		SappMapTable = new MapEntry[size];
		memcpy(SappMapTable, *MapTable, (*MapTableSize)*sizeof(MapEntry));
		delete[] *MapTable;
		*MapTable = SappMapTable;
		*MapTableMaxSize = size;
	}

	MapEntry NewEntry;
	NewEntry.name = mapname;
	NewEntry.id = 0x13;
	NewEntry.checked = 1;
#ifndef HPC
	NewEntry.checksum = checksum;
#endif
	SappMapTable[(*MapTableSize)++] = NewEntry;
}

void InstallMapTableHandler()
{
	MapTableMaxSize = (DWORD*)(MapTableSize + 1);
	MapTable = (MapEntry**)((DWORD*)(MapTableSize - 1));

	SappMapTable = new MapEntry[*MapTableMaxSize];
	memcpy(SappMapTable, *MapTable, (*MapTableSize)*sizeof(MapEntry));
	GlobalFree(*MapTable);
	*MapTable = SappMapTable;
}

void UninstallMapTableHandler()
{
	*MapTable = (MapEntry*)GlobalAlloc(0, (*MapTableMaxSize)*sizeof(MapEntry));
	memcpy(*MapTable, SappMapTable, (*MapTableSize)*sizeof(MapEntry));	
	delete[] SappMapTable;
}