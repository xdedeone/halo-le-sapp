#include "ScoreboardPatch.h"

#include "Addresses.h"
#include "MiscFunctions.h"
#include "NetworkEvents.h"
#include "Patch.h"

short ScoreBoardTopSig[] = { 0x8B, 0x8C, 0x24, 0x50, 0x03, 0x00, 0x00, 0x5F, 0x5E, 0x5D, 0x66, 0xC7, 0x81, 0x9E, 0x00, 0x00, 0x00, 0x00, 0x00 }; // 8B8C24500300005F5E5D66C7819E0000000000
short ScoreBoardHeaderSig[] = { 0x8B, 0x84, 0x24, 0x80, 0x00, 0x00, 0x00, 0x8B, 0x4C, 0x24, 0x7C }; // 8B8424800000008B4C247C
short ScoreBoardSig[] = { 0x56, 0x50, 0x53, 0x8D, 0x4C, 0x29, 0x04, 0x51, 0x52 }; // 5650538D4C29045152
short ScoreBoardStarSig[] = { 0x51, 0x50, 0x53, 0x8D, 0x4C, 0x2E, 0x04, 0x51, 0x52 }; // 5150538D4C2E045152
short GameEndSig[] = { 0xE8, -1, -1, -1, -1, 0x84, 0xC0, 0x74, -1, 0x8B, 0x04, 0x24, 0x48 }; // E8????????84C074??8B042448

const wchar_t* __stdcall FormatNameTopString(wchar_t* text)
{
	ENCRYPT_BEGIN
	if (ServerApp == APP_SAPP && ServerAppVersion >= 0x09070100)
	{
		size_t len = wcslen(text);
		if (len && text[len - 1] == ' ')
			text[--len] = 0;
		if (TimeLeft > 0)
		{
			int tmp = TimeLeft / 30;
			swprintf(text + len, L", time left: %d:%02d", tmp / 60, tmp % 60);
		}
		else
		{
			wcscpy(text + len, L", time left: infinite");
		}
	}
	ENCRYPT_END
	return text;
}

DWORD BackToScoreboardTop;
void __declspec(naked) ScoreBoardTopCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		mov eax, dword ptr ss : [esp + 0x350]

		push eax
		call FormatNameTopString

		mov ecx, eax
		ENCRYPT_END
		jmp BackToScoreboardTop
	}
}

std::wstring wstrnh;
const wchar_t* __stdcall FormatNameHeaderString(wchar_t* name)
{
	ENCRYPT_BEGIN
	wstrnh = FormatWArgs(L"%ls [Sapp: %ls] [AC: %ls]", name, SappServer ? L"Yes" : L"No", ACServer ? L"ON" : L"OFF");
	ENCRYPT_END
	return wstrnh.c_str();
}

DWORD BackToScoreboardHeader;
void __declspec(naked) ScoreBoardHeaderCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		mov eax, dword ptr ss : [esp + 0x80]

		push ecx
		push ebx
		push edx
		push esi
		push edi

		push eax		
		call FormatNameHeaderString

		pop edi
		pop esi
		pop edx
		pop ebx
		pop ecx
		ENCRYPT_END
		jmp BackToScoreboardHeader
	}
}

std::wstring wstrn;
const wchar_t* __stdcall FormatNameString(wchar_t* name, size_t index)
{
	ENCRYPT_BEGIN
	index /= 0x200;
	wstrn = FormatWArgs(L"%ls - %hs", name, ACPlayers[index]);
	ENCRYPT_END
	return wstrn.c_str();
}

DWORD BackToScoreboard;
void __declspec(naked) ScoreBoardCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		push eax
		push ebx
		push edx
		push esi
		push edi

		lea ecx, [ebp + ecx + 4]
		push ebp
		push ecx
		call FormatNameString

		mov ecx, eax

		pop edi
		pop esi
		pop edx
		pop ebx
		pop eax

		push ecx
		ENCRYPT_END
		jmp BackToScoreboard
	}
}

DWORD BackToScoreboardStar;
void __declspec(naked) ScoreBoardStarCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		push eax
		push ebx
		push edx
		push esi
		push edi

		lea ecx, [ebp + esi + 4]
		push ebp
		push ecx
		call FormatNameString

		mov ecx, eax

		pop edi
		pop esi
		pop edx
		pop ebx
		pop eax

		push ecx
		ENCRYPT_END
		jmp BackToScoreboardStar
	}
}

extern bool TagHashReady;
void __stdcall GameEnd(size_t state)
{
	if (state == 2)
	{
		TagHashReady = false;
		for (size_t i = 0; i < 16; i++)
		{
			memcpy(ACPlayers[i], "---", 4);
		}
	}
}

DWORD OriginalGameEnd;
void __declspec(naked) GameEndCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		pushad

		push ecx
		call GameEnd

		popad
		ENCRYPT_END

		call OriginalGameEnd

		retn
	}
}

void __declspec(noinline) InstallScoreboardHookCode()
{
	ENCRYPT_BEGIN
	for (size_t i = 0; i < 16; i++)
	{
		memcpy(ACPlayers[i], "---", 4);
	}
	Patches.NewPatch(new CodeCaveHook(hhalo, ScoreBoardTopSig, sizeof(ScoreBoardTopSig), 7, 0, &BackToScoreboardTop, ScoreBoardTopCC));
	Patches.NewPatch(new CodeCaveHook(hhalo, ScoreBoardHeaderSig, sizeof(ScoreBoardHeaderSig), 7, 0, &BackToScoreboardHeader, ScoreBoardHeaderCC));
	Patches.NewPatch(new CodeCaveHook(hhalo, ScoreBoardSig, sizeof(ScoreBoardSig), 5, 3, &BackToScoreboard, ScoreBoardCC));
	Patches.NewPatch(new CodeCaveHook(hhalo, ScoreBoardStarSig, sizeof(ScoreBoardStarSig), 5, 3, &BackToScoreboardStar, ScoreBoardStarCC));
	Patches.NewPatch(new MemoryValueFromOPCode(hhalo, GameEndSig, sizeof(GameEndSig), 1, &OriginalGameEnd));
	Patches.NewPatch(new CodeCaveHook(hhalo, GameEndSig, sizeof(GameEndSig), 5, 0, NULL, GameEndCC, true));
	ENCRYPT_END
}