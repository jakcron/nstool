#pragma once
#include <string>
#include <nx/AciBinary.h>
#include <crypto/rsa.h>

namespace nx
{
	class AcidBinary :
		public AciBinary
	{
	public:
		AcidBinary();
		AcidBinary(const AcidBinary& other);

		void operator=(const AcidBinary& other);
		bool operator==(const AcidBinary& other) const;
		bool operator!=(const AcidBinary& other) const;

		// export/import binary
		void toBytes();
		void signBinary(const crypto::rsa::sRsa2048Key& key);
		void fromBytes(const byte_t* bytes, size_t len);
		void verifyBinary(const crypto::rsa::sRsa2048Key& key) const;
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		virtual void clear();

		const crypto::rsa::sRsa2048Key& getNcaHeader2RsaKey() const;
		void setNcaHeader2RsaKey(const crypto::rsa::sRsa2048Key& key);

	private:
		const std::string kModuleName = "ACID_BINARY";

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		crypto::rsa::sRsa2048Key mEmbeddedPublicKey;
	};
}

