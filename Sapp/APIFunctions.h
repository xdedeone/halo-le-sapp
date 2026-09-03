#pragma once

struct lua_State;

namespace lua
{

void RegisterFunctions(lua_State* state);

template <class T>
T* ReadMemory(T* Address, bool safe_read, size_t size = sizeof(T))
{
	if (safe_read)
	{
		size_t count = 0;	
		do
		{
			MEMORY_BASIC_INFORMATION Info;
			if (VirtualQuery(&Address, &Info, sizeof(MEMORY_BASIC_INFORMATION)))
			{
				if (Info.AllocationProtect <= 1 /*PAGE_NOACCESS*/ || Info.AllocationProtect & 0x100 /*PAGE_GUARD*/)
				{
					return nullptr;
				}
			}
			else
			{
				return nullptr;
			}
			count += (size_t)Address - (size_t)Info.BaseAddress;
		} while (count < size);

		return Address;
	}
	else
	{
		return Address;
	}
}

template <class T>
bool WriteMemory(T* Address, T Value, bool safe_write, size_t size = sizeof(T))
{
	if (safe_write)
	{
		DWORD OldProtect;
		if (VirtualProtect(Address, size, 0x40 /*PAGE_EXECUTE_READWRITE*/, &OldProtect))
		{
			*Address = Value;
			VirtualProtect(Address, size, OldProtect, &OldProtect);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		*Address = Value;
		return true;
	}
}
}