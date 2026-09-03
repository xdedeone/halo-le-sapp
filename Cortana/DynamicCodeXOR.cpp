#include "DynamicCodeXOR.h"

unsigned int MagicUserKey;

void __declspec(naked) EncryptBegin()
{
	__asm
	{
		push eax
		push ecx
		push edi
		mov edi, dword ptr ds : [esp + 0x0c]
		mov ecx, dword ptr ds : [esp - 4]
		mov eax, MagicUserKey
		DoDecrypt :
			xor[edi], eax
			add edi, 4
			dec ecx
		jnz DoDecrypt
		pop edi
		pop ecx
		pop eax
		retn
	}
}

void __declspec(naked) EncryptEnd()
{
	__asm
	{
		push eax
		push ecx
		push edi
		mov edi, dword ptr ds : [esp + 0x0c]
		mov ecx, dword ptr ds : [esp - 4]
		mov eax, ecx
		shr ecx, 2
		and eax, 3
		sub edi, eax
		sub edi, 4
		mov eax, MagicUserKey
		DoDecrypt :
			xor[edi], eax
			sub edi, 4
			dec ecx
		jnz DoDecrypt
		pop edi
		pop ecx
		pop eax
		retn
	}
}