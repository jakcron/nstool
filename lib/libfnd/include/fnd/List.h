#pragma once
#include <fnd/types.h>
#include <vector>

namespace fnd
{
	template <class T>
	class List
	{
	public:
		List() :
			mElements()
		{
		}

		List(const T* elements, size_t num) :
			mElements(num)
		{
			initList(elements, num);
		}

		// assignment operator
		const List& operator=(const List& other)
		{
			mElements.clear();
			for (size_t i = 0; i < other.getSize(); i++)
			{
				mElements.push_back(other[i]);
			}

			return *this;
		}

		// comparision operator
		bool operator==(const List& other) const
		{
			if (other.getSize() != this->getSize())
			{
				return false;
			}

			for (size_t i = 0; i < this->getSize(); i++)
			{
				if (getElement(i) != other[i])
				{
					return false;
				}
			}

			return true;
		}

		bool operator!=(const List& other) const
		{
			return !operator==(other);
		}

		// access operators
		const T& getElement(size_t index) const
		{
			return mElements[index];
		}
		T& getElement(size_t index)
		{
			if (index == mElements.size()) mElements.push_back(T());
			return mElements[index];
		}

		const T& operator[](size_t index) const { return getElement(index); }
		T& operator[](size_t index) { return getElement(index); }
		const T& atBack() const { return getElement(getSize() - 1); }
		T& atBack() { return getElement(getSize() - 1); }

		// functions
		void addElement(const T& element) { mElements.push_back(element); }
		size_t getIndexOf(const T& key) const
		{
			for (size_t i = 0; i < getSize(); i++)
			{
				if (getElement(i) == key) return i;
			}

			throw Exception("LIST", "Element does not exist");
		}
		bool hasElement(const T& key) const
		{
			try
			{
				getIndexOf(key);
			} catch (const Exception&)
			{
				return false;
			}

			return true;
		}

		// special
		template <class X>
		size_t getIndexOf(const X& key) const
		{
			for (size_t i = 0; i < getSize(); i++)
			{
				if (getElement(i) == key) return i;
			}

			throw Exception("LIST", "Element does not exist");
		}
		template <class X>
		bool hasElement(const X& key) const
		{
			try
			{
				getIndexOf(key);
			} catch (const Exception&)
			{
				return false;
			}

			return true;
		}
		size_t getSize() const { return mElements.size(); }
		void clear() { mElements.clear(); }
	private:
		std::vector<T> mElements;

		void initList(T* elements, size_t num)
		{
			mElements.clear();
			for (size_t i = 0; i < num; i++)
			{
				mElements.push_back(elements[i]);
			}
		}
	};
}

