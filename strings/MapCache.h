#pragma once

struct ChecksumEntry
{
	char name[36];
	unsigned long checksum;
};

void InstallMapCache();