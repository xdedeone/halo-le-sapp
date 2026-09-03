//LPVOID HaloStaticMemory = NULL;
//LPVOID HaloStaticMemoryTagTable = NULL;
//TagTableHeader** TagTablePtr = NULL;
//
//LPVOID __stdcall VirtualAllocReplace(LPVOID Address, SIZE_T Size, DWORD AllocType, DWORD Protect)
//{
//	LPVOID ret;
//	if((DWORD)Address == 0x40000000)
//	{
//		ret = VirtualAllocEx(INVALID_HANDLE_VALUE, 0, Size, AllocType, Protect);
//		HaloStaticMemory = ret;
//		HaloStaticMemoryTagTable = (LPVOID)(DWORD(ret) + 0x440000);
//		Patches.NewPatch(new CodePatch("TagtableAddr", HaloTagTableAddrCode, 4, 6, (byte*)&HaloStaticMemoryTagTable));
//	}
//	else
//	{
//		ret = VirtualAllocEx(INVALID_HANDLE_VALUE, Address, Size, AllocType, Protect);
//	}
//
//	return ret;
//}
//
//LPVOID pVirtualAllocReplace = &VirtualAllocReplace;
//LPVOID ppVirtualAllocReplace = &pVirtualAllocReplace;
//
//void TagTableFix()
//{
//	TagTableHeader* TagTableHead = (TagTableHeader*)HaloStaticMemoryTagTable;
//	*TagTablePtr = TagTableHead;
//
//	TagEntry* TagTable = (TagEntry*)(DWORD(HaloStaticMemoryTagTable) + sizeof(TagTableHeader));
//	TagTableHead->TagTableEntryPointer = (DWORD)TagTable;
//	
//	for(size_t i = 0; i < TagTableHead->TagCount; i++)
//	{
//		TagTable[i].TagNameAddress -= 0x40000000;
//		TagTable[i].TagNameAddress += DWORD(HaloStaticMemory);
//		TagTable[i].TagStruct -= 0x40000000;
//		TagTable[i].TagStruct += DWORD(HaloStaticMemory);
//	}
//}
//
//DWORD BackToTagTable;
//void __declspec(naked) TagTableFixCodeCave()
//{
//	__asm
//	{
//		pushad
//		pushfd
//		call TagTableFix
//		popfd
//		popad
//		jmp BackToTagTable
//	}
//}


//Patches.NewPatch(new CodePatch("MemoryAlloc", HaloMemoryAllocCode, 4, 3, (byte*)&ppVirtualAllocReplace));
//Patches.NewPatch(new MemoryAddress("TagTableFix", TagTableFixCode, 2, (DWORD*)&TagTablePtr));
//Patches.NewPatch(new CodeCaveHook("TagTableFixCC", TagTableFixCode, 6, 0, &BackToTagTable, (DWORD)TagTableFixCodeCave));


//ServerInfoStruct* HaloServer = NULL;
//DWORD HaloServerMask = NULL;
//
//void InitServer()
//{
//	HaloServer->MaxPlayers = _PLAYER_NUMBER_;
//	HaloServer->SomeOne = 1;
//	HaloServer->GamesPlayed = -1;
//
//	for(size_t i = 0; i < _PLAYER_NUMBER_; i++)
//	{
//		HaloServer->PlayerData[i].Prescene = -1;
//	}
//	for(size_t i = 0; i < _PLAYER_NUMBER_; i++)
//	{
//		HaloServer->MachineInfo[i].PlayerNumber = -1;
//		HaloServer->MachineInfo[i].HashID = -1;
//	}
//}
//
//DWORD FinaliseInit;
//void __declspec(naked) InitServerCC()
//{
//	__asm
//	{
//		pushad
//		pushfd
//		call InitServer
//		popfd
//		popad
//		mov ebx, HaloServer
//		jmp FinaliseInit
//	}
//}
//
//void Install_PLAYER_NUMBER_Players()
//{
//	if(HaloServer != NULL)
//		delete HaloServer;
//
//	HaloServer = new ServerInfoStruct;
//	memset(HaloServer, 0, sizeof(ServerInfoStruct));
//	DWORD HaloServerMask = (DWORD)HaloServer + 6;
//
//	Patches.NewPatch(new CodePatch("ServerInit1", InitServerStructCode1, 4, 4, (byte*)&HaloServer));
//	Patches.NewPatch(new CodePatch("ServerInit1", InitServerStructCode1, 4, 23, (byte*)&HaloServerMask));
//	Patches.NewPatch(new CodePatch("ServerInit1", InitServerStructCode1, 4, 77, (byte*)&HaloServer));
//	Patches.NewPatch(new CodePatch("ServerInit1", InitServerStructCode1, 4, 89, (byte*)&HaloServerMask));
//	Patches.NewPatch(new CodePatch("ServerInit1", InitServerStructCode1, 4, 116, (byte*)&HaloServer));
//	Patches.NewPatch(new CodePatch("ServerInit3", InitServerStructCode3, 4, 10, (byte*)&HaloServer));
//	Patches.NewPatch(new CodeCaveHook("ServerInit2", InitServerStructCode2, 5, 0, NULL, (DWORD)InitServerCC));
//	Patches.NewAddress(new FunctionAddress("ServerInit3", InitServerStructCode3, 0, &FinaliseInit));
//
//	Patches.NewPatch(new CodePatchInc("AllocObjectTable", AllocObjectTableCode, 1, 0x800)); // Increase MaxObjects from 2048 to 4096 because why not
//	Patches.NewPatch(new CodePatchInc("AllocPlayerAndTeamTables", AllocPlayerAndTeamTablesCode, 4, (_PLAYER_NUMBER_-16))); // Increase Player table from 16 to to 64 entrys
//	Patches.NewPatch(new CodePatchInc("AllocPlayerAndTeamTables", AllocPlayerAndTeamTablesCode, 21, (_PLAYER_NUMBER_-16))); // Increase Teams table from 16 to to 64 entrys
//	
//	Patches.NewPatch(new CodePatchInc("FinishServerInit", FinishServerInitCode, 7, (_PLAYER_NUMBER_-16)*32));
//	Patches.NewPatch(new CodePatchInc("FinishServerInit", FinishServerInitCode, 15, (_PLAYER_NUMBER_-16)*32));
//	Patches.NewPatch(new CodePatchInc("SomeFuncs0", SomeFuncsCode0, 7, (_PLAYER_NUMBER_-16)*32));
//	Patches.NewPatch(new CodePatchInc("SomeFuncs0", SomeFuncsCode0, 15, (_PLAYER_NUMBER_-16)));
//	Patches.NewPatch(new CodePatchInc("SomeFuncs0", SomeFuncsCode0, 7, (_PLAYER_NUMBER_-16)*32));
//	Patches.NewPatch(new CodePatchInc("SomeFuncs0", SomeFuncsCode0, 15, (_PLAYER_NUMBER_-16)));
//	Patches.NewPatch(new CodePatchInc("SomeInitFuncs1", SomeInitFuncsCode1, 7, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeInitFuncs1", SomeInitFuncsCode1, 54, (_PLAYER_NUMBER_-16)*32));
//	Patches.NewPatch(new CodePatchInc("InitFuncEnd1", InitFuncEndCode1, 4, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeInitFuncsInline2", SomeInitFuncsCodeInline2, 5, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeInitFuncsInline2", SomeInitFuncsCodeInline2, 35, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeInitFuncs2", SomeInitFuncsCode2, 5, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeInitFuncs2", SomeInitFuncsCode2, 21, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("InitFuncEnd2", InitFuncEndCode2, 10, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	
//	Patches.NewPatch(new CodePatchInc("sv_maxplayers1", sv_maxplayersCode1, 7, (_PLAYER_NUMBER_-16))); // sv_maxplayers max 16 -> 64
//	Patches.NewPatch(new CodePatchInc("sv_maxplayers2", sv_maxplayersCode2, 1, (_PLAYER_NUMBER_-16))); // "sv_maxplayers must be between 1 and 16 -> 64"
//	
//	Patches.NewPatch(new CodePatchInc("sv_maxplayersFix", sv_maxplayersFixCode, 8, (_PLAYER_NUMBER_-16))); // Halo try to override sv_maxplayers if it's more than 16, now 64 // if maxpl > 16 -> 64
//	Patches.NewPatch(new CodePatchInc("sv_maxplayersFix", sv_maxplayersFixCode, 12, (_PLAYER_NUMBER_-16))); // Halo try to override sv_maxplayers if it's more than 16, now 64 // maxpl = 16 -> 64
//	
//	Patches.NewPatch(new CodePatchInc("SomeCheckAtJoin", SomeCheckAtJoinCode, 1, (_PLAYER_NUMBER_-16)));
//	Patches.NewPatch(new CodePatchInc("SomeCheckAtJoin", SomeCheckAtJoinCode, 27, (_PLAYER_NUMBER_-16)));
//
//	Patches.NewPatch(new CodePatchInc("GetRandomNames", GetRandomNamesCode, 9, (_PLAYER_NUMBER_-16)));
//
//	Patches.NewPatch(new CodePatchInc("SomeServerFullFix", SomeServerFullFixCode, 2, (_PLAYER_NUMBER_-16)));
//	Patches.NewPatch(new CodePatchInc("SomeServerFullFix", SomeServerFullFixCode, 19, (_PLAYER_NUMBER_-16)));
//
//	Patches.NewPatch(new CodePatchInc("ServerFull1", ServerFullCode1, 1, (_PLAYER_NUMBER_-16)));
//	Patches.NewPatch(new CodePatchInc("ServerFull2", ServerFullCode2, 11, (_PLAYER_NUMBER_-16)));
//	Patches.NewPatch(new CodePatchInc("ServerFull3", ServerFullCode3, 2, (_PLAYER_NUMBER_-16)));
//	Patches.NewPatch(new CodePatchInc("ServerFull4", ServerFullCode4, 11, (_PLAYER_NUMBER_-16)));
//
//	Patches.NewPatch(new CodePatchInc("SomeFunc1", SomeFuncCode1, 14, (_PLAYER_NUMBER_-16)*32));
//	Patches.NewPatch(new CodePatchInc("SomeFunc1", SomeFuncCode1, 20, (_PLAYER_NUMBER_-16)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_1", SomeBigFunc1Code1, 4, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_2", SomeBigFunc1Code2, 2, (_PLAYER_NUMBER_-16)*32));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_2", SomeBigFunc1Code2, 7, (_PLAYER_NUMBER_-16)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_3", SomeBigFunc1Code3, 2, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_4", SomeBigFunc1Code4, 2, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_4", SomeBigFunc1Code4, 13, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_5", SomeBigFunc1Code5, 2, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_5", SomeBigFunc1Code5, 10, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_5", SomeBigFunc1Code5, 25, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_6", SomeBigFunc1Code6, 15, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_7", SomeBigFunc1Code7, 6, (_PLAYER_NUMBER_-16)*32));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_7", SomeBigFunc1Code7, 12, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_8", SomeBigFunc1Code8, 8, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_9", SomeBigFunc1Code9, 2, (_PLAYER_NUMBER_-16)*32));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_9", SomeBigFunc1Code9, 7, (_PLAYER_NUMBER_-16)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_10", SomeBigFunc1Code10, 2, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_11", SomeBigFunc1Code11, 2, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_12", SomeBigFunc1Code12, 2, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_13", SomeBigFunc1Code13, 6, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc1_14", SomeBigFunc1Code14, 4, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc2_1", SomeBigFunc2Code1, 4, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc2_2", SomeBigFunc2Code2, 11, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc3_1", SomeBigFunc3Code1, 12, (_PLAYER_NUMBER_-16)*32));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc3_2", SomeBigFunc3Code2, 2, (_PLAYER_NUMBER_-16)*32));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc3_3", SomeBigFunc3Code3, 2, (_PLAYER_NUMBER_-16)*32));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc3_4", SomeBigFunc3Code4, 5, (_PLAYER_NUMBER_-16)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc3_4", SomeBigFunc3Code4, 26, (_PLAYER_NUMBER_-16)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc3_4", SomeBigFunc3Code4, 33, (_PLAYER_NUMBER_-16)*32));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc3_5", SomeBigFunc3Code5, 2, (_PLAYER_NUMBER_-16)*32));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc4_1", SomeBigFunc4Code1, 9, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc4_2", SomeBigFunc4Code2, 2, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc4_2", SomeBigFunc4Code2, 14, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc4_3", SomeBigFunc4Code3, 14, (_PLAYER_NUMBER_-16)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc4_3", SomeBigFunc4Code3, 19, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc4_4", SomeBigFunc4Code4, 3, (_PLAYER_NUMBER_-16)*32));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc4_4", SomeBigFunc4Code4, 13, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//	Patches.NewPatch(new CodePatchInc("SomeBigFunc4_5", SomeBigFunc4Code5, 13, (_PLAYER_NUMBER_-16)*32 + (_PLAYER_NUMBER_-16)*sizeof(Player_Machine_Info)));
//}