#pragma once

#include <string>

class StringICmp
{
public:
	bool operator() (const std::string& s1, const std::string& s2) const;
};

class WStringICmp
{
public:
	bool operator() (const std::wstring& s1, const std::wstring& s2) const;
};