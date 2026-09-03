#pragma once

#include <Windows.h>

#include <map>

#include "codefinder.h"
#include "signatures.h"

class Patch {
	friend class PatchContainer;
protected:
	const char* Name;
	DWORD PatchAddress;
	bool found;
public:
	Patch(const char* name, const char* sign, int difference);
	virtual ~Patch() = 0;
};

class FunctionAddress : public Patch {
public:
	FunctionAddress(const char* name, const char* sign, int Difference, DWORD *Address);
};

class MemoryAddress : public Patch {
public:
	MemoryAddress(const char* name, const char* sign, int Difference, DWORD *Value);
};

class MemoryValue : public Patch {
public:
	MemoryValue(const char* name, const char* sign, int Difference, DWORD *Value);
};

class MemoryValueFromOPCode : public Patch {
public:
	MemoryValueFromOPCode(const char* name, const char* sign, int Difference, DWORD *Value);
};

class CodePatch : public Patch {
	size_t size;
	char OrginalCode[32];

public:
	CodePatch(const char* name, const char* sign, size_t size, int Difference, const byte* PatchCode, DWORD Pointer = 0);
	~CodePatch();
};

class CodeCaveHook : public Patch {
	size_t size;
	char OrginalCode[32];

public:
	CodeCaveHook(const char* name, const char* sign, size_t size, int Difference, DWORD *BackAddress, LPVOID Function, bool Call = false);
	~CodeCaveHook();
};

class APIHook : public Patch {
	size_t size;
	char OrginalCode[32];

public:
	APIHook(const char* name, const char* sign, int Difference, DWORD *BackAddress, LPVOID Function, bool Call = false);
	~APIHook();
};

class APIReplace : public Patch {
	DWORD Original;

public:
	APIReplace(const char* name, const char* sign, int Difference, LPVOID *OldFunction, LPVOID NewFunction);
	~APIReplace();
};

class DelayedAPIReplace : public Patch {
	DWORD Original;

public:
	DelayedAPIReplace(const char* name, const char* sign, const char* Module, const char* Process, int Difference, LPVOID *OldFunction, LPVOID NewFunction);
	~DelayedAPIReplace();
};

class PatchContainer {
	std::map <std::string, Patch*> PatchMap;
public:
	void NewPatch(Patch* newPatch);
	void Remove(const char* Name);
	void RemoveAll();
};

extern PatchContainer Patches;