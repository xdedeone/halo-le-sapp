#pragma once

#define pushad __asm \
{					 \
	__asm push eax   \
	__asm push ecx   \
	__asm push edx   \
	__asm push ebx   \
	__asm push esi   \
	__asm push edi   \
}

#define popad __asm \
{					\
	__asm pop edi   \
	__asm pop esi   \
	__asm pop ebx   \
	__asm pop edx   \
	__asm pop ecx   \
	__asm pop eax   \
}

extern unsigned int MagicUserKey;

void EncryptBegin();
void EncryptEnd();

#define ENCRYPT_BEGIN {							\
	__asm mov dword ptr[esp - 0x14], 0xFFFFFFFF \
	__asm call EncryptBegin						\
}

#define ENCRYPT_END {							\
	__asm mov dword ptr[esp - 0x14], 0xFFFFFFFF \
	__asm call EncryptEnd						\
}

//#define ENCRYPT_BEGIN
//#define ENCRYPT_END