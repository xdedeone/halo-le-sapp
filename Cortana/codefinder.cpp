#include "codefinder.h"

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
	ENCRYPT_BEGIN
	__asm
	{
		mov edx, code
		mov edi, Address
		mov ecx, SizeLeft
		BEGIN_FOR :
			mov al, [edx]
			repne scas byte ptr es : [edi]
			test ecx, ecx
			JE NOT_FOUND
			mov ebx, 1
			SCAN_NEXT_BYTES :
				mov eax, mask
				cmp[eax + ebx], 0
				je BYTE_MATCH
				mov al, [edx + ebx]
				cmp[edi], al
				jne END_FOR
			BYTE_MATCH :
				inc edi
				inc ebx
				cmp len, ebx
				jne SCAN_NEXT_BYTES
			mov eax, edi
			sub eax, ebx
			jmp END_SEARCH
		END_FOR :
			sub edi, ebx
			inc edi
			jmp BEGIN_FOR
		NOT_FOUND :
		xor eax, eax
		END_SEARCH :
	}
	ENCRYPT_END
}

DWORD __declspec(noinline) FindCode(DWORD address, size_t size, const short* code, size_t length)
{
	ENCRYPT_BEGIN
	length /= 2;
	char *code_data = new char[length];
	char *mask_data = new char[length];

	memset(mask_data, 1, length);

	for (size_t i = 0; i < length; i++)
	{
		if (code[i] == -1)
			mask_data[i] = 0;
		else
			code_data[i] = (char)code[i];
	}

	DWORD ret = FindSignature(address, size, code_data, mask_data, length);

	delete[] code_data;
	delete[] mask_data;

	ENCRYPT_END

	return ret;
}

DWORD __declspec(noinline) FindCodeFromHandle(HANDLE hModule, const short* code, size_t length)
{
	ENCRYPT_BEGIN
	length /= 2;
	char *code_data = new char[length];
	char *mask_data = new char[length];

	memset(mask_data, 1, length);

	for (size_t i = 0; i < length; i++)
	{
		if (code[i] == -1)
			mask_data[i] = 0;
		else
			code_data[i] = (char)code[i];
	}

	DWORD ret = NULL;
	PIMAGE_SECTION_HEADER CodeSection = GetSection(hModule);
	if (CodeSection != NULL)
	{
		ret = FindSignature((DWORD)hModule + CodeSection->VirtualAddress, CodeSection->SizeOfRawData - length, code_data, mask_data, length);
	}
	
	delete[] code_data;
	delete[] mask_data;

	ENCRYPT_END

	return ret;
}