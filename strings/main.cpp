#include "main.h"

#include "resource.h"

#include "sha256.h"

#include "codefinder.h"
#include "globals.h"
#include "MapCache.h"
#include "NetworkClient.h"

HMODULE SappModule = NULL;
CHandlerPROC CHandler = NULL;

int ConsoleHookHandler(const char* command)
{
	if (!SappModule)
	{
		if (SappModule = GetModuleHandleW(L"sapp.dll"))
		{
			CHandler = (CHandlerPROC)GetProcAddress(SappModule, "CHandler");
		}
	}
	if(!_stricmp(command, "load"))
	{
		if(SappModule == NULL)
		{
			if(LoadLibraryW(L"sapp.dll"))
			{
				PrintConsoleAsm("Successfully loaded sapp.dll!");
			}
			else
			{
				PrintConsoleAsm("Failed to load sapp.dll!");
			}
		}
		else
		{
			PrintConsoleAsm("sapp.dll is already loaded!");
		}
		return 1;
	}
	else if(!_stricmp(command, "unload"))
	{
		if(SappModule != NULL)
		{		
			if (FreeLibrary(SappModule))
			{
				SappModule = NULL;
				CHandler = NULL;
				PrintConsoleAsm("Successfully unloaded sapp.dll!");
			}
			else
			{
				PrintConsoleAsm("Failed to unload sapp.dll!");
			}
		}
		else
		{
			PrintConsoleAsm("sapp.dll is not loaded!");
		}
		return 1;
	}
	else if(SappModule != NULL)
	{
		if(CHandler != NULL)
		{
			return CHandler(command);
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

DWORD StringOffset;
DWORD FormatPrintAddr;
void __declspec(naked)ConsoleHookHandlerCodecave()
{
	__asm
	{
		pushad

		push edi
		call ConsoleHookHandler
		test eax, eax
		jnz cmd_ok
		cmp [CHandler], 0
		jnz cmd_ok
		push StringOffset
		push 0
		call FormatPrintAddr
		add esp, 8
	cmd_ok:
		add esp, 4

		popad
		pop esi
		pop ebx
		add esp, 0x500
		retn
	}
}

bool info_entered;
loginfoStr LogInfo = {};
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_OK:
		{
			DWORD u_size = GetDlgItemText(hDlg, IDC_EDIT1, LogInfo.username, 32);
			DWORD p_size = GetDlgItemText(hDlg, IDC_EDIT2, LogInfo.password, 32);

			if (u_size && p_size)
			{
				SHA256_Data((unsigned char*)LogInfo.password, p_size, (unsigned char*)LogInfo.username, u_size, (unsigned char*)LogInfo.password);

				if (IsDlgButtonChecked(hDlg, IDC_CHECK1) == BST_CHECKED)
				{
					FILE* loginfo;
					if (!fopen_s(&loginfo, "loginfo.dat", "wb") && loginfo)
					{
						fwrite((char*)&LogInfo, 64, 1, loginfo);
						fclose(loginfo);
					}
					else
					{
						MessageBox(0, "Failed to save login data!", "Error", MB_OK | MB_ICONERROR);
					}
				}
				info_entered = true;
				SendMessage(hDlg, WM_CLOSE, 0, 0);
			}
			else
			{
				MessageBox(0, "You have to specify the username and the password!", "Error", MB_OK | MB_ICONERROR);
			}
			return TRUE;
		}
		}
		break;
	}
	case WM_CLOSE:
	{
		DestroyWindow(hDlg);
		return TRUE;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return TRUE;
	}
	}
	return FALSE;
}

void LoadAC()
{
	if (MessageBox(0, "Do you want to start Halo with Anticheat ON?", "Anticheat", MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		WSInit();

		NetworkClient myClient;

		if (myClient.Connect() && myClient.GetRC6() == success)
		{
			LoginState state = invalid_user;
			do
			{
				info_entered = false;

				FILE* loginfo;
				if (!fopen_s(&loginfo, "loginfo.dat", "rb") && loginfo)
				{			
					size_t ret = fread((char*)&LogInfo, 64, 1, loginfo);
					fclose(loginfo);
					info_entered = true;
				}
				else
				{
					HWND hDlg = CreateDialog(GetModuleHandle("Strings.dll"), MAKEINTRESOURCE(IDD_DIALOG1), 0, DialogProc);

					ShowWindow(hDlg, SW_SHOW);

					SetFocus(GetDlgItem(hDlg, IDC_EDIT1));

					BOOL ret;
					MSG msg;
					while ((ret = GetMessage(&msg, 0, 0, 0)) != 0 && ret != -1)
					{
						if (!IsDialogMessage(hDlg, &msg))
						{
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
					}
				}

				if (!info_entered)
					break;

				state = myClient.Login(LogInfo);
				switch (state)
				{
				case success:
					{
						break;
					}
				case connection_terminated:
					{
						MessageBox(0, "Connection terminated by the server!", "Error", MB_OK | MB_ICONERROR);
						break;
					}
				case timeout:
					{
						MessageBox(0, "Connection timed out!", "Error", MB_OK | MB_ICONERROR);
						break;
					}
				case invalid_user:
					{
						remove("loginfo.dat");
						MessageBox(0, "Incorrect username or password!", "Error", MB_OK | MB_ICONERROR);
						break;
					}
				case not_activated:
					{
						MessageBox(0, "Your account is not activated yet!", "Error", MB_OK | MB_ICONERROR);
						break;
					}
				case banned:
					{
						MessageBox(0, "You are BANNED!", "Error", MB_OK | MB_ICONERROR);
						break;
					}
				}
			} while (state == invalid_user);
			
			if (state == success)
			{
				switch (myClient.GetCortana())
				{
				case success:
					{
						break;
					}
				case connection_terminated:
					{
						MessageBox(0, "Connection terminated by the server!", "Error", MB_OK | MB_ICONERROR);
						break;
					}
				case timeout:
					{
						MessageBox(0, "Connection timed out!", "Error", MB_OK | MB_ICONERROR);
						break;
					}
				}
			}
			else
			{
				myClient.Disconnect();
				WSShutDown();
			}
		}
		else
		{
			MessageBox(0, "Failed to connect to the global server, please try again later!", "Error", MB_OK | MB_ICONERROR);
		}
	}
}

DWORD BackToStrings;
void __declspec(naked) LoadACCC()
{
	__asm
	{
		pushad
		pushfd
		call LoadAC
		popfd
		popad

		push edx
		push 0x20019
		jmp BackToStrings
	}
}

void __declspec(naked) CPUsageFixCC()
{
	__asm
	{
		push 1
		call Sleep
		mov al, byte ptr ss : [esp + 0x1b]
		retn 4
	}
}

byte Nops[8] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
byte UsingProfilePathPatch1[6] = { 0xEB, 0x08, 0x90, 0x81, 0xC4, 0x20 }; // jmp short 0x08 / nop / add esp, 0x124...
byte UsingProfilePathPatch2[3] = { 0xEB, 0xE8, 0x90 }; // jmp short 0xE8 / nop
byte UsingProfilePathPatch3[9] = { 0x74, 0x33, 0x8B, 0x04, 0x24, 0x85, 0xC0, 0x74, 0x2C }; // jz short 0x33 / mov eax, dword ptr ss:[esp] / test eax, eax / jz short 0x2c
byte UsingProfilePathPatch4[7] = { 0x04, 0xEB, 0xB9, 0x90, 0x90, 0x90, 0x90 }; // .. 0x04, jmp short 0xB9 / nop nop nop nop
byte IntegrationCheckPatch[5] = { 0xEB, 0x20, 0x90, 0x90, 0x90 }; // jmp short 0x20 / nop nop nop
byte ServerlistPatch[8] = { 0x31, 0xC0, 0x40, 0x90, 0x90, 0x90, 0x0F, 0x84 }; // xor eax, eax / inc eax / nop nop nop / je
byte Portable1Patch[24] = { 0xC7, 0x44, 0x24, 0x28, 0xA4, 0x00, 0x00, 0x00, 0xC7, 0x44, 0x24, 0x2C, 0x03, 0x00, 0x00, 0x00, 0xEB, 0x1F, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }; // mov dword ptr ss : [esp + 0x28], 0a4 / mov dword ptr ss : [esp + 0x2c] / jmp short 0x1f / nop nop nop nop nop nop
byte Portable2Patch[4] = { 0xEB, 0x10, 0x90, 0x90 }; // jmp short 0x10 / nop nop
byte Portable3Patch[5] = { 0x5E, 0x90, 0x90, 0x90, 0x90 }; // pop esi / nop nop nop nop
byte DulicKeyPatch[2] = { 0xEB, 0x61 }; // jmp short 0x61
byte VersionCheckPatch[2] = { 0xEB, 0x1A }; // jmp short 0x1A
byte DevmodePatch[9] = { 0xB0, 0x01, 0xC3, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }; // mov al, 1 / retn

byte halo = 0; // 0 - unknown(?), 1 - pc, 2 - ce

void InstallPatches()
{
	char* HaloStr;
	Patches.NewPatch(new MemoryAddress("Halo Type", HaloTypeCode, 11, (DWORD*)&HaloStr));
	if (!strcmp(HaloStr, "halor"))
		halo = 1;
	else if (!strcmp(HaloStr, "halom"))
		halo = 2;

	Patches.NewPatch(new CodeCaveHook("CPUsage Fix1", CPUsageFix1Code, 6, 0, NULL, CPUsageFixCC, true));
	Patches.NewPatch(new CodeCaveHook("CPUsage Fix2", CPUsageFix1Code, 6, 0, NULL, CPUsageFixCC, true));
	Patches.NewPatch(new CodeCaveHook("CPUsage Fix3", CPUsageFix2Code, 10, 0, NULL, CPUsageFixCC, true));
	Patches.NewPatch(new CodeCaveHook("CPUsage Fix4", CPUsageFix2Code, 10, 0, NULL, CPUsageFixCC, true));
	Patches.NewPatch(new CodeCaveHook("CPUsage Fix5", CPUsageFix2Code, 10, 0, NULL, CPUsageFixCC, true));
	
	if (FindCode((char*)hModule, IsClientCode)) // Client
	{
		Patches.NewPatch(new CodePatch("IntegrationCheck Patch", IntegrationCheckCode, 5, 0, IntegrationCheckPatch));
		Patches.NewPatch(new CodePatch("ServerCheck Patch", ServerCheckCode, 8, 0, ServerlistPatch));
		Patches.NewPatch(new CodePatch("Portable1-1 Patch", Portable1Code, 24, 22, Portable1Patch));
		Patches.NewPatch(new CodePatch("Portable1-2 Patch", Portable1Code, 4, 7, Nops));
		Patches.NewPatch(new CodePatch("Portable2 Patch", Portable2Code, 4, 8, Portable2Patch));
		Patches.NewPatch(new CodePatch("Portable3 Patch", Portable3Code, 5, 34, Portable3Patch));
		Patches.NewPatch(new CodePatch("Portable3 Patch", Portable3Code, 4, 11, Nops));
		Patches.NewPatch(new CodeCaveHook("Strings Patch", LoadACCode, 6, 0, &BackToStrings, LoadACCC));
	}
	else // Server
	{
		Patches.NewPatch(new FunctionAddress("ConsolePrint function", PrintCode, 0, &HaloPrintAddress));
		Patches.NewPatch(new MemoryAddress("Bad Command String Offset", ConsoleHookCode, 1, &StringOffset));
		Patches.NewPatch(new MemoryValueFromOPCode("ConsoleFPrint function", ConsoleHookCode, 7, &FormatPrintAddr));
		Patches.NewPatch(new CodeCaveHook("Console Hook", ConsoleHookCode, 5, -5, NULL, ConsoleHookHandlerCodecave));
		Patches.NewPatch(new CodePatch("UsingProfilePath Patch 1", UsingProfilePathCode1, 6, 25, UsingProfilePathPatch1));
		Patches.NewPatch(new CodePatch("UsingProfilePath Patch 2", UsingProfilePathCode1, 3, 50, UsingProfilePathPatch2));
		Patches.NewPatch(new CodePatch("UsingProfilePath Patch 3", UsingProfilePathCode1, 9, 0, UsingProfilePathPatch3));
		Patches.NewPatch(new CodePatch("UsingProfilePath Patch 4", UsingProfilePathCode2, 7, 2, UsingProfilePathPatch4));
		Patches.NewPatch(new CodePatch("KeyDup Patch", DuplicKeysCode, 2, 11, DulicKeyPatch));
		//Patches.NewPatch(new CodePatch("KeyCheck Patch", CDKeyCheckCodeS, 6, 4, Nops));
		Patches.NewPatch(new CodePatch("VersionCheck Patch", VersionCheckCode, 2, 24, VersionCheckPatch));
		Patches.NewPatch(new CodePatch("ASCII Patch 1", ASCIICheckCode, 2, 11, Nops));
		Patches.NewPatch(new CodePatch("ASCII Patch 2", ASCIICheckCode, 2, 22, Nops));
		Patches.NewPatch(new CodePatch("ASCII Patch 3", ASCIICheckCode, 4, 38, Nops));
		Patches.NewPatch(new CodePatch("Devmode Patch", DevmodeCode, 9, 8, DevmodePatch, 2));
		InstallMapCache();
	}
}

void UninstallPatches()
{
	Patches.RemoveAll();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fwdreason, LPVOID lpvReserved)
{
	switch (fwdreason)
    {
	case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hinstDLL);
			InstallPatches();
			//Install_PLAYER_NUMBER_Players();
			break;
		}
	case DLL_PROCESS_DETACH:
		{
			UninstallPatches();
			break;
		}
	}

	return 1;
}