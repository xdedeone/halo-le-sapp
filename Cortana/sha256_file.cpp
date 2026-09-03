#include "sha256_file.h"
#include "sha256.h"
#include "StringCrypter.h"
#include "DynamicCodeXOR.h"

#include <Windows.h>
#include <stdio.h>

unsigned char rb_str[] = { 0x8d, 0x9d, 0xff }; // rb

void SHA256_File(const char* name, unsigned char* hash)
{
	ENCRYPT_BEGIN
	FILE* map_file;
	if (!fopen_s(&map_file, name, CryptExtractor(StringCrypt(rb_str, sizeof(rb_str)))) && map_file)
	{
		size_t file_lenght = 0;
		HANDLE hFile = CreateFile(name, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			file_lenght = GetFileSize(hFile, NULL);
			CloseHandle(hFile);
		}

		if (file_lenght != 0)
		{
			unsigned char* map_cache = new unsigned char[file_lenght];
			fread(map_cache, file_lenght, 1, map_file);
			SHA256_Data(map_cache, file_lenght, hash);
			delete[] map_cache;
		}

		fclose(map_file);
	}
	ENCRYPT_END
}