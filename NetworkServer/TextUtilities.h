#pragma once

#include <string>

unsigned long endian_swap(const unsigned long x);

void ToLower(char *s);
void ToLower(std::string &s);

std::string ConvertToString(const unsigned char* buff);

bool ToNumber(const std::string &string_int, size_t &l);

void ToHexNumber(const std::string &string_int, size_t &l);

std::string WToStr(const std::wstring &orig);

std::string ToStr(int number);

std::string FormatArgs(const char* fmt, ...);

std::wstring FormatWArgs(const wchar_t* fmt, ...);

unsigned long FindSequence(const char* Buffer, const char* Sequence, size_t BufferSize, size_t SequenceSize);