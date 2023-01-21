#include "EsCertProcess.h"
#include "PkiValidator.h"
#include "util.h"

#include <pietendo/hac/es/SignUtils.h>

nstool::EsCertProcess::EsCertProcess() :
	mModuleName("nstool::EsCertProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false)
{
}

void nstool::EsCertProcess::process()
{
	importCerts();

	if (mVerify)
		validateCerts();

	if (mCliOutputMode.show_basic_info)
		displayCerts();
}

void nstool::EsCertProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::EsCertProcess::setKeyCfg(const KeyBag& keycfg)
{
	mKeyCfg = keycfg;
}

void nstool::EsCertProcess::setCliOutputMode(CliOutputMode mode)
{
	mCliOutputMode = mode;
}

void nstool::EsCertProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void nstool::EsCertProcess::importCerts()
{
	if (mFile == nullptr)
	{
		throw tc::Exception(mModuleName, "No file reader set.");
	}
	if (mFile->canRead() == false || mFile->canSeek() == false)
	{
		throw tc::NotSupportedException(mModuleName, "Input stream requires read/seek permissions.");
	}

	// check if file_size is greater than 20MB, don't import.
	size_t file_size = tc::io::IOUtil::castInt64ToSize(mFile->length());
	if (file_size > (0x100000 * 20))
	{
		throw tc::Exception(mModuleName, "File too large.");
	}

	// import certs
	tc::ByteData scratch = tc::ByteData(file_size);
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());

	pie::hac::es::SignedData<pie::hac::es::CertificateBody> cert;
	for (size_t f_pos = 0; f_pos < scratch.size(); f_pos += cert.getBytes().size())
	{
		cert.fromBytes(scratch.data() + f_pos, scratch.size() - f_pos);
		mCert.push_back(cert);
	}
}

void nstool::EsCertProcess::validateCerts()
{
	PkiValidator pki;
	
	try
	{
		pki.setKeyCfg(mKeyCfg);
		pki.addCertificates(mCert);
	}
	catch (const tc::Exception& e)
	{
		fmt::print("[WARNING] {}\n", e.error());
		return;
	}
}

void nstool::EsCertProcess::displayCerts()
{
	for (size_t i = 0; i < mCert.size(); i++)
	{
		displayCert(mCert[i]);
	}
}

void nstool::EsCertProcess::displayCert(const pie::hac::es::SignedData<pie::hac::es::CertificateBody>& cert)
{
	fmt::print("[ES Certificate]\n");

	fmt::print("  SignType       {:s}", getSignTypeStr(cert.getSignature().getSignType()));
	if (mCliOutputMode.show_extended_info)
		fmt::print(" (0x{:x}) ({:s})", (uint32_t)cert.getSignature().getSignType(), getEndiannessStr(cert.getSignature().isLittleEndian()));
	fmt::print("\n");

	fmt::print("  Issuer:        {:s}\n", cert.getBody().getIssuer());
	fmt::print("  Subject:       {:s}\n", cert.getBody().getSubject());
	fmt::print("  PublicKeyType: {:s}", getPublicKeyTypeStr(cert.getBody().getPublicKeyType()));
	if (mCliOutputMode.show_extended_info)
		fmt::print(" ({:d})", (uint32_t)cert.getBody().getPublicKeyType());
	fmt::print("\n");
	fmt::print("  CertID:        0x{:x}\n", cert.getBody().getCertId());
	
	if (cert.getBody().getPublicKeyType() == pie::hac::es::cert::RSA4096)
	{
		fmt::print("  PublicKey:\n");
		if (mCliOutputMode.show_extended_info)
		{
			fmt::print("    Modulus:\n");
			fmt::print("      {:s}", tc::cli::FormatUtil::formatBytesAsStringWithLineLimit(cert.getBody().getRsa4096PublicKey().n.data(), cert.getBody().getRsa4096PublicKey().n.size(), true, "", 0x10, 6, false));
			fmt::print("    Public Exponent:\n");
			fmt::print("      {:s}", tc::cli::FormatUtil::formatBytesAsStringWithLineLimit(cert.getBody().getRsa4096PublicKey().e.data(), cert.getBody().getRsa4096PublicKey().e.size(), true, "", 0x10, 6, false));
		}
		else
		{
			fmt::print("    Modulus:\n");
			fmt::print("      {:s}\n", getTruncatedBytesString(cert.getBody().getRsa4096PublicKey().n.data(), cert.getBody().getRsa4096PublicKey().n.size()));
			fmt::print("    Public Exponent:\n");
			fmt::print("      {:s}\n", getTruncatedBytesString(cert.getBody().getRsa4096PublicKey().e.data(), cert.getBody().getRsa4096PublicKey().e.size()));
		}
	}
	else if (cert.getBody().getPublicKeyType() == pie::hac::es::cert::RSA2048)
	{
		fmt::print("  PublicKey:\n");
		if (mCliOutputMode.show_extended_info)
		{
			fmt::print("    Modulus:\n");
			fmt::print("      {:s}", tc::cli::FormatUtil::formatBytesAsStringWithLineLimit(cert.getBody().getRsa2048PublicKey().n.data(), cert.getBody().getRsa2048PublicKey().n.size(), true, "", 0x10, 6, false));
			fmt::print("    Public Exponent:\n");
			fmt::print("      {:s}", tc::cli::FormatUtil::formatBytesAsStringWithLineLimit(cert.getBody().getRsa2048PublicKey().e.data(), cert.getBody().getRsa2048PublicKey().e.size(), true, "", 0x10, 6, false));
		}
		else
		{
			fmt::print("    Modulus:\n");
			fmt::print("      {:s}\n", getTruncatedBytesString(cert.getBody().getRsa2048PublicKey().n.data(), cert.getBody().getRsa2048PublicKey().n.size()));
			fmt::print("    Public Exponent:\n");
			fmt::print("      {:s}\n", getTruncatedBytesString(cert.getBody().getRsa2048PublicKey().e.data(), cert.getBody().getRsa2048PublicKey().e.size()));
		}
	}
	else if (cert.getBody().getPublicKeyType() == pie::hac::es::cert::ECDSA240)
	{
		fmt::print("  PublicKey:\n");
		if (mCliOutputMode.show_extended_info)
		{
			fmt::print("    Modulus:\n");
			fmt::print("      {:s}", tc::cli::FormatUtil::formatBytesAsStringWithLineLimit(cert.getBody().getEcdsa240PublicKey().r.data(), cert.getBody().getEcdsa240PublicKey().r.size(), true, "", 0x10, 6, false));
			fmt::print("    Public Exponent:\n");
			fmt::print("      {:s}", tc::cli::FormatUtil::formatBytesAsStringWithLineLimit(cert.getBody().getEcdsa240PublicKey().s.data(), cert.getBody().getEcdsa240PublicKey().s.size(), true, "", 0x10, 6, false));
		}
		else
		{
			fmt::print("    Modulus:\n");
			fmt::print("      {:s}\n", getTruncatedBytesString(cert.getBody().getEcdsa240PublicKey().r.data(), cert.getBody().getEcdsa240PublicKey().r.size()));
			fmt::print("    Public Exponent:\n");
			fmt::print("      {:s}\n", getTruncatedBytesString(cert.getBody().getEcdsa240PublicKey().s.data(), cert.getBody().getEcdsa240PublicKey().s.size()));
		}
	}
}

std::string nstool::EsCertProcess::getSignTypeStr(pie::hac::es::sign::SignatureId type) const
{
	std::string str;
	switch (type)
	{
	case (pie::hac::es::sign::SIGN_ID_RSA4096_SHA1):
		str = "RSA4096-SHA1";
		break;
	case (pie::hac::es::sign::SIGN_ID_RSA2048_SHA1):
		str = "RSA2048-SHA1";
		break;
	case (pie::hac::es::sign::SIGN_ID_ECDSA240_SHA1):
		str = "ECDSA240-SHA1";
		break;
	case (pie::hac::es::sign::SIGN_ID_RSA4096_SHA256):
		str = "RSA4096-SHA256";
		break;
	case (pie::hac::es::sign::SIGN_ID_RSA2048_SHA256):
		str = "RSA2048-SHA256";
		break;
	case (pie::hac::es::sign::SIGN_ID_ECDSA240_SHA256):
		str = "ECDSA240-SHA256";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}

std::string nstool::EsCertProcess::getEndiannessStr(bool isLittleEndian) const
{
	return isLittleEndian ? "LittleEndian" : "BigEndian";
}

std::string nstool::EsCertProcess::getPublicKeyTypeStr(pie::hac::es::cert::PublicKeyType type) const
{
	std::string str;
	switch (type)
	{
	case (pie::hac::es::cert::RSA4096):
		str = "RSA4096";
		break;
	case (pie::hac::es::cert::RSA2048):
		str = "RSA2048";
		break;
	case (pie::hac::es::cert::ECDSA240):
		str = "ECDSA240";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}