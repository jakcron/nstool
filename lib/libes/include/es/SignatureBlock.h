#pragma once
#include <string>
#include <fnd/ISerialisable.h>
#include <es/sign.h>

namespace es
{
	class SignatureBlock
		: public fnd::ISerialisable
	{
	public:
		SignatureBlock();
		SignatureBlock(const SignatureBlock& other);

		void operator=(const SignatureBlock& other);
		bool operator==(const SignatureBlock& other) const;
		bool operator!=(const SignatureBlock& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* src, size_t size);
		const const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		es::sign::SignType getSignType() const;
		void setSignType(es::sign::SignType type);

		bool isLittleEndian() const;
		void setLittleEndian(bool isLE);

		const fnd::Vec<byte_t>& getSignature() const;
		void setSignature(const fnd::Vec<byte_t>& signature);
	
		
	private:
		const std::string kModuleName = "SIGNATURE_BLOCK";

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		es::sign::SignType mSignType;
		bool mIsLittleEndian;
		fnd::Vec<byte_t> mSignature;
	};
}