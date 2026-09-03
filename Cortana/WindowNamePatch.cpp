#include "WindowNamePatch.h"

#include "Addresses.h"
#include "Patch.h"

using namespace std;

short WindowNameSig[] = { 0xA1, -1, -1, -1, -1, 0xA3, -1, -1, -1, -1, 0x8A, 0x0D, -1, -1, -1, -1, 0x88, 0x0D, -1, -1, -1, -1, 0xA3, -1, -1, -1, -1, 0x88, 0x0D, -1, -1, -1, -1, 0x68, 0x00, 0x7F, 0x00, 0x00, 0x53 };

void __declspec(noinline) InstallWindowNamePatchCode()
{
	ENCRYPT_BEGIN		
	DWORD Text = 0xFFFFFFFF;
	Patches.NewPatch(new CodePatch(hhalo, WindowNameSig, sizeof(WindowNameSig), 4, 1, (byte*)&Text, 1));
	ENCRYPT_END
}