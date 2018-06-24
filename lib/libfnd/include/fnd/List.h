#pragma once
#include <fnd/types.h>
#include <fnd/Vec.h>

namespace fnd
{
	template <class T>
	class List
	{
	public:
		// constructors
		List();
		List(const List<T>& other);

		// copy operator
		const List<T>& operator=(const List<T>& other);

		// equivalence operators
		bool operator==(const List<T>& other) const;
		bool operator!=(const List<T>& other) const;

		// back relative insertion
		void addElement(const T& element);
		
		// element access
		const T& operator[](size_t index) const;
		T& operator[](size_t index);
		const T& atBack() const;
		T& atBack();

		// element num
		size_t size() const;

		// clear List
		void clear();

		// element access by key
		template <class K>
		bool hasElement(const K& key) const;
		template <class K>
		const T& getElement(const K& key) const;
		template <class K>
		T& getElement(const K& key);
		
	private:
		static const size_t kDefaultSize = 20;

		fnd::Vec<T> m_Vec;
		size_t m_Num;
	};	

	template<class T>
	inline List<T>::List() :
		m_Vec(),
		m_Num(0)
	{
		m_Vec.alloc(kDefaultSize);
	}

	template<class T>
	inline List<T>::List(const List<T>& other) :
		m_Vec(other.m_Vec),
		m_Size(other.m_Size)
	{}

	template<class T>
	inline const List<T>& List<T>::operator=(const List<T>& other)
	{
		m_Vec = other.m_Vec;
		m_Size = other.m_Size;
	}

	template<class T>
	inline bool List<T>::operator==(const List<T>& other) const
	{
		bool isEqual = true;

		if (m_Num == other.m_Num)
		{
			for (size_t i = 0; i < m_Num && isEqual == true; i++)
			{
				if ((*this)[i] != other[i])
					isEqual = false;
			}
		}
		else
		{
			isEqual = false;
		}

		return isEqual;
	}

	template<class T>
	inline bool List<T>::operator!=(const List<T>& other) const
	{
		return !(*this == other);
	}

	template<class T>
	inline void List<T>::addElement(const T & element)
	{
		(*this)[m_Num] = element;
	}

	template<class T>
	inline const T & List<T>::operator[](size_t index) const
	{
		if (index >= m_Num)
		{
			throw fnd::Exception("List", "Out of bound read");
		}

		return m_Vec[index];
	}

	template<class T>
	inline T & List<T>::operator[](size_t index)
	{
		if (index > m_Num)
		{
			throw fnd::Exception("List", "Out of bound read");
		}
		else if (index == m_Num)
		{
			if ((m_Num * 2) >= m_Vec.size())
			{
				m_Vec.alloc((m_Num + 1) * 2);
			}

			m_Num++;
		}

		return m_Vec[index];
	}

	template<class T>
	inline const T & List<T>::atBack() const
	{
		return m_Vec[m_Num - 1];
	}

	template<class T>
	inline T & List<T>::atBack()
	{
		return m_Vec[m_Num - 1];
	}

	template<class T>
	inline size_t List<T>::size() const
	{
		return m_Num;
	}

	template<class T>
	inline void List<T>::clear()
	{
		m_Num = 0;
		m_Vec.clear();
	}

	template<class T>
	template<class K>
	inline bool List<T>::hasElement(const K & key) const
	{
		for (size_t i = 0; i < m_Num; i++)
		{
			if (m_List[i] == key)
			{
				return true;
			}
		}

		return false;
	}

	template<class T>
	template<class K>
	inline const T & List<T>::getElement(const K & key) const
	{
		for (size_t i = 0; i < m_Num; i++)
		{
			if (m_List[i] == key)
			{
				return m_List[i];
			}
		}

		throw fnd::Exception("getElement(): element does not exist");
	}

	template<class T>
	template<class K>
	inline T & List<T>::getElement(const K & key)
	{
		for (size_t i = 0; i < m_Num; i++)
		{
			if (m_List[i] == key)
			{
				return m_List[i];
			}
		}

		throw fnd::Exception("getElement(): element does not exist");
	}
}
