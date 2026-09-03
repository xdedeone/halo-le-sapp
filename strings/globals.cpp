#include "globals.h"

HMODULE hModule = GetModuleHandleW(0);

DWORD HaloPrintAddress;
void PrintConsoleAsm(char* buffer)
{
	__asm
	{
		pushad
		pushfd
		
		xor eax, eax
		push buffer
		call HaloPrintAddress
		add esp, 4

		popfd
		popad
	}
}

//void FPrintConsoleAsm(char* format, char* buffer)
//{
//	__asm
//	{
//		pushad
//		pushfd
//		
//		xor eax, eax
//		push buffer
//		push format
//		call HaloPrintAddress
//		add esp, 8
//
//		popfd
//		popad
//	}
//}