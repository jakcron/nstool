#pragma once
#include <string>
#include <fnd/ISerialisable.h>
#include <nn/pki/sign.h>

namespace nn
{
namespace pki
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
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		pki::sign::SignatureId getSignType() const;
		void setSignType(pki::sign::SignatureId type);

		bool isLittleEndian() const;
		void setLittleEndian(bool isLE);

		const fnd::Vec<byte_t>& getSignature() const;
		void setSignature(const fnd::Vec<byte_t>& signature);
	
		
	private:
		const std::string kModuleName = "SIGNATURE_BLOCK";

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		pki::sign::SignatureId mSignType;
		bool mIsLittleEndian;
		fnd::Vec<byte_t> mSignature;
	};
}
}