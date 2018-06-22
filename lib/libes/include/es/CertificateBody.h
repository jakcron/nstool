#pragma once
#include <string>
#include <fnd/MemoryBlob.h>
#include <fnd/ISerialiseableBinary.h>
#include <es/cert.h>

namespace es
{
	class CertificateBody
		: public fnd::ISerialiseableBinary
	{
	public:
		CertificateBody();
		CertificateBody(const CertificateBody& other);

		void operator=(const CertificateBody& other);
		bool operator==(const CertificateBody& other) const;
		bool operator!=(const CertificateBody& other) const;

		void importBinary(byte_t* src, size_t size);
		void exportBinary();

		const byte_t* getBytes() const;
		size_t getSize() const;

		void clear();

		const std::string& getIssuer() const;
		void setIssuer(const std::string& issuer);

		es::cert::PublicKeyType getPublicKeyType() const;
		void setPublicKeyType(cert::PublicKeyType type);

		const std::string& getSubject() const;
		void setSubject(const std::string& subject);

		uint32_t getCertId() const;
		void setCertId(uint32_t id);

		const crypto::rsa::sRsa4096Key& getRsa4098PublicKey() const;
		void setRsa4098PublicKey(const crypto::rsa::sRsa4096Key& key);

		const crypto::rsa::sRsa2048Key& getRsa2048PublicKey() const;
		void setRsa2048PublicKey(const crypto::rsa::sRsa2048Key& key);

		const crypto::ecdsa::sEcdsa240Point& getEcdsa240PublicKey() const;
		void setEcdsa240PublicKey(const crypto::ecdsa::sEcdsa240Point& key);
		
	private:
		const std::string kModuleName = "CERTIFICATE_BODY";

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		std::string mIssuer;
		std::string mSubject;
		uint32_t mCertId;
		cert::PublicKeyType mPublicKeyType;

		crypto::rsa::sRsa4096Key mRsa4096PublicKey;
		crypto::rsa::sRsa2048Key mRsa2048PublicKey;
		crypto::ecdsa::sEcdsa240Point mEcdsa240PublicKey;

		// helpers
		bool isEqual(const CertificateBody& other) const;
		void copyFrom(const CertificateBody& other);
	};
}