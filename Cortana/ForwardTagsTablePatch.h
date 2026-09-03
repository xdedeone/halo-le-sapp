#pragma once

struct Tag_Entry
{
	unsigned long Class0;
	unsigned long Class1;
	unsigned long Class2;
	unsigned long TagID;
	char* TagName;
	unsigned long TagStruct;
	unsigned char Unknown[8];
};

struct Tag_Table_Header
{
	Tag_Entry* TagTableEntryPointer;
	unsigned long FirstTagID;
	unsigned long MapID;
	unsigned long TagCount;
	unsigned long VerticesCount;
	unsigned long VerticesOffset;
	unsigned long IndicesCount;
	unsigned long IndicesOffset;
	unsigned long ModelDataSize;
	unsigned long Signature;
};

extern unsigned long TagsTable;

extern unsigned long long TagsTableHash;
extern bool TagHashReady;

void InstallForwardTagsTablePatch();