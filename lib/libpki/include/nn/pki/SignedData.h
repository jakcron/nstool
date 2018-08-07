#pragma once
#include <string>
#include <fnd/ISerialisable.h>
#include <nn/pki/SignatureBlock.h>

namespace nn
{
namespace pki
{
	template <class T>
	class SignedData
		: public fnd::ISerialisable
	{
	public:
		SignedData();
		SignedData(const SignedData& other);

		void operator=(const SignedData& other);
		bool operator==(const SignedData& other) const;
		bool operator!=(const SignedData& other) const;

		// export/import
		void toBytes();
		void fromBytes(const byte_t* src, size_t size);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		const pki::SignatureBlock& getSignature() const;
		void setSignature(const SignatureBlock& signature);

		const T& getBody() const;
		void setBody(const T& body);
	private:
		const std::string kModuleName = "SIGNED_DATA";

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		SignatureBlock mSignature;
		T mBody;
	};

	template <class T>
	inline SignedData<T>::SignedData()
	{
		clear();
	}

	template <class T>
	inline SignedData<T>::SignedData(const SignedData& other)
	{
		*this = other;
	}

	template <class T>
	inline void SignedData<T>::operator=(const SignedData& other)
	{
		mRawBinary = other.mRawBinary;
		mSignature = other.mSignature;
		mBody = other.mBody;
	}

	template <class T>
	inline bool SignedData<T>::operator==(const SignedData& other) const
	{
		return (mSignature == other.mSignature) \
			&& (mBody == other.mBody);
	}

	template <class T>
	inline bool SignedData<T>::operator!=(const SignedData& other) const
	{
		return !(*this == other);
	}

	template <class T>
	inline void SignedData<T>::toBytes()
	{
		mSignature.toBytes();
		mBody.toBytes();

		mRawBinary.alloc(mSignature.getBytes().size() + mBody.getBytes().size());

		memcpy(mRawBinary.data(), mSignature.getBytes().data(), mSignature.getBytes().size());
		memcpy(mRawBinary.data() + mSignature.getBytes().size(), mBody.getBytes().data(), mBody.getBytes().size());
	}

	template <class T>
	inline void SignedData<T>::fromBytes(const byte_t* src, size_t size)
	{
		mSignature.fromBytes(src, size);
		mBody.fromBytes(src + mSignature.getBytes().size(), size - mSignature.getBytes().size());

		mRawBinary.alloc(mSignature.getBytes().size() + mBody.getBytes().size());
		memcpy(mRawBinary.data(), src, mRawBinary.size());
	}

	template <class T>
	inline const fnd::Vec<byte_t>& SignedData<T>::getBytes() const
	{
		return mRawBinary;
	}

	template <class T>
	inline void SignedData<T>::clear()
	{
		mRawBinary.clear();
		mSignature.clear();
		mBody.clear();
	}

	template <class T>
	inline const pki::SignatureBlock& SignedData<T>::getSignature() const
	{
		return mSignature;
	}

	template <class T>
	inline void SignedData<T>::setSignature(const SignatureBlock& signature)
	{
		mSignature = signature;
	}

	template <class T>
	inline const T& SignedData<T>::getBody() const
	{
		return mBody;
	}

	template <class T>
	inline void SignedData<T>::setBody(const T& body)
	{
		mBody = body;
	}
}
}