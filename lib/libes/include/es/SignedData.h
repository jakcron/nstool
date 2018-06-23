#pragma once
#include <string>
#include <fnd/MemoryBlob.h>
#include <fnd/ISerialiseableBinary.h>
#include <es/SignatureBlock.h>

namespace es
{
	template <class T>
	class SignedData
		: public fnd::ISerialiseableBinary
	{
	public:
		SignedData()
		{
			clear();
		}
		SignedData(const SignedData& other)
		{
			copyFrom(other);
		}

		void operator=(const SignedData& other)
		{
			copyFrom(other);
		}
		bool operator==(const SignedData& other) const
		{
			return isEqual(other);
		}
		bool operator!=(const SignedData& other) const
		{
			return !(*this == other);
		}

		void importBinary(const byte_t* src, size_t size)
		{
			mSignature.importBinary(src, size);
			mBody.importBinary(src + mSignature.getSize(), size - mSignature.getSize());

			mBinaryBlob.alloc(mSignature.getSize() + mBody.getSize());
			memcpy(mBinaryBlob.getBytes(), src, mBinaryBlob.getSize());
		}

		void exportBinary()
		{
			mSignature.exportBinary();
			mBody.exportBinary();

			mBinaryBlob.alloc(mSignature.getSize() + mBody.getSize());

			memcpy(mBinaryBlob.getBytes(), mSignature.getBytes(), mSignature.getSize());
			memcpy(mBinaryBlob.getBytes() + mSignature.getSize(), mBody.getBytes(), mBody.getSize());
		}

		const byte_t* getBytes() const
		{
			return mBinaryBlob.getBytes();
		}
		size_t getSize() const
		{
			return mBinaryBlob.getSize();
		}

		void clear()
		{
			mBinaryBlob.clear();
			mSignature.clear();
			mBody.clear();
		}

		const es::SignatureBlock& getSignature() const
		{
			return mSignature;
		}
		void setSignature(const SignatureBlock& signature)
		{
			mSignature = signature;
		}

		const T& getBody() const
		{
			return mBody;
		}
		void setBody(const T& body)
		{
			mBody = body;
		}
	private:
		const std::string kModuleName = "SIGNED_DATA";

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		SignatureBlock mSignature;
		T mBody;

		// helpers
		bool isEqual(const SignedData& other) const
		{
			return (mSignature == other.mSignature) \
				&& (mBody == other.mBody);
		}
		void copyFrom(const SignedData& other)
		{
			mBinaryBlob = other.mBinaryBlob;
			mSignature = other.mSignature;
			mBody = other.mBody;
		}
	};
}

