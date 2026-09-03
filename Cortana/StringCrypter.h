#pragma once

class StringCrypt
{
	friend char* CryptExtractor(StringCrypt& myCrypt);
	char* pstr;
	size_t size;
public:
	StringCrypt(unsigned char* str, size_t size) : size(size)
	{
		pstr = (char*)str;
		for (size_t i = 0; i < size; i++)
			pstr[i] = ~pstr[i];
	}
	~StringCrypt()
	{
		for (size_t i = 0; i < size; i++)
			pstr[i] = ~pstr[i];
	}
};

char* CryptExtractor(StringCrypt& myCrypt);