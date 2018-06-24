#pragma once
#include <fnd/types.h>
#include <vector>
//#include <fnd/Vec.h>

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
		void operator=(const List<T>& other);

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
		std::vector<T> m_Vec;
	};	

	template<class T>
	inline List<T>::List() :
		m_Vec()
	{
	}

	template<class T>
	inline List<T>::List(const List<T>& other) :
		List()
	{
		*this = other;
	}

	template<class T>
	inline void List<T>::operator=(const List<T>& other)
	{
		m_Vec = other.m_Vec;
	}

	template<class T>
	inline bool List<T>::operator==(const List<T>& other) const
	{
		return m_Vec == other.m_Vec;
	}

	template<class T>
	inline bool List<T>::operator!=(const List<T>& other) const
	{
		return !(*this == other);
	}

	template<class T>
	inline void List<T>::addElement(const T & element)
	{
		m_Vec.push_back(element);
	}

	template<class T>
	inline const T & List<T>::operator[](size_t index) const
	{
		return m_Vec[index];
	}

	template<class T>
	inline T & List<T>::operator[](size_t index)
	{
		return m_Vec[index];
	}

	template<class T>
	inline const T & List<T>::atBack() const
	{
		return m_Vec.back();
	}

	template<class T>
	inline T & List<T>::atBack()
	{
		return m_Vec.back();
	}

	template<class T>
	inline size_t List<T>::size() const
	{
		return m_Vec.size();
	}

	template<class T>
	inline void List<T>::clear()
	{
		m_Vec.clear();
	}

	template<class T>
	template<class K>
	inline bool List<T>::hasElement(const K & key) const
	{
		for (size_t i = 0; i < m_Vec.size(); i++)
		{
			if (m_Vec[i] == key)
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
		for (size_t i = 0; i < m_Vec.size(); i++)
		{
			if (m_Vec[i] == key)
			{
				return m_Vec[i];
			}
		}

		throw fnd::Exception("getElement(): element does not exist");
	}

	template<class T>
	template<class K>
	inline T & List<T>::getElement(const K & key)
	{
		for (size_t i = 0; i < m_Vec.size(); i++)
		{
			if (m_Vec[i] == key)
			{
				return m_Vec[i];
			}
		}

		throw fnd::Exception("getElement(): element does not exist");
	}
}
