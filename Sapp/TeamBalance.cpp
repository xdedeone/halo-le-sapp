#include "TeamBalance.h"

#include "Addresses.h"
#include "HaloUtilities.h"
#include "Patcher.h"
#include "Prefs.h"

bool __stdcall TeamBalanceHandler(DWORD PlayerIndex, DWORD Team)
{
	if (!PlayerPresent(PlayerIndex))
		return true;

	if (PlayerTable[PlayerIndex].Team == Team)
		return true;

	int RealPlayerNumber = 0;
	int Reds = 0, Blues = 0;
	size_t TakenSlotIndexes[16];

	GetPlayers(RealPlayerNumber, TakenSlotIndexes);

	for(int i = 0; i < RealPlayerNumber; i++)
		if(PlayerTable[TakenSlotIndexes[i]].Team == 0)
			Reds++;
		else
			Blues++;

	if(Team == 0)
	{
		if(Reds >= Blues)
			return false;
	}
	else
	{
		if(Blues >= Reds)
			return false;
	}

	return true;
}

void __declspec(naked) TeamBalanceHandlerCodecave()
{
	__asm
	{
		movzx eax, byte ptr ss:[esp + 0x10]
		push ebx
		push eax
		call TeamBalanceHandler
		ret
	}
}

bool CheckBlockTC()
{
	return GlobalPrefs->BlockTC;
}

bool __stdcall TeamChangeBlockHandler(DWORD PlayerIndex, DWORD Team)
{
	if (!PlayerPresent(PlayerIndex))
		return false;

	if (Server->PlayerData[PlayerIndex].team == Team)
		return false;

	return true;
}

DWORD RejectFunc;
DWORD EndAddr;
void __declspec(naked) TeamChangeBlockHandlerCodecave()
{
	__asm
	{
		push eax
		pushfd

		call CheckBlockTC
		test al, al
		jnz block
		popfd
		pop eax

		push ebx
		movzx ebx, byte ptr ss:[esp+5]

		jmp EndAddr

	block:
		popfd
		pop eax

		movzx eax, byte ptr ss:[esp + 1]
		movzx ecx, byte ptr ss:[esp]
		push eax
		push ecx
		call TeamChangeBlockHandler
		test al, al
		jz no_block_msg

		movzx ecx, byte ptr ss:[esp]
		push ecx
		mov ecx, 0x91
		call RejectFunc
		add esp, 4

	no_block_msg:
		add esp, 0x1c
		retn
	}
}

void InstallTBHandler()
{
	Patches.NewPatch(new RelativeMemoryValue("Team Balance Reject Func", TeamChangeRejecetCode, 6, &RejectFunc));
	Patches.NewPatch(new CodeCaveHook("Team Balance Hook", TeamBalanceCode, 5, 0, NULL, TeamBalanceHandlerCodecave, true));
	Patches.NewPatch(new CodeCaveHook("Team Change Block Hook", BlockTeamChange, 6, 0, &EndAddr, TeamChangeBlockHandlerCodecave));
}