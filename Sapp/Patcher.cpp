#include "Patcher.h"

#include "HaloUtilities.h"
#include "TextUtilities.h"

extern HMODULE hModule;

Patch::Patch(const char* name, const char* sign, int Difference) : Name(name)
{
	PatchAddress = FindCode(hModule, sign);
	if(PatchAddress == NULL)
	{
		found = false;
		PrintConsole(12, FormatArgs("Failed to find signature for \"%hs\"!", name).c_str());
	}
	else
	{
		PatchAddress += Difference;
		found = true;	
	}
}

Patch::~Patch()
{
}

FunctionAddress::FunctionAddress(const char* name, const char* sign, int Difference, DWORD *Address) : Patch(name, sign, Difference)
{
	if(found)
	{	
		*Address = PatchAddress;
	}
}

MemoryAddress::MemoryAddress(const char* name, const char* sign, int Difference, DWORD *Value) : Patch(name, sign, Difference)
{
	if(found)
	{
		*Value = (DWORD)*(DWORD*)(PatchAddress);
	}
}

MemoryValue::MemoryValue(const char* name, const char* sign, int Difference, DWORD *Value) : Patch(name, sign, Difference)
{
	if(found)
	{
		*Value = *(DWORD*)*(DWORD*)(PatchAddress);
	}
}

RelativeMemoryValue::RelativeMemoryValue(const char* name, const char* sign, int Difference, DWORD *Value) : Patch(name, sign, Difference)
{
	if(found)
	{
		*Value = *(DWORD*)PatchAddress + PatchAddress + 4;
	}
}

CodePatch::CodePatch(const char* name, const char* sign, size_t size, int Difference, const byte* PatchCode, DWORD Pointer) : Patch(name, sign, Difference), size(size)
{
	if(found)
	{
		if(Pointer > 0)
		{
			if(Pointer == 1)
				PatchAddress = *(DWORD*)PatchAddress;
			else
				PatchAddress = *(DWORD*)PatchAddress + PatchAddress + 4;
		}

		memcpy(OrginalCode, (LPVOID)PatchAddress, size);

		DWORD OldProtect;
		VirtualProtect((LPVOID)PatchAddress, size, PAGE_EXECUTE_READWRITE, &OldProtect);

		memcpy((LPVOID)PatchAddress, PatchCode, size); // patch the code

		VirtualProtect((LPVOID)PatchAddress, size, OldProtect, &OldProtect);
	}
}
CodePatch::~CodePatch()
{
	if(found)
	{
		DWORD OldProtect;
		VirtualProtect((LPVOID)PatchAddress, size, PAGE_EXECUTE_READWRITE, &OldProtect);
			
		memcpy((LPVOID)PatchAddress, OrginalCode, size); // restore the original code

		VirtualProtect((LPVOID)PatchAddress, size, OldProtect, &OldProtect);
	}
}

CodeCaveHook::CodeCaveHook(const char* name, const char* sign, size_t size, int Difference, DWORD *BackAddress, LPVOID Function, bool Call) : Patch(name, sign, Difference), size(size)
{
	if(found)
	{
		char *cc_code = (char*)PatchAddress;
		memcpy(OrginalCode, cc_code, size);

		if(BackAddress != NULL)
			*BackAddress = PatchAddress + size;

		DWORD OldProtect;
		// insert codecave jump
		VirtualProtect((LPVOID)PatchAddress, size, PAGE_EXECUTE_READWRITE, &OldProtect);

		if(Call)
			cc_code[0] = '\xE8'; // opcode of call
		else
			cc_code[0] = '\xE9'; // opcode of jmp

		DWORD CallOffset = (DWORD)Function - (PatchAddress + 5);
		*(DWORD*)(&cc_code[1]) = CallOffset;

		memset(&cc_code[5], 0x90, size-5); // fill the rest with nops

		VirtualProtect((LPVOID)PatchAddress, size, OldProtect, &OldProtect);
	}
}
CodeCaveHook::~CodeCaveHook()
{
	if(found)
	{
		DWORD OldProtect;
		VirtualProtect((LPVOID)PatchAddress, size, PAGE_EXECUTE_READWRITE, &OldProtect);
			
		memcpy((LPVOID)PatchAddress, OrginalCode, size); // restore the original code

		VirtualProtect((LPVOID)PatchAddress, size, OldProtect, &OldProtect);
	}
}

APIHook::APIHook(const char* name, const char* sign, int Difference, DWORD *BackAddress, LPVOID Function, bool Call) : Patch(name, sign, Difference), size(5)
{
	if (found)
	{
		PatchAddress = **(DWORD**)PatchAddress;

		char *cc_code = (char*)PatchAddress;
		memcpy(OrginalCode, cc_code, size);

		if (BackAddress != NULL)
			*BackAddress = PatchAddress + size;

		DWORD OldProtect;
		// insert codecave jump
		VirtualProtect((LPVOID)PatchAddress, size, PAGE_EXECUTE_READWRITE, &OldProtect);

		if (Call)
			cc_code[0] = '\xE8'; // opcode of call
		else
			cc_code[0] = '\xE9'; // opcode of jmp

		DWORD CallOffset = (DWORD)Function - (PatchAddress + 5);
		*(DWORD*)(&cc_code[1]) = CallOffset;

		VirtualProtect((LPVOID)PatchAddress, size, OldProtect, &OldProtect);
	}
}
APIHook::~APIHook()
{
	if (found)
	{
		DWORD OldProtect;
		VirtualProtect((LPVOID)PatchAddress, size, PAGE_EXECUTE_READWRITE, &OldProtect);

		memcpy((LPVOID)PatchAddress, OrginalCode, size); // restore the original code

		VirtualProtect((LPVOID)PatchAddress, size, OldProtect, &OldProtect);
	}
}

APIReplace::APIReplace(const char* name, const char* sign, int Difference, LPVOID NewFunction) : Patch(name, sign, Difference)
{
	if (found)
	{
		PatchAddress = *(DWORD*)PatchAddress;

		Original = *(DWORD*)PatchAddress;

		DWORD OldProtect;
		VirtualProtect((LPVOID)PatchAddress, 4, PAGE_EXECUTE_READWRITE, &OldProtect);

		*(DWORD*)PatchAddress = (DWORD)NewFunction;

		VirtualProtect((LPVOID)PatchAddress, 4, OldProtect, &OldProtect);
	}
}
APIReplace::~APIReplace()
{
	if (found)
	{
		DWORD OldProtect;
		VirtualProtect((LPVOID)PatchAddress, 4, PAGE_EXECUTE_READWRITE, &OldProtect);

		*(DWORD*)PatchAddress = Original;

		VirtualProtect((LPVOID)PatchAddress, 4, OldProtect, &OldProtect);
	}
}

DelayedAPIReplace::DelayedAPIReplace(const char* name, const char* sign, const char* Module, const char* Process, int Difference, LPVOID *OldFunction, LPVOID NewFunction) : Patch(name, sign, Difference)
{
	if (found)
	{
		HMODULE hModuleAPI = GetModuleHandle(Module);
		if (hModuleAPI == NULL)
		{
			hModuleAPI = LoadLibrary(Module);
		}
		if (hModuleAPI != NULL)
		{
			*OldFunction = GetProcAddress(hModuleAPI, Process);
			if (*OldFunction != NULL)
			{
				PatchAddress = *(DWORD*)(PatchAddress + (*(DWORD*)PatchAddress) + 6);

				Original = *(DWORD*)PatchAddress;

				DWORD OldProtect;
				VirtualProtect((LPVOID)PatchAddress, 4, PAGE_EXECUTE_READWRITE, &OldProtect);

				*(DWORD*)PatchAddress = (DWORD)NewFunction;

				VirtualProtect((LPVOID)PatchAddress, 4, OldProtect, &OldProtect);

				return;
			}
		}

		found = false;
	}
}

DelayedAPIReplace::~DelayedAPIReplace()
{
	if (found)
	{
		DWORD OldProtect;
		VirtualProtect((LPVOID)PatchAddress, 4, PAGE_EXECUTE_READWRITE, &OldProtect);

		*(DWORD*)PatchAddress = Original;

		VirtualProtect((LPVOID)PatchAddress, 4, OldProtect, &OldProtect);
	}
}

void PatchContainer::NewPatch(Patch* newPatch)
{
	PatchMap.emplace(newPatch->Name, std::unique_ptr<Patch>(newPatch));
}

void PatchContainer::Remove(const char* Name)
{
	auto& it = PatchMap.find(Name);
	if(it != PatchMap.end())
	{
		PatchMap.erase(it);
	}
}

void PatchContainer::RemoveAll()
{
	PatchMap.clear();
}

PatchContainer Patches;