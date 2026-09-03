//#pragma once
//
//#include <sstream>
//#include <string>
//#include <vector>
//
//const size_t Max_Stack_Table_Size = 262144; // 256 KB
//
//class StackAllocator
//{
//	std::vector<void*> stacks;
//	size_t allocated;
//
//	void alloc()
//	{
//		void* original_ptr = ::operator new(Max_Stack_Table_Size + 64 + sizeof(void*));
//
//		void* ptr = reinterpret_cast<void*>((reinterpret_cast<size_t>(original_ptr) + sizeof(void*) + 63) & -64); // round up to 64 byte boundary (cache line alignment)
//
//		*reinterpret_cast<void**>(reinterpret_cast<size_t>(ptr) - sizeof(void*)) = original_ptr;
//		stacks.emplace_back(ptr);
//
//		allocated = 0;
//	}
//
//	void clear()
//	{
//		for (auto &stack : stacks)
//		{
//			::operator delete(*reinterpret_cast<void**>(reinterpret_cast<size_t>(stack) - sizeof(void*)));
//		}
//		stacks.clear();
//	}
//
//public:
//	StackAllocator()
//	{
//		alloc();
//	}
//
//	~StackAllocator()
//	{
//		clear();
//	}
//
//	template<class T, class... _Types>
//	T* make(_Types&&... _Args)
//	{
//		void* ptr = allocate(sizeof(T));
//		::new (ptr) T(std::forward<_Types>(_Args)...);
//		return reinterpret_cast<T*>(ptr);
//	}
//
//	template<class U>
//	void destroy(U *_Ptr)
//	{
//		_Ptr->~U();
//	}
//
//	void* allocate(size_t size)
//	{
//		size = (size + 7) & -8; // round up to 8 byte boundary
//		if (size <= Max_Stack_Table_Size - allocated)
//		{
//			void* ret = (void*)((size_t)stacks.back() + allocated);
//			allocated += size;
//			return ret;
//		}
//
//		alloc();
//
//		allocated += size;
//		return stacks.back();
//	}
//
//	void deallocate(void* ptr)
//	{
//	}
//
//	void clean()
//	{
//		for (size_t i = 1; i < stacks.size(); i++)
//		{
//			::operator delete(*reinterpret_cast<void**>(reinterpret_cast<size_t>(stacks[i]) - sizeof(void*)));
//		}
//
//		stacks.erase(stacks.begin() + 1, stacks.end());
//		allocated = 0;
//	}
//
//protected:
//	StackAllocator(const StackAllocator&) = delete;
//	StackAllocator& operator=(const StackAllocator&) = delete;
//};
//
//extern StackAllocator SappAllocator;
//
//template<class T>
//class tmp_allocator
//{
//public:
//	typedef T value_type;
//
//	typedef value_type *pointer;
//	typedef const value_type *const_pointer;
//
//	typedef value_type& reference;
//	typedef const value_type& const_reference;
//
//	typedef size_t size_type;
//	typedef ptrdiff_t difference_type;
//
//	typedef std::true_type propagate_on_container_move_assignment;
//
//	template<class _Other>
//	struct rebind
//	{
//		typedef tmp_allocator<_Other> other;
//	};
//
//	tmp_allocator() noexcept
//	{
//	}
//
//	tmp_allocator(const tmp_allocator<T>&) noexcept
//	{
//	}
//
//	template<class _Other>
//	tmp_allocator(const tmp_allocator<_Other>&) noexcept
//	{
//	}
//
//	template<class _Other>
//	tmp_allocator<T>& operator=(const tmp_allocator<_Other>&)
//	{
//		return *this;
//	}
//
//	pointer allocate(size_type _Count, const void * = 0)
//	{
//		return reinterpret_cast<pointer>(SappAllocator.allocate(_Count * sizeof(T)));
//		//return reinterpret_cast<pointer>(::operator new(_Count * sizeof(T)));
//	}
//
//	void deallocate(pointer _Ptr, size_type _Count)
//	{
//		SappAllocator.deallocate((void*)_Ptr);
//		//::operator delete(_Ptr);
//	}
//
//	template<class _Objty, class... _Types>
//	void construct(_Objty *_Ptr, _Types&&... _Args)
//	{
//		::new ((void *)_Ptr) _Objty(std::forward<_Types>(_Args)...);
//	}
//
//	template<class U>
//	void destroy(U *_Ptr)
//	{
//		_Ptr->~U();
//	}
//
//	size_t max_size() const noexcept
//	{
//		return (Max_Stack_Table_Size / sizeof(T));
//	}
//
//	pointer address(reference _Val) const noexcept
//	{
//		return &_Val;
//	}
//
//	const_pointer address(const_reference _Val) const noexcept
//	{
//		return &_Val;
//	}
//};
//
//template<class T, class _Other>
//bool operator==(const tmp_allocator<T>&, const tmp_allocator<_Other>&) noexcept
//{
//	return true;
//}
//
//template<class T, class _Other>
//bool operator!=(const tmp_allocator<T>&, const tmp_allocator<_Other>&) noexcept
//{
//	return false;
//}
//
//typedef std::basic_string<char, std::char_traits<char>, tmp_allocator<char>> string;
//typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, tmp_allocator<wchar_t>> wstring;
//typedef std::basic_ostringstream<wchar_t, std::char_traits<wchar_t>, tmp_allocator<wchar_t>> wostringstream;
//
//const std::string& TmpStrToStr(const string &str);
//const string& StrToTmpStr(const std::string &str);
//const std::wstring& TmpWStrToWStr(const wstring &str);
//const wstring& WStrToTmpWStr(const std::wstring &str);