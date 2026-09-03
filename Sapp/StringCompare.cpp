#include "StringCompare.h"

bool StringICmp::operator() (const std::string& s1, const std::string& s2) const
{
	return _stricmp(s1.c_str(), s2.c_str()) < 0;
}

bool WStringICmp::operator() (const std::wstring& s1, const std::wstring& s2) const
{
	return _wcsicmp(s1.c_str(), s2.c_str()) < 0;
}