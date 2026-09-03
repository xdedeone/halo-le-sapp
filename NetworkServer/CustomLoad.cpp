#include "CustomLoad.h"
#include "TextUtilities.h"

const unsigned char SIGNATURE[] = { 0xC7, 0x44, 0x24, 0xEC, 0xFF, 0xFF, 0xFF, 0xFF, 0xE8 };

//unsigned char* signatureFind(const unsigned char* signature, unsigned int sigLength, unsigned char* memory, unsigned int memLength) {
//	unsigned int bad_char_skip[256];
//	unsigned int last = sigLength - 1;
//
//	for (unsigned int i = 0; i <= 255; i++) {
//		bad_char_skip[i] = sigLength;
//	}
//
//	for (unsigned int i = 0; i < last; i++) {
//		bad_char_skip[signature[i]] = last - i;
//	}
//
//	while (memLength >= sigLength) {
//		for (unsigned int i = last; signature[i] == memory[i]; i--) {
//			if (i == 0) {
//				return &memory[i];
//			}
//		}
//
//		memLength -= bad_char_skip[memory[last]];
//		memory += bad_char_skip[memory[last]];
//	}
//
//	return 0;
//}

void encryptCode(unsigned char* code, unsigned int length, unsigned int magic)
{
	unsigned char* start = (unsigned char*)FindSequence((char*)code, (const char*)SIGNATURE, length, sizeof(SIGNATURE));

	if (start == 0)
		return;

	length -= (start - code) - 1;

	unsigned char* finish = (unsigned char*)FindSequence((char*)(start + 1), (const char*)SIGNATURE, length, sizeof(SIGNATURE));

	if (finish == 0)
		return;

	length -= (finish - start);

	size_t size = finish - start;
	*(DWORD*)&(start[4]) = size / 4;
	*(DWORD*)&(start[size + 4]) = size;

	DWORD* ccode = (DWORD*)(start + 13);
	for (size_t i = 0; i < size / 4; i++)
		ccode[i] ^= magic;

	encryptCode(finish + 1, length, magic);
}

void PerformBaseRelocation(unsigned char *codeBase, PIMAGE_NT_HEADERS headers, size_t base)
{
	PIMAGE_DATA_DIRECTORY directory = &headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	if (directory->Size > 0)
	{
		size_t delta = base - headers->OptionalHeader.ImageBase;
		PIMAGE_BASE_RELOCATION relocation = (PIMAGE_BASE_RELOCATION)(codeBase + directory->VirtualAddress);
		while (relocation->VirtualAddress > 0)
		{
			unsigned char *dest = codeBase + relocation->VirtualAddress;
			unsigned short *relInfo = (unsigned short *)((unsigned char *)relocation + (sizeof(IMAGE_BASE_RELOCATION)));
			for (DWORD i = 0; i < ((relocation->SizeOfBlock - (sizeof(IMAGE_BASE_RELOCATION))) / 2); i++, relInfo++) {
				DWORD *patchAddrHL;

				int type, offset;

				// the upper 4 bits define the type of relocation
				type = *relInfo >> 12;
				// the lower 12 bits define the offset
				offset = *relInfo & 0xfff;

				switch (type)
				{
				case IMAGE_REL_BASED_ABSOLUTE:
					// skip relocation
					break;

				case IMAGE_REL_BASED_HIGHLOW:
					// change complete 32 bit address
					patchAddrHL = (DWORD*)(dest + offset);
					*patchAddrHL += (DWORD)delta;
					break;

				default:
					//printf("Unknown relocation: %d\n", type);
					break;
				}

				// delete reloc offset
				*relInfo = 0;
			}

			// advance to next relocation block
			PIMAGE_BASE_RELOCATION prelocation = (PIMAGE_BASE_RELOCATION)(((char *)relocation) + relocation->SizeOfBlock);
			// delete reloc info
			relocation->VirtualAddress = 0;
			relocation->SizeOfBlock = 0;
			relocation = prelocation;
		}
	}
	directory->VirtualAddress = 0;
	directory->Size = 0;
}

void CopySections(const unsigned char *data, unsigned char *codeBase, PIMAGE_NT_HEADERS headers)
{
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(headers);
	for (WORD i = 0; i < headers->FileHeader.NumberOfSections; i++, section++)
	{
		if (section->SizeOfRawData != 0)
		{
			memcpy((unsigned char *)((DWORD)codeBase + section->VirtualAddress), data + section->PointerToRawData, section->SizeOfRawData);
		}
	}
}

void UnCopySections(unsigned char *data, const unsigned char *codeBase, PIMAGE_NT_HEADERS headers)
{
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(headers);
	for (WORD i = 0; i < headers->FileHeader.NumberOfSections; i++, section++)
	{
		if (section->SizeOfRawData != 0)
		{
			memcpy(data + section->PointerToRawData, (unsigned char *)((DWORD)codeBase + section->VirtualAddress), section->SizeOfRawData);
		}
	}
}

void XORFuncs(unsigned char* code, PIMAGE_NT_HEADERS header, DWORD magic)
{
	PIMAGE_SECTION_HEADER Section = (PIMAGE_SECTION_HEADER)((DWORD)header + sizeof(IMAGE_NT_HEADERS));

	for (WORD i = 0; i < header->FileHeader.NumberOfSections; i++)
	{
		if (Section->Characteristics & IMAGE_SCN_MEM_EXECUTE)
		{
			encryptCode((unsigned char*)((DWORD)code + Section->VirtualAddress), Section->SizeOfRawData, magic);
			break;
		}
		Section++;
	}
}