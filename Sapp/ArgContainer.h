#pragma once

#include <string>
#include <vector>

class ArgContainer
{
	std::vector <std::wstring> args;
public:
	size_t argc = 0;
	ArgContainer(const std::wstring &arg);
	std::wstring& operator[](size_t i);
};