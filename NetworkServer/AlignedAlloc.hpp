#pragma once

class AlignedAlloc
{
	void* oPtr;
	void* myPtr;
public:
	AlignedAlloc(AlignedAlloc&& aa)
	{
		oPtr = aa.oPtr;
		myPtr = aa.myPtr;
		aa.oPtr = nullptr;
		aa.myPtr = nullptr;
	}
	AlignedAlloc(size_t size)
	{
		oPtr = new char[size + 16];
		myPtr = (void*)((size_t)oPtr + (16 - ((size_t)oPtr % 16)));
	}	
	AlignedAlloc& operator=(AlignedAlloc&& aa)
	{
		if (this != &aa)
		{
			delete[] oPtr;
			oPtr = aa.oPtr;
			myPtr = aa.myPtr;
			aa.oPtr = nullptr;
			aa.myPtr = nullptr;
		}
		return *this;
	}
	
	virtual ~AlignedAlloc()
	{
		delete[] oPtr;
	}
	void* operator()()
	{
		return myPtr;
	}
};