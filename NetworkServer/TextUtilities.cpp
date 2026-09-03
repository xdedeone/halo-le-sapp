#include "TextUtilities.h"

#include <Windows.h>

using namespace std;

DWORD endian_swap(const DWORD x)
{
	return (x >> 24) |
		((x << 8) & 0x00FF0000) |
		((x >> 8) & 0x0000FF00) |
		(x << 24);
}

void ToLower(char *s)
{
	for (unsigned int i = 0; s[i]; i++)
	{
		if (s[i] >= 'A' && s[i] <= 'Z')
			s[i] += 32;
	}
}

void ToLower(string &s)
{
	for (unsigned int i = 0; i < s.size(); i++)
	{
		if (s[i] >= 'A' && s[i] <= 'Z')
			s[i] += 32;
	}
}

const char* const tohex = "0123456789abcdef";
string ConvertToString(const unsigned char* buff)
{
	string str;
	str.resize(64);
	for (size_t i = 0; i < 32; i++)
	{
		str[i * 2] = tohex[buff[i] >> 4];
		str[i * 2 + 1] = tohex[buff[i] % 16];
	}
	return str;
}

bool ToNumber(const string &string_int, size_t &l)
{
	if (string_int.size() && (string_int[0] >= '0' && string_int[0] <= '9'))
	{
		for (unsigned int k = 1; k < string_int.length(); k++)
		{
			if (string_int[k] < '0' || string_int[k] > '9')
				return false;
		}
		l = strtoul(string_int.c_str(), NULL, 10);
		return true;
	}
	return false;
}

void ToHexNumber(const string &string_int, size_t &l)
{
	l = strtoul(string_int.c_str(), NULL, 16);
}

thread_local char format_buff[1024];

string WToStr(const wstring &orig)
{
	WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, orig.c_str(), -1, format_buff, 1024, NULL, NULL);
	return string(format_buff);
}

string ToStr(int number)
{
	char buff[256];
	_itoa_s(number, buff, 256, 10);
	return string(buff);
}

string FormatArgs(const char* fmt, ...)
{
	va_list alist;
	va_start(alist, fmt);

	_vsnprintf_s(format_buff, 1024, 1023, fmt, alist);

	va_end(alist);

	return string(format_buff);
}

wstring FormatWArgs(const wchar_t* fmt, ...)
{
	va_list alist;
	va_start(alist, fmt);

	_vsnwprintf_s((wchar_t*)format_buff, 512, 511, fmt, alist);

	va_end(alist);

	return wstring((wchar_t*)format_buff);
}

DWORD FindSequence(const char* Buffer, const char* Sequence, size_t BufferSize, size_t SequenceSize)
{
	__asm
	{
		mov edx, Sequence
		mov edi, Buffer
		mov ecx, BufferSize
		BEGIN_FOR:
			mov al, [edx]				
			repne scas byte ptr es:[edi]
			test ecx, ecx
			je NOT_FOUND
			mov ebx, 1
			SCAN_NEXT_BYTES:
				mov al, [edx+ebx]
				cmp [edi], al
				jne END_FOR
				inc edi		
				inc ebx
				cmp SequenceSize, ebx
				jne SCAN_NEXT_BYTES
			mov eax, edi
			sub eax, ebx
			jmp END_SEARCH
		END_FOR:
			inc edi
			jmp BEGIN_FOR
		NOT_FOUND:
			xor eax, eax
	END_SEARCH:
	}
}