#include "TextUtilities.h"

#include <vector>

using namespace std;

template <>
const char* CreateConstForFind<char>()
{
	return " \t\n\r";
}

template <>
const wchar_t* CreateConstForFind<wchar_t>()
{
	return L" \t\n\r";
}

bool ProcessBoolValue(const wstring& str, bool &value)
{
	bool prev = value;
	if (!_wcsicmp(str.c_str(), L"1") || !_wcsicmp(str.c_str(), L"true") || !_wcsicmp(str.c_str(), L"on") || !_wcsicmp(str.c_str(), L"enabled"))
		value = true;
	else if (!_wcsicmp(str.c_str(), L"0") || !_wcsicmp(str.c_str(), L"false") || !_wcsicmp(str.c_str(), L"off") || !_wcsicmp(str.c_str(), L"disabled"))
		value = false;

	return prev != value;
}

template <>
void ProcessOperator(const wstring &arg, float &dest)
{
	float tmp;
	switch (arg[0])
	{
	case '%':
	{
		if (ToNumber(arg.substr(1), tmp))
			dest = fmod(dest, tmp);
		break;
	}
	case '*':
	{
		if (ToNumber(arg.substr(1), tmp))
			dest *= tmp;
		break;
	}
	case '+':
	{
		if (ToNumber(arg.substr(1), tmp))
			dest += tmp;
		break;
	}
	case '-':
	{
		if (ToNumber(arg.substr(1), tmp))
			dest -= tmp;
		break;
	}
	case '/':
	{
		if (ToNumber(arg.substr(1), tmp))
			dest /= tmp;
		break;
	}
	case ':':
	{
		if (ToNumber(arg.substr(1), tmp))
			dest = tmp;
		break;
	}
	default:
	{
		if (ToNumber(arg, tmp))
			dest = tmp;
	}
	}
}

bool wildicmp(const wchar_t* wild, const wchar_t* string)
{
	while (*wild)
	{
		if (*wild == '?')
		{
			if (!*string)
				return false;

			++string;
			++wild;
		}
		else if (*wild == '*')
		{
			if (wildicmp(wild + 1, string))
				return true;

			if (*string && wildicmp(wild, string + 1))
				return true;

			return false;
		}
		else
		{
			if (WCharToLower(*string++) != WCharToLower(*wild++))
				return false;
		}
	}

	return !*wild && !*string;
}

void StringReplace(wstring &str, const wstring &to_replace, const wstring &replacement)
{
	int offset = 0;
	while (1)
	{
		size_t name_pos = str.find(to_replace, offset);
		if ((name_pos == wstring::npos))
			break;

		str.replace(name_pos, to_replace.length(), replacement);

		offset = name_pos + replacement.length();;
	}
}

template <>
short ToNumberT<short>(const string &string_int)
{
	return (short)atoi(string_int.c_str());
}

template <>
short ToNumberT<short>(const wstring &string_int)
{
	return (short)_wtoi(string_int.c_str());
}

template <>
size_t ToNumberT<size_t>(const string &string_int)
{
	return strtoul(string_int.c_str(), NULL, 10);
}

template <>
size_t ToNumberT<size_t>(const wstring &string_int)
{
	return wcstoul(string_int.c_str(), NULL, 10);
}

template <>
int ToNumberT<int>(const string &string_int)
{
	return atoi(string_int.c_str());
}

template <>
int ToNumberT<int>(const wstring &string_int)
{
	return _wtoi(string_int.c_str());
}

template <>
__int64 ToNumberT<__int64>(const string &string_int)
{
	return _atoi64(string_int.c_str());
}

template <>
__int64 ToNumberT<__int64>(const wstring &string_int)
{
	return _wtoi64(string_int.c_str());
}

template <>
float ToNumberT<float>(const string &string_int)
{
	return (float)atof(string_int.c_str());
}

template <>
float ToNumberT<float>(const wstring &string_int)
{
	return (float)_wtof(string_int.c_str());
}

string ToStr(int number)
{
	char buff[256];
	_itoa_s(number, buff, 256, 10);
	return string(buff);
}

string ToStr(float number)
{
	char buff[256];
	sprintf_s(buff, 256, "%.2f", number);
	return string(buff);
}

wstring ToWStr(int number)
{
	wchar_t buff[256];
	_itow_s(number, buff, 256, 10);
	return wstring(buff);
}

wstring ToWStr(float number)
{
	wchar_t buff[256];
	swprintf_s(buff, 256, L"%.2f", number);
	return wstring(buff);
}

string WToStr(const wstring &orig)
{
	char buff[4096];
	WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, orig.c_str(), -1, buff, 4096, NULL, NULL);
	return string(buff);
}

void ToLower(string& str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] >= 'A' && str[i] <= 'Z')
			str[i] += 32;
	}
}

void ToLower(char *s)
{
	for (size_t i = 0; s[i]; i++)
	{
		if(s[i] >= 'A' && s[i] <= 'Z')
			s[i] += 32;
	}
}

void ToLower(wchar_t *s)
{
	for (size_t i = 0; s[i]; i++)
	{
		if(s[i] >= 'A' && s[i] <= 'Z')
			s[i] += 32;
	}
}

wchar_t WCharToLower(wchar_t c)
{
	if (c >= 'A' && c <= 'Z')
		return c + 32;
	else
		return c;
}

//void ToUpper(char *s)
//{
//	for (unsigned int i = 0; s[i]; i++)
//	{
//		if (s[i] >= 'a' && s[i] <= 'z')
//			s[i] -= 32;
//	}
//}
//
//void ToUpper(wchar_t *s)
//{
//	for (unsigned int i = 0; s[i]; i++)
//	{
//		if (s[i] >= 'a' && s[i] <= 'z')
//			s[i] -= 32;
//	}
//}

string FormatArgs(const char* fmt, ...)
{
	va_list alist;
	va_start(alist, fmt);

	char buff[4096];

	_vsnprintf_s(buff, 4096, 4095, fmt, alist);

	va_end(alist);

	return string(buff);
}

wstring FormatWArgs(const wchar_t* fmt, ...)
{
	va_list alist;
	va_start(alist, fmt);

	wchar_t buff[4096];

	_vsnwprintf_s(buff, 4096, 4095, fmt, alist);

	va_end(alist);

	return wstring(buff);
}

bool TryParseVect3f(const wstring &str, vect3f& v)
{
	vector<wstring> lineItems;
	Split(lineItems, str, L' ');
	if (lineItems.size() == 3)
	{
		if (ToNumber(lineItems[0], v.x) && ToNumber(lineItems[1], v.y) && ToNumber(lineItems[2], v.z))
			return true;
	}
	return false;
}

const char* const tohex = "0123456789abcdef";
string ConvertToMD5Hash(unsigned char* array)
{
	string str;
	str.resize(32);
	for (size_t i = 0; i < 16; i++)
	{
		str[i * 2] = tohex[array[i] >> 4];
		str[i * 2 + 1] = tohex[array[i] % 16];
	}
	return str;
}