#include <Windows.h>
#include <stdio.h>

void encrypt(unsigned char* buff, size_t siz)
{
	PIMAGE_NT_HEADERS old_header = (PIMAGE_NT_HEADERS)&(buff)[((PIMAGE_DOS_HEADER)buff)->e_lfanew];
	PIMAGE_SECTION_HEADER Section = (PIMAGE_SECTION_HEADER)((DWORD)old_header + sizeof(IMAGE_NT_HEADERS));

	for (WORD i = 0; i < old_header->FileHeader.NumberOfSections; i++)
	{
		memset(Section->Name, 0, IMAGE_SIZEOF_SHORT_NAME); // .text .data etc.
		Section++;
	}
}

void main(int argc, char* argv[])
{
	HANDLE hFile = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
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

		printf("Successfully packed %s!", argv[1]);
	}
	else
	{
		printf("Failed to pack %s!", argv[1]);
	}
}