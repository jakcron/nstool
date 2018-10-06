#pragma once
#include <fnd/types.h>
#include <cstdio>

namespace fnd
{
	template <class T>
	class SharedPtr
	{
	public:
		SharedPtr();

		// constructor for creating owner object
		SharedPtr(T* ptr);

		// copy constructor
		SharedPtr(const SharedPtr<T>& other);

		// destructor
		~SharedPtr();

		// own operator
		void operator=(T* ptr);

		// copy operator
		void operator=(const SharedPtr<T>& other);

		// access ptr
		const T* operator*() const;
		T* operator*();

	private:
		T* mPtr;
		size_t* mRefCnt;

		void deletePtr();
	};

	template <class T>
	inline SharedPtr<T>::SharedPtr() :
		mPtr(nullptr),
		mRefCnt(new size_t)
	{
		*mRefCnt = 0;
	}

	template <class T>
	inline SharedPtr<T>::SharedPtr(T* ptr) :
		SharedPtr()
	{
		*this = ptr;
	}

	template <class T>
	inline SharedPtr<T>::SharedPtr(const SharedPtr<T>& other) :
		SharedPtr()
	{
		*this = other;
	}

	template <class T>
	inline SharedPtr<T>::~SharedPtr()
	{
		deletePtr();
	}

	template <class T>
	inline void SharedPtr<T>::operator=(T* ptr)
	{
		deletePtr();
		if (ptr != nullptr)
		{
			mPtr = ptr;
			mRefCnt = new size_t;
			*mRefCnt = 1;
		}
		else
		{
			mPtr = nullptr;
			mRefCnt = new size_t;
			*mRefCnt = 0;
		}
		
	}

	template <class T>
	inline void SharedPtr<T>::operator=(const SharedPtr<T>& other)
	{
		deletePtr();

		mPtr = other.mPtr;
		mRefCnt = other.mRefCnt;
		*mRefCnt += 1;
	}

	template <class T>
	inline const T* SharedPtr<T>::operator*() const
	{
		return mPtr;
	}

	template <class T>
	inline T* SharedPtr<T>::operator*()
	{
		return mPtr;
	}

	template <class T>
	inline void SharedPtr<T>::deletePtr()
	{
		// if this is not the last reference
		if (*mRefCnt > 1)
		{
			// decrement reference count
			*mRefCnt -= 1;

			// make ptrs null
			mPtr = nullptr;
			mRefCnt = nullptr;
		}
		// if this is the last refeference
		else if (*mRefCnt == 1)
		{
			// delete memory
			delete mPtr;
			delete mRefCnt;

			// make ptrs null
			mPtr = nullptr;
			mRefCnt = nullptr;
		}
		// else if this is an empty refernce
		else if (*mRefCnt == 0)
		{
			delete mRefCnt;

			mPtr = nullptr;
			mRefCnt = nullptr;
		}
	}
}