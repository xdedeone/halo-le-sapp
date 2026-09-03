#include "ScreenShots.h"

#include <d3d9.h>

#include "jpge.h"

#include "Addresses.h"
#include "Patch.h"

#define SystemFunction036 NTAPI SystemFunction036
#include <NTSecAPI.h>
#undef SystemFunction036

#pragma comment(lib, "d3d9.lib")

short PresentSig[] = { 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x00, 0x50, 0xFF, 0x51, 0x44 }; // 6A006A006A006A0050FF5144
short LookEnabledSig[] = { 0x8B, 0x0D, -1, -1, -1, -1, 0x32, 0xC0, 0x85, 0xC9, 0x0F, 0x84, -1, -1, -1, -1, 0xA0, -1, -1, -1, -1, 0x84, 0xC0 }; // 8B0D????????32C085C90F84????????A0????????84C0
short RconBuffSig[] = { 0x53, 0x55, 0x56, 0x57, 0xBB, -1, -1, -1, -1, 0x6A, 0x20, 0x53 }; // 53555657BB????????6A2053

LPDIRECT3DSURFACE9 offscreen_surface = NULL;
unsigned char *downsampled = NULL;

DWORD Width = 0, Height = 0;

HANDLE ScreenShotSemaphore(CreateSemaphore(NULL, 0, 1, NULL));

bool shot = false;

bool* LookEnabled;

DWORD* rcon_buff;

DWORD GetRand(size_t min, size_t max)
{
	DWORD rand;
	RtlGenRandom(&rand, sizeof(DWORD));
	rand = (rand % (max - min)) + min;
	return rand;
}

void SaveThread(LPVOID p)
{
	ENCRYPT_BEGIN
	NtSIT(GetCurrentThread(), 0x11, 0, 0);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);	
	ENCRYPT_END
	while (true)
	{
		Sleep(GetRand(30000, 120000));
		shot = true;

		WaitForSingleObject(ScreenShotSemaphore, INFINITE);

		ENCRYPT_BEGIN
		
		size_t width;
		size_t height;

		if (*LookEnabled || *rcon_buff != 0x6e6f6372) // "rcon"
		{
			width = Width / 2;
			height = Height / 2;
		}
		else
		{
			width = Width / 2;
			height = Height / 2;
			height = height - size_t(ceil(height * 0.033333333)) - 1;
		}

		int resultBufSize = width * height * 3;
		char *pBufferCompressed = new char[resultBufSize];

		if (jpge::compress_image_to_jpeg_file_in_memory(pBufferCompressed, resultBufSize, width, height, 3, downsampled, jpge::params(50, jpge::H2V2, false, true)))
		{
			myClient->SendBigEvent(event_screenshot, pBufferCompressed, resultBufSize);
		}

		delete[] pBufferCompressed;
		ENCRYPT_END
	}
}

void __stdcall PresentHook(LPDIRECT3DDEVICE9 pDevice)
{
	//PerfomanceCounter QPC;
	//QPC.StartQPC();
	ENCRYPT_BEGIN
	
	shot = false;

	LPDIRECT3DSURFACE9 back_buffer = NULL;
	pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);

	D3DSURFACE_DESC Desc;
	back_buffer->GetDesc(&Desc);

	if (Desc.Width != Width || Desc.Height != Height)
	{
		Width = Desc.Width;
		Height = Desc.Height;

		if (offscreen_surface != NULL)
			offscreen_surface->Release();
		pDevice->CreateOffscreenPlainSurface(Desc.Width, Desc.Height, Desc.Format, D3DPOOL_SYSTEMMEM, &offscreen_surface, NULL);

		delete[] downsampled;
		downsampled = new unsigned char[Width * Height * 3 / 4];
	}

	pDevice->GetRenderTargetData(back_buffer, offscreen_surface);

	D3DLOCKED_RECT d3dlr;
	offscreen_surface->LockRect(&d3dlr, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_READONLY);

	unsigned char* buffer = (unsigned char*)d3dlr.pBits;

	size_t width = Width / 2;
	size_t height = Height / 2;

	// Creates a half sized "RGB" picture from the full size "BGRX" one
	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < width; x++)
		{
			downsampled[x * 3 + y * width * 3] = ((size_t)buffer[x * 8 + 2 + y * width * 16] + (size_t)buffer[(x * 2 + 1) * 4 + 2 + y * width * 16] + (size_t)buffer[x * 8 + 2 + (y * 2 + 1) * width * 8] + (size_t)buffer[(x * 2 + 1) * 4 + 2 + (y * 2 + 1) * width * 8]) / 4;
			downsampled[x * 3 + 1 + y * width * 3] = ((size_t)buffer[x * 8 + 1 + y * width * 16] + (size_t)buffer[(x * 2 + 1) * 4 + 1 + y * width * 16] + (size_t)buffer[x * 8 + 1 + (y * 2 + 1) * width * 8] + (size_t)buffer[(x * 2 + 1) * 4 + 1 + (y * 2 + 1) * width * 8]) / 4;
			downsampled[x * 3 + 2 + y * width * 3] = ((size_t)buffer[x * 8 + y * width * 16] + (size_t)buffer[(x * 2 + 1) * 4 + y * width * 16] + (size_t)buffer[x * 8 + (y * 2 + 1) * width * 8] + (size_t)buffer[(x * 2 + 1) * 4 + (y * 2 + 1) * width * 8]) / 4;
		}
	}

	offscreen_surface->UnlockRect();

	back_buffer->Release();

	ReleaseSemaphore(ScreenShotSemaphore, 1, 0);

	ENCRYPT_END

	//DWORD ms = QPC.GetQPC();

	//char cbuff[32];
	//sprintf(cbuff, "This took %u ms!", ms);

	//MessageBox(0, cbuff, "Result", 0);
}

DWORD Device;
DWORD BackToPresent;
void __declspec(naked) PresentCC()
{
	__asm
	{
		ENCRYPT_BEGIN
		mov	Device, eax

		push 0
		push 0
		push 0
		push 0
		push eax
		call dword ptr ds : [ecx + 0x44]
		
		cmp byte ptr ds: [shot], 0
		je no_shot
		pushad
		mov eax, Device
		push eax
		call PresentHook
		popad
	no_shot:
		ENCRYPT_END
		jmp BackToPresent
	}
}

void __declspec(noinline) InstallPresentHookCode()
{
	ENCRYPT_BEGIN
	_beginthread(SaveThread, 0, 0);
	Patches.NewPatch(new MemoryAddress(hhalo, LookEnabledSig, sizeof(LookEnabledSig), 17, (DWORD*)&LookEnabled));
	Patches.NewPatch(new MemoryAddress(hhalo, RconBuffSig, sizeof(RconBuffSig), 5, (DWORD*)&rcon_buff));
	Patches.NewPatch(new CodeCaveHook(hhalo, PresentSig, sizeof(PresentSig), 12, 0, (DWORD*)&BackToPresent, PresentCC));
	ENCRYPT_END
}