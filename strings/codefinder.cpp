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

DWORD FindSignature(DWORD Address, DWORD SizeLeft, char *code, char* mask, size_t len)
{
	__asm
	{
		mov edx, code
		mov edi, Address
		mov ecx, SizeLeft
		BEGIN_FOR:
			mov al, [edx]				
			repne scas byte ptr es:[edi]
			test ecx, ecx
			JE NOT_FOUND
			mov ebx, 1
			SCAN_NEXT_BYTES:
				mov eax, mask
				cmp [eax+ebx], 0
				je BYTE_MATCH
				mov al, [edx+ebx]
				cmp [edi], al
				jne END_FOR
			BYTE_MATCH:
				inc edi		
				inc ebx
				cmp len, ebx
				jne SCAN_NEXT_BYTES
			mov eax, edi
			sub eax, ebx
			jmp END_SEARCH
		END_FOR:
			sub edi, ebx
			inc edi
			jmp BEGIN_FOR
		NOT_FOUND:
			xor eax, eax
	END_SEARCH:
	}
}

long GetByte(char a, char b)
{
	if(a == '?')
		return -1;

	long ret = 0;

	if(a < 0x41) // / 0-9
		ret += (a - 0x30) * 16;
	else // A-F
		ret += (a - 0x37) * 16;

	if(b < 0x41) // / 0-9
		ret += (b - 0x30);
	else // A-F
		ret += (b - 0x37);

	return ret;
}


DWORD FindCode(HANDLE hModule, const char* code)
{
	size_t strlenght = strlen(code);
	size_t length = strlenght / 2;
	char *code_data = new char[length];
	char *mask_data = new char[length];

	memset(mask_data, 1, length);

	for(size_t i = 0; i < strlenght; i += 2)
	{
		long c = GetByte(code[i], code[i+1]);

		if(c == -1)
			mask_data[i/2] = 0;
		else
			code_data[i/2] = (char)c;
	}

	DWORD ret = NULL;
	PIMAGE_SECTION_HEADER CodeSection = GetSection(hModule);
	if (CodeSection != NULL)
	{
		ret = FindSignature((DWORD)hModule + CodeSection->VirtualAddress, CodeSection->SizeOfRawData - length, code_data, mask_data, length);
	}

	delete[] code_data;
    delete[] mask_data;

	return ret;
}