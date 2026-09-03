#include "codefinder.h"
#include <Windows.h>

PIMAGE_SECTION_HEADER GetSection(HANDLE hModule)
{
	PIMAGE_NT_HEADERS NtHeader = (PIMAGE_NT_HEADERS)((DWORD)hModule + ((PIMAGE_DOS_HEADER)hModule)->e_lfanew);

	PIMAGE_SECTION_HEADER Section = (PIMAGE_SECTION_HEADER)((DWORD)NtHeader + sizeof(IMAGE_NT_HEADERS));

	for (WORD i = 0; i < NtHeader->FileHeader.NumberOfSections; i++, Section++)
	{
		if (Section->Characteristics & IMAGE_SCN_MEM_EXECUTE)
			return Section;
	}

	return NULL;
}

DWORD XXFindCode(HANDLE exe, char *code, size_t siz)
{
	PIMAGE_SECTION_HEADER CodeSection = GetSection(exe);
	if(!CodeSection)
		return -1;

	DWORD Start = (DWORD)exe + CodeSection->VirtualAddress;
	DWORD SizeLeft = CodeSection->SizeOfRawData - siz;
	__asm
	{
		mov edx, code
		mov edi, Start
		mov ecx, SizeLeft
		BEGIN_FOR:
			mov al, [edx]				
			repne scas byte ptr es:[edi]
			test ecx, ecx
			JE NOT_FOUND
			mov ebx, 1
			SCAN_NEXT_BYTES:
				mov al, [edx+ebx]
				cmp [edi], al
				jne END_FOR
				inc edi		
				inc ebx
				cmp siz, ebx
				jne SCAN_NEXT_BYTES
			mov eax, edi
			sub eax, ebx
			jmp END_SEARCH
		END_FOR:
			inc edi
			jmp BEGIN_FOR
		NOT_FOUND:
			xor eax, eax
	END_SEARCH:
	}
}