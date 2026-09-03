#include <Windows.h>
#include <stdio.h>

struct DLL_Str
{
	IMAGE_DOS_HEADER dos_header;
	char dos_stub[64];
};

void encrypt(unsigned char* buff, size_t siz)
{
	DLL_Str* dll = (DLL_Str*)buff;
	dll->dos_header.e_magic = 0; // MZ

	memset(dll->dos_stub, 0, 64); // "This program cannot be run in DOS mode." etc.

	PIMAGE_NT_HEADERS old_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(buff))[dll->dos_header.e_lfanew];
	old_header->Signature = 0; // PE

	PIMAGE_SECTION_HEADER Section = (PIMAGE_SECTION_HEADER)((DWORD)old_header + sizeof(IMAGE_NT_HEADERS));

	for (WORD i = 0; i < old_header->FileHeader.NumberOfSections; i++)
	{
		memset(Section->Name, 0, IMAGE_SIZEOF_SHORT_NAME); // .text .data etc.
		Section++;
	}
}

void main()
{
	HANDLE hFile = CreateFile("Cortana.dll", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		size_t file_lenght = GetFileSize(hFile, NULL);

		unsigned char* buff = new unsigned char[file_lenght];

		DWORD numBytes;
		ReadFile(hFile, buff, file_lenght, &numBytes, NULL);

		encrypt(buff, file_lenght);

		SetFilePointer(hFile, NULL, NULL, FILE_BEGIN);

		WriteFile(hFile, buff, file_lenght, &numBytes, NULL);

		delete[] buff;

		CloseHandle(hFile);

		printf("Successfully packed Cortana.dll");
	}
	else
	{
		printf("Failed to pack Cortana.dll");
	}
}