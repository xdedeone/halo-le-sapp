#include "ConsolePatch.h"

#include "Patcher.h"

const byte ConsoleSpamPatch[2] = { 0xEB, 0x11 }; // jmp short 0x11
const byte ConsoleInputPatch[6] = { 0x31, 0xDB, 0x90, 0x90, 0x90, 0x90 }; // xor ebx, ebx / nops

void InstallConsoleSpamPatch()
{
	Patches.NewPatch(new CodePatch("Console Spam Patch", ConsoleSpamCode, 2, 0, ConsoleSpamPatch));
}

void UninstallConsoleSpamPatch()
{
	Patches.Remove("Console Spam Patch");
}

void InstallConsoleInputPatch()
{
	Patches.NewPatch(new CodePatch("Console Input Patch", ConsoleInputCode, 6, 0, ConsoleInputPatch));
}
void UninstallConsoleInputPatch()
{
	Patches.Remove("Console Input Patch");
}