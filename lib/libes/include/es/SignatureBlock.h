#pragma once
#include <string>
#include <fnd/MemoryBlob.h>
#include <fnd/ISerialiseableBinary.h>
#include <es/sign.h>

namespace es
{
	class SignatureBlock
		: public fnd::ISerialiseableBinary
	{
	public:
		SignatureBlock();
		SignatureBlock(const SignatureBlock& other);

		void operator=(const SignatureBlock& other);
		bool operator==(const SignatureBlock& other) const;
		bool operator!=(const SignatureBlock& other) const;

		void importBinary(byte_t* src, size_t size);
		void exportBinary();

		const byte_t* getBytes() const;
		size_t getSize() const;

		void clear();

		es::sign::SignType getSignType() const;
		void setSignType(es::sign::SignType type);

		bool isLittleEndian() const;
		void setLittleEndian(bool isLE);

		const fnd::MemoryBlob& getSignature() const;
		void setSignature(const fnd::MemoryBlob& signature);
	
		
	private:
		const std::string kModuleName = "SIGNATURE_BLOCK";

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		es::sign::SignType mSignType;
		bool mIsLittleEndian;
		fnd::MemoryBlob mSignature;

		// helpers
		bool isEqual(const SignatureBlock& other) const;
		void copyFrom(const SignatureBlock& other);
	};
}