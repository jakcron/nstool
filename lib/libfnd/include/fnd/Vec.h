#pragma once
#include <fnd/types.h>

namespace fnd
{
	template <class T>
	class Vec
	{
	public:
		// constructors
		Vec();
		Vec(const Vec<T>& other);
		Vec(const T* array, size_t num);
		~Vec();

		// copy operator
		void operator=(const Vec<T>& other);

		// equivalence operators
		bool operator==(const Vec<T>& other) const;
		bool operator!=(const Vec<T>& other) const;

		// element access
		const T& operator[](size_t index) const;
		T& operator[](size_t index);

		// raw access 
		const T* data() const;
		T* data();

		// element num
		size_t size() const;

		// allocate vector
		void alloc(size_t new_size);

		// resize vector
		void resize(size_t new_size);

		// clear vector
		void clear();
	private:
		T* m_Vec;
		size_t m_Size;

		void copyFrom(const T * array, size_t num);
	};

	template<class T>
	inline Vec<T>::Vec() :
		m_Vec(nullptr),
		m_Size(0)
	{}

	template<class T>
	inline Vec<T>::Vec(const Vec<T>& other) :
		Vec()
	{
		copyFrom(other.data(), other.size());
	}

	template<class T>
	inline Vec<T>::Vec(const T * array, size_t num) :
		Vec()
	{
		copyFrom(array, num);
	}

	template<class T>
	inline Vec<T>::~Vec()
	{
		clear();
	}

	template<class T>
	inline void Vec<T>::operator=(const Vec<T>& other)
	{
		copyFrom(other.data(), other.size());
	}

	template<class T>
	inline bool Vec<T>::operator==(const Vec<T>& other) const
	{
		bool isEqual = true;

		if (m_Size == other.m_Size)
		{
			for (size_t i = 0; i < m_Size && isEqual; i++)
			{
				if (m_Vec[i] != other.m_Vec[i])
				{
					isEqual = false;
				}
			}
		}
		else
		{
			isEqual = false;
		}

		return isEqual;
	}

	template<class T>
	inline bool Vec<T>::operator!=(const Vec<T>& other) const
	{
		return !(*this == other);
	}

	template<class T>
	inline const T & Vec<T>::operator[](size_t index) const
	{
		return m_Vec[index];
	}

	template<class T>
	inline T & Vec<T>::operator[](size_t index)
	{
		return m_Vec[index];
	}

	template<class T>
	inline const T * Vec<T>::data() const
	{
		return m_Vec;
	}

	template<class T>
	inline T * Vec<T>::data()
	{
		return m_Vec;
	}

	template<class T>
	inline size_t Vec<T>::size() const
	{
		return m_Size;
	}

	template<class T>
	inline void Vec<T>::alloc(size_t new_size)
	{
		clear();
		m_Vec = new T[new_size];
		if (m_Vec == nullptr)
		{
			fnd::Exception("Vec", "Failed to allocate memory for vector");
		}
		for (size_t i = 0; i < new_size; i++)
		{
			m_Vec[i] = 0;
		}

		m_Size = new_size;
	}

	template<class T>
	inline void Vec<T>::resize(size_t new_size)
	{
		if (m_Vec != nullptr)
		{
			T* new_vec = new T[new_size];
			if (new_vec == nullptr)
			{
				fnd::Exception("Vec", "Failed to allocate memory for vector");
			}
			for (size_t i = 0; i < new_size; i++)
			{
				new_vec[i] = 0;
			}

			for (size_t i = 0; i < _MIN(m_Size, new_size); i++)
			{
				new_vec[i] = m_Vec[i];
			}
			delete[] m_Vec;
			m_Vec = new_vec;
			m_Size = new_size;
		}
		else
		{
			alloc(new_size);
		}
	}

	template<class T>
	inline void Vec<T>::clear()
	{
		if (m_Vec != nullptr)
		{
			delete[] m_Vec;
		}
		m_Vec = nullptr;
		m_Size = 0;
	}

	template<class T>
	inline void Vec<T>::copyFrom(const T * array, size_t num)
	{
		clear();
		alloc(num);
		for (size_t i = 0; i < m_Size; i++)
		{
			m_Vec[i] = array[i];
		}
	}
}