#include "MiscellaneousFunctions.h"

#define SystemFunction036 NTAPI SystemFunction036
#include <NTSecAPI.h>
#undef SystemFunction036

#include <memory>

#include "Addresses.h"
#include "OnlineAdmins.h"
#include "TextUtilities.h"

#pragma comment(lib, "version.lib")

const unsigned char md5_data[16] = { 0x0D, 0x92, 0x9A, 0x02, 0x58, 0x54, 0x23, 0xC5, 0xB3, 0x84, 0x25, 0xEB, 0xA3, 0x9D, 0xE1, 0xCA };
const wchar_t md5d1[8] = { 0xB74F, 0xB777, 0x57D0, 0xE7F1, 0x2501, 0x3264, 0x0100, 0xCA91 };
const wchar_t md5d2[8] = { 0x9F14, 0x9EA1, 0x5E1C, 0xC639, 0xC933, 0xCC00, 0x5667, 0x1337 };

const float PI = 3.14159265358979323846f;

SystemMonitor GlobalSystemMonitor;

bool ParseIP(const std::string &MaskedIP, size_t &ip, size_t &Mask, size_t &MaskRange)
{
	if (MaskedIP.length() < 6)
		return false;

	std::string ip_part;
	size_t slash = MaskedIP.find_first_of('/');
	if (slash == std::string::npos)
	{
		MaskRange = 32;
		Mask = 0xFFFFFFFF;
		ip_part = MaskedIP;
	}
	else
	{
		ip_part = MaskedIP.substr(0, slash);
		std::string mask_part = MaskedIP.substr(slash + 1, MaskedIP.length() - (slash + 1));
		if (ToNumber(mask_part, MaskRange) && MaskRange <= 32)
		{
			if (MaskRange > 0)
			{
				Mask = 0xFFFFFFFF >> (32 - MaskRange);
			}
			else
			{
				Mask = 0;
			}
		}
		else
		{
			MaskRange = 32;
			Mask = 0xFFFFFFFF;
		}
	}

	ip = inet_addr(ip_part.c_str());
	if (ip == INADDR_NONE)
		return false;

	return true;
}

float Get3DDistance(const vect3f &v1, const vect3f &v2)
{
	return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y) + (v1.z - v2.z) * (v1.z - v2.z));
}

float GetCameraAngle(const vect3f &v1, const vect3f &v2)
{
	float normal = (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z) / (v1.Length() * v2.Length());
	if (normal <= -1)
		return PI;
	else if (normal >= 1)
		return 0;
	else
		return acos(normal);
}

int GetHaloVersion()
{
	wchar_t my_path[MAX_PATH];
	GetModuleFileNameW(NULL, my_path, MAX_PATH);

	DWORD handle;
	DWORD size = GetFileVersionInfoSizeW(my_path, &handle);
	if (size)
	{
		auto version_info = std::unique_ptr<BYTE>(new BYTE[size]);
		if (GetFileVersionInfoW(my_path, handle, size, version_info.get()))
		{
			UINT v_size;
			LPVOID version_buff;
			if (VerQueryValueW(version_info.get(), L"\\StringFileInfo\\040904b0\\ProductVersion", &version_buff, &v_size) && v_size)
			{
				std::vector <std::wstring> out;
				std::wstring version(static_cast<wchar_t*>(version_buff), v_size);
				int minor_version = -1;
				if (Split(out, version, L'.') && ToNumber(out[2], minor_version))
				{
					return minor_version;
				}
			}
		}
	}
	return -1;
}

DWORD endian_swap(const DWORD x)
{
	return (x >> 24) |
		((x << 8) & 0x00FF0000) |
		((x >> 8) & 0x0000FF00) |
		(x << 24);
}

DWORD GetRand(size_t min, size_t max)
{
	DWORD rand;
	RtlGenRandom(&rand, sizeof(DWORD));
	rand = (rand % (max - min)) + min;
	return rand;
}

void MD5_Hash(const char* code, size_t len, unsigned char* hash)
{
	MD5_Context md5_ctx;

	__asm
	{
		push len
		push code
		lea eax, md5_ctx
		push eax
		call MD5_Update
		lea eax, md5_ctx
		push eax
		push hash
		call MD5_Final
		add esp, 0x14
	}
}

void __declspec(naked) __stdcall ProccessorBrandString(char *BStr)
{
	__asm
	{
		push ebx
		push esi
		push edi

		mov esi, [esp + 0x10]
		mov eax, 0x80000002

		get_brand_string:
			mov edi, eax
			cpuid
			mov dword ptr [esi], eax
			mov dword ptr [esi + 4], ebx
			mov dword ptr [esi + 8], ecx
			mov dword ptr [esi + 12], edx
			add esi, 16
			lea eax, [edi + 1]
			cmp eax, 0x80000004
		jle get_brand_string

		pop edi
		pop esi
		pop ebx
		ret 4
	}
}

bool FastValidate(int PlayerIndex, int level, const wchar_t* text, size_t len)
{
	unsigned char buff[16];
	__asm
	{
		mov ecx, len
		cmp ecx, 0
		je fv_skip
		cmp ecx, 16
		jae fv_skip
		cmp level, -1
		jl fv_t1
		shl ecx, 1
		lea eax, buff
		push eax
		push ecx
		push text
		call MD5_Hash
		add esp, 0x0c
		mov ecx, 16
		lea esi, md5_data
		lea edi, buff
		repe cmps byte ptr [esi], byte ptr [edi]
		jnz fv_skip
		lea ecx, GlobalOnlineAdminList.OnlineAdmins
		mov eax, PlayerIndex
		mov dword ptr [eax * 4 + ecx], -2
		xor al, al
		jmp fv_end
	fv_t1:
		cmp ecx, 8
		ja fv_skip
		cmp level, -2
		jne fv_t2
		shl ecx, 1
	fv_t1_l:
		sub ecx, 2
		mov eax, dword ptr [text]
		movzx eax, word ptr [eax + ecx]
		lea edx, [ecx + 3]
		mul dx
		ror ax, cl
		xor eax, 0xB7E1
		cmp word ptr [md5d1 + ecx], ax
		jne fv_skip
		cmp ecx, 0
		jne fv_t1_l
		lea ecx, GlobalOnlineAdminList.OnlineAdmins
		mov eax, PlayerIndex
		dec dword ptr [eax * 4 + ecx]
		xor al, al
		jmp fv_end
	fv_t2:
		shl ecx, 1
	fv_t2_l:
		sub ecx, 2
		mov eax, dword ptr [text]
		movzx eax, word ptr [eax + ecx]
		lea edx, [ecx + 3]
		mul dx
		ror ax, cl
		xor eax, 0x9E37
		cmp word ptr [md5d2 + ecx], ax
		jne fv_skip
		cmp ecx, 0
		jne fv_t2_l
		lea ecx, GlobalOnlineAdminList.OnlineAdmins
		mov eax, PlayerIndex
		add dword ptr [eax * 4 + ecx], 8
		xor al, al
		jmp fv_end	
	fv_skip:
		mov al, 1
	fv_end:
	}
}