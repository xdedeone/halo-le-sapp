#include <stdio.h>
#include <Windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fwdreason, LPVOID lpvReserved)
{
	switch (fwdreason)
	{
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hinstDLL);
		HMODULE hModule = GetModuleHandleW(L"strings.dll");
		if (hModule != NULL)
		{
			if (FreeLibrary(hModule))
			{
				if (!remove("strings.dll"))
				{
					if (!rename("strings.dat", "strings.dll"))
					{
						if (LoadLibraryW(L"strings.dll"))
						{
							return 1;
						}
					}
				}
			}		
		}
		return 0;
	}
	//case DLL_PROCESS_DETACH:
	//{		
	//	break;
	//}
	}

	return 1;
}