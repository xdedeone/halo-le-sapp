#pragma once

#include "SuperHeader.h"

#include <string>
#include <vector>

template <class T>
const T* CreateConstForFind();

bool ProcessBoolValue(const std::wstring& str, bool &value);

template <class _Elem, class _Traits>
void TrimSpaces(std::basic_string<_Elem, _Traits> &str)
{
	size_t startpos = str.find_first_not_of(CreateConstForFind<_Elem>());
	size_t endpos = str.find_last_not_of(CreateConstForFind<_Elem>());

	if (endpos == std::basic_string<_Elem, _Traits>::npos)
		str.clear();
	else if (endpos - startpos + 1 == str.length())
		return;
	else
		str = str.substr(startpos, endpos - startpos + 1);
}

template <class _Elem, class _Traits>
bool Split(std::vector<std::basic_string<_Elem, _Traits>> &result, const std::basic_string<_Elem, _Traits> &s, _Elem c)
{
	size_t offset = 0;
	while (true)
	{
		size_t i = s.find(c, offset);

		if (i == -1)
		{
			if (offset == 0)
			{
				return false;
			}
			else
			{
				result.emplace_back(s.substr(offset, s.length() - offset));
				return true;
			}
		}
		result.emplace_back(s.substr(offset, i - offset));
		offset = i + 1;
	}
}

template <class T>
void ProcessOperator(const std::wstring &arg, T &dest)
{
	T tmp;
	switch (arg[0])
	{
	case '%':
	{
		if (ToNumber(arg.substr(1), tmp))
			dest %= tmp;
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
		if (ToNumber(arg.substr(1), tmp) && tmp != 0)
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

template <>
void ProcessOperator(const std::wstring &arg, float &dest);

bool wildicmp(const wchar_t* wild, const wchar_t* str);

void StringReplace(std::wstring &str, const std::wstring &to_replace, const std::wstring &replacement);

template <class T, class _Elem, class _Traits>
T ToNumberT(const std::basic_string<_Elem, _Traits> &string_int);

template <class T, class _Elem, class _Traits>
bool ToNumber(const std::basic_string<_Elem, _Traits> &str, T &s)
{
	if (!str.empty() && ((str[0] >= '0' && str[0] <= '9') || str[0] == '-'))
	{
		for (size_t k = 1; k < str.length(); k++)
		{
			if ((str[k] < '0' || str[k] > '9') && str[k] != '.')
				return false;
		}
		s = ToNumberT<T>(str);
		return true;
	}
	return false;
}

std::string ToStr(int number);
std::string ToStr(float number);
std::wstring ToWStr(int number);
std::wstring ToWStr(float number);

std::string WToStr(const std::wstring &orig);

void ToLower(std::string& str);
void ToLower(char *s);
void ToLower(wchar_t *s);
wchar_t WCharToLower(wchar_t c);
//void ToUpper(char *s);
//void ToUpper(wchar_t *s);

std::string FormatArgs(const char* fmt, ...);
std::wstring FormatWArgs(const wchar_t* fmt, ...);

bool TryParseVect3f(const std::wstring &str, vect3f& v);

std::string ConvertToMD5Hash(unsigned char* array);
