#pragma once
#pragma warning(disable: 4146)

//#include <Windows.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

template <class T>
class lfsrHashTable
{
	T* HashTable;
	WORD lfsr_begin;
	WORD lfsr_end;
	WORD mysize;
	
	lfsrHashTable(const lfsrHashTable&);

public:
	lfsrHashTable()
	{
		HashTable = new T[65536];
		memset(HashTable, 0, sizeof(T)*65536);

		lfsr_end = GetTickCount() % 65536;
		if(lfsr_end == 0)
			lfsr_end++;

		lfsr_end = (lfsr_end >> 1) ^ (-(lfsr_end & 1u) & 0xB400u);
		lfsr_begin = lfsr_end;
		mysize = 0;
	}
	~lfsrHashTable()
	{
		delete[] HashTable;
	}
	T& operator[](size_t ID)
	{
		if(ID < 65536)
			return HashTable[ID];
		else
			return HashTable[0];
	}
	const T& operator[](size_t ID) const
	{
		if(ID < 65536)
			return HashTable[ID];
		else
			return HashTable[0];
	}
	WORD size()
	{
		return mysize;
	}
	bool empty()
	{
		return lfsr_begin == lfsr_end;
	}
	void insert(T& object)
	{
		HashTable[lfsr_end] = object;
		lfsr_end = (lfsr_end >> 1) ^ (-(lfsr_end & 1u) & 0xB400u);
		mysize++;
	}
	void clear()
	{
		lfsr_begin = lfsr_end;
		memset(HashTable, 0, sizeof(T)*65536);
		mysize = 0;
	}

	class iterator
	{
		T* currentTable;
		WORD current;
	public:

		iterator(T* currentTableEntry, WORD ID) : currentTable(currentTableEntry), current(ID) {}

		iterator& operator++() // pre
		{
			current = (current >> 1) ^ (-(current & 1u) & 0xB400u);
			return *this;
		}

		bool operator==(const iterator& it) const
		{
			return this->current == it.current;
		}

		bool operator!=(const iterator& it) const
		{
			return this->current != it.current;
		}

		T& operator*() const
		{
			return currentTable[current];
		}

		T* operator->() const
		{
			return &currentTable[current];
		}

		WORD Current()
		{
			return current;
		}
	};

	iterator begin() const
	{
		return iterator(HashTable, lfsr_begin);
	}
	iterator end() const
	{
		return iterator(HashTable, lfsr_end);
	}
};