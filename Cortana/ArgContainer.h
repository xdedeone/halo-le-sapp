#pragma once

#include <string>
#include <vector>

class ArgContainer
{
	std::vector <std::string> args;
public:
	size_t argc;
	ArgContainer(const std::string &arg);
	const std::string& operator[](size_t i) const;
};