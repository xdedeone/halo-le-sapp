#pragma once

#include <set>

#include "codefinder.h"
#include "NetworkClient.h"

class Patch {
protected:
	DWORD PatchAddress;
	const short* sig;
	size_t slen;
	bool found;
public:
	Patch(HMODULE hModule, const short* sign, size_t length, size_t where = 0);
	virtual ~Patch() {}
	const short* GetSig(size_t &length);
};

class FunctionAddress : public Patch {
public:
	FunctionAddress(HMODULE hModule, const short* sign, size_t length, int Difference, DWORD *Address);
};

class MemoryAddress : public Patch {
public:
	MemoryAddress(HMODULE hModule, const short* sign, size_t length, int Difference, DWORD *Value);
};

class MemoryValue : public Patch {
public:
	MemoryValue(HMODULE hModule, const short* sign, size_t length, int Difference, DWORD *Value);
};

class MemoryValueFromOPCode : public Patch {
public:
	MemoryValueFromOPCode(HMODULE hModule, const short* sign, size_t length, int Difference, DWORD *Value);
};

class CodePatch : public Patch {
	size_t size;
	char OrginalCode[16];

public:
	CodePatch(HMODULE hModule, const short* sign, size_t length, size_t size, int Difference, byte* PatchCode, DWORD Pointer = 0);
	~CodePatch();
};

class CodeCaveHook : public Patch {
	size_t size;
	char OrginalCode[16];

public:
	CodeCaveHook(HMODULE hModule, const short* sign, size_t length, size_t size, int Difference, DWORD *BackAddress, LPVOID Function, bool Call = false, size_t where = 0);
	~CodeCaveHook();
};

class PatchContainer {
	std::set <Patch*> Patches;
public:
	void NewPatch(Patch* newPatch);
	void RemoveAll();
};

extern PatchContainer Patches;

void InstallPatches();