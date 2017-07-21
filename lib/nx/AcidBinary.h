#pragma once
#include <string>
#include <fnd/memory_blob.h>
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
		AcidBinary(const u8* bytes, size_t len);

		bool operator==(const AcidBinary& other) const;
		bool operator!=(const AcidBinary& other) const;
		void operator=(const AcidBinary& other);

		// to be used after export
		const u8* getBytes() const;
		size_t getSize() const;

		// export/import binary
		virtual void exportBinary();
		void signBinary(const crypto::rsa::sRsa2048Key& key);
		virtual void importBinary(const u8* bytes, size_t len);
		void verifyBinary(const crypto::rsa::sRsa2048Key& key);

		// variables
		virtual void clear();

		const crypto::rsa::sRsa2048Key& getNcaHeader2RsaKey() const;
		void setNcaHeader2RsaKey(const crypto::rsa::sRsa2048Key& key);

	private:
		const std::string kModuleName = "ACID_BINARY";

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		crypto::rsa::sRsa2048Key mEmbeddedPublicKey;

		bool isEqual(const AcidBinary& other) const;
		void copyFrom(const AcidBinary& other);
	};
}

