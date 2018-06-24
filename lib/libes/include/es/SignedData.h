#pragma once
#include <string>
#include <fnd/ISerialisable.h>
#include <es/SignatureBlock.h>

namespace es
{
	template <class T>
	class SignedData
		: public fnd::ISerialiseable
	{
	public:
		SignedData();
		SignedData(const SignedData& other);

		void operator=(const SignedData& other);
		bool operator==(const SignedData& other) const;
		bool operator!=(const SignedData& other) const;

		// export/import
		const void toBytes();
		void fromBytes(const byte_t* src, size_t size);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		const es::SignatureBlock& getSignature() const;
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
	inline SignedData::SignedData()
	{
		clear();
	}

	template <class T>
	inline SignedData::SignedData(const SignedData& other)
	{
		*this = other;
	}

	template <class T>
	inline void SignedData::operator=(const SignedData& other)
	{
		mRawBinary = other.mRawBinary;
		mSignature = other.mSignature;
		mBody = other.mBody;
	}

	template <class T>
	inline bool SignedData::operator==(const SignedData& other) const
	{
		return (mSignature == other.mSignature) \
			&& (mBody == other.mBody);
	}

	template <class T>
	inline bool SignedData::operator!=(const SignedData& other) const
	{
		return !(*this == other);
	}

	template <class T>
	inline const void SignedData::toBytes()
	{
		mSignature.toBytes();
		mBody.toBytes();

		mRawBinary.alloc(mSignature.getBytes().size() + mBody.getBytes().size());

		memcpy(mRawBinary.getBytes().data(), mSignature.getBytes().data(), mSignature.getBytes().size());
		memcpy(mRawBinary.getBytes().data() + mSignature.getBytes().size(), mBody.getBytes().data(), mBody.getBytes().size());
	}

	template <class T>
	inline void SignedData::fromBytes(const byte_t* src, size_t size)
	{
		mSignature.fromBytes(src, size);
		mBody.fromBytes(src + mSignature.getBytes().size(), size - mSignature.getBytes().size());

		mRawBinary.alloc(mSignature.getBytes().size() + mBody.getBytes().size());
		memcpy(mRawBinary.getBytes().data(), src, mRawBinary.getBytes().size());
	}

	template <class T>
	inline const fnd::Vec<byte_t>& SignedData::getBytes() const
	{
		return mRawBinary;
	}

	template <class T>
	inline void SignedData::clear()
	{
		mRawBinary.clear();
		mSignature.clear();
		mBody.clear();
	}

	template <class T>
	inline const es::SignatureBlock& SignedData::getSignature() const
	{
		return mSignature;
	}

	template <class T>
	inline void SignedData::setSignature(const SignatureBlock& signature)
	{
		mSignature = signature;
	}

	template <class T>
	inline const T& SignedData::getBody() const
	{
		return mBody;
	}

	template <class T>
	inline void SignedData::setBody(const T& body)
	{
		mBody = body;
	}
}

