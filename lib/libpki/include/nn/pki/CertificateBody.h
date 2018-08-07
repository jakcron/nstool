#pragma once
#include <string>
#include <fnd/ISerialisable.h>
#include <nn/pki/cert.h>

namespace nn
{
namespace pki
{
	class CertificateBody
		: public fnd::ISerialisable
	{
	public:
		CertificateBody();
		CertificateBody(const CertificateBody& other);

		void operator=(const CertificateBody& other);
		bool operator==(const CertificateBody& other) const;
		bool operator!=(const CertificateBody& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* src, size_t size);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		const std::string& getIssuer() const;
		void setIssuer(const std::string& issuer);

		pki::cert::PublicKeyType getPublicKeyType() const;
		void setPublicKeyType(cert::PublicKeyType type);

		const std::string& getSubject() const;
		void setSubject(const std::string& subject);

		uint32_t getCertId() const;
		void setCertId(uint32_t id);

		const fnd::rsa::sRsa4096Key& getRsa4098PublicKey() const;
		void setRsa4098PublicKey(const fnd::rsa::sRsa4096Key& key);

		const fnd::rsa::sRsa2048Key& getRsa2048PublicKey() const;
		void setRsa2048PublicKey(const fnd::rsa::sRsa2048Key& key);

		const fnd::ecdsa::sEcdsa240Point& getEcdsa240PublicKey() const;
		void setEcdsa240PublicKey(const fnd::ecdsa::sEcdsa240Point& key);
		
	private:
		const std::string kModuleName = "CERTIFICATE_BODY";

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		std::string mIssuer;
		std::string mSubject;
		uint32_t mCertId;
		cert::PublicKeyType mPublicKeyType;

		fnd::rsa::sRsa4096Key mRsa4096PublicKey;
		fnd::rsa::sRsa2048Key mRsa2048PublicKey;
		fnd::ecdsa::sEcdsa240Point mEcdsa240PublicKey;
	};
}
}