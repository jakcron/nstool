#include "EsTikProcess.h"
#include "PkiValidator.h"

#include <pietendo/hac/es/SignUtils.h>

nstool::EsTikProcess::EsTikProcess() :
	mModuleName("nstool::EsTikProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false)
{
}

void nstool::EsTikProcess::process()
{
	importTicket();

	if (mVerify)
		verifyTicket();

	if (mCliOutputMode.show_basic_info)
		displayTicket();
}

void nstool::EsTikProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::EsTikProcess::setKeyCfg(const KeyBag& keycfg)
{
	mKeyCfg = keycfg;
}

void nstool::EsTikProcess::setCertificateChain(const std::vector<pie::hac::es::SignedData<pie::hac::es::CertificateBody>>& certs)
{
	mCerts = certs;
}

void nstool::EsTikProcess::setCliOutputMode(CliOutputMode mode)
{
	mCliOutputMode = mode;
}

void nstool::EsTikProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void nstool::EsTikProcess::importTicket()
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

	// read ticket
	tc::ByteData scratch = tc::ByteData(file_size);
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());

	mTik.fromBytes(scratch.data(), scratch.size());
}

void nstool::EsTikProcess::verifyTicket()
{
	PkiValidator pki_validator;
	tc::ByteData tik_hash;

	switch (pie::hac::es::sign::getHashAlgo(mTik.getSignature().getSignType()))
	{
	case (pie::hac::es::sign::HASH_ALGO_SHA1):
		tik_hash = tc::ByteData(tc::crypto::Sha1Generator::kHashSize);
		tc::crypto::GenerateSha1Hash(tik_hash.data(), mTik.getBody().getBytes().data(), mTik.getBody().getBytes().size());
		break;
	case (pie::hac::es::sign::HASH_ALGO_SHA256):
		tik_hash = tc::ByteData(tc::crypto::Sha2256Generator::kHashSize);
		tc::crypto::GenerateSha2256Hash(tik_hash.data(), mTik.getBody().getBytes().data(), mTik.getBody().getBytes().size());
		break;
	}

	try 
	{
		pki_validator.setKeyCfg(mKeyCfg);
		pki_validator.addCertificates(mCerts);
		pki_validator.validateSignature(mTik.getBody().getIssuer(), mTik.getSignature().getSignType(), mTik.getSignature().getSignature(), tik_hash);
	}
	catch (const tc::Exception& e)
	{
		fmt::print("[WARNING] Ticket signature could not be validated ({:s})\n", e.error());
	}
}

void nstool::EsTikProcess::displayTicket()
{
	const pie::hac::es::TicketBody_V2& body = mTik.getBody();	

	fmt::print("[ES Ticket]\n");
	fmt::print("  SignType:         {:s}", getSignTypeStr(mTik.getSignature().getSignType()));
	if (mCliOutputMode.show_extended_info)
		fmt::print(" (0x{:x})", (uint32_t)mTik.getSignature().getSignType());
	fmt::print("\n");

	fmt::print("  Issuer:           {:s}\n", body.getIssuer());
	fmt::print("  Title Key:\n");
	fmt::print("    EncMode:        {:s}\n", getTitleKeyPersonalisationStr(body.getTitleKeyEncType()));
	fmt::print("    KeyGeneration:  {:d}\n", (uint32_t)body.getCommonKeyId());
	if (body.getTitleKeyEncType() == pie::hac::es::ticket::RSA2048)
	{
		fmt::print("    Data:\n");
		fmt::print("      {:s}", tc::cli::FormatUtil::formatBytesAsStringWithLineLimit(body.getEncTitleKey(), 0x100, true, "", 0x10, 6, false));
	}
	else if (body.getTitleKeyEncType() == pie::hac::es::ticket::AES128_CBC)
	{
		fmt::print("    Data:\n");
		fmt::print("      {:s}\n", tc::cli::FormatUtil::formatBytesAsString(body.getEncTitleKey(), 0x10, true, ""));
	}
	else
	{
		fmt::print("    Data:           <cannot display>\n");
	}
	fmt::print("  Version:          {:s} (v{:d})\n", getTitleVersionStr(body.getTicketVersion()), body.getTicketVersion());
	fmt::print("  License Type:     {:s}\n", getLicenseTypeStr(body.getLicenseType())); 
	if (body.getPropertyFlags().size() > 0 || mCliOutputMode.show_extended_info)
	{
		pie::hac::es::sTicketBody_v2* raw_body = (pie::hac::es::sTicketBody_v2*)body.getBytes().data();
		fmt::print("  PropertyMask:     0x{:04x}\n", ((tc::bn::le16<uint16_t>*)&raw_body->property_mask)->unwrap());
		for (size_t i = 0; i < body.getPropertyFlags().size(); i++)
		{
			fmt::print("    {:s}\n", getPropertyFlagStr(body.getPropertyFlags()[i]));
		}
	}
	if (mCliOutputMode.show_extended_info)
	{
		fmt::print("  Reserved Region:\n");
		fmt::print("    {:s}\n", tc::cli::FormatUtil::formatBytesAsString(body.getReservedRegion(), 8, true, ""));
	}
	
	if (body.getTicketId() != 0 || mCliOutputMode.show_extended_info)
		fmt::print("  TicketId:         0x{:016x}\n", body.getTicketId());
	
	if (body.getDeviceId() != 0 || mCliOutputMode.show_extended_info)
		fmt::print("  DeviceId:         0x{:016x}\n", body.getDeviceId());
	
	fmt::print("  RightsId:         \n");
	fmt::print("    {:s}\n", tc::cli::FormatUtil::formatBytesAsString(body.getRightsId(), 16, true, ""));

	fmt::print("  SectionTotalSize:       0x{:x}\n", body.getSectionTotalSize());
	fmt::print("  SectionHeaderOffset:    0x{:x}\n", body.getSectionHeaderOffset());
	fmt::print("  SectionNum:             0x{:x}\n", body.getSectionNum());
	fmt::print("  SectionEntrySize:       0x{:x}\n", body.getSectionEntrySize());
}

std::string nstool::EsTikProcess::getSignTypeStr(uint32_t type) const
{
	std::string str;
	switch(type)
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

std::string nstool::EsTikProcess::getTitleKeyPersonalisationStr(byte_t flag) const
{
	std::string str;
	switch(flag)
	{
	case (pie::hac::es::ticket::AES128_CBC):
		str = "Generic (AESCBC)";
		break;
	case (pie::hac::es::ticket::RSA2048):
		str = "Personalised (RSA2048)";
		break;
	default:
		str = fmt::format("Unknown ({:d})", flag);
		break;
	}
	return str;
}

std::string nstool::EsTikProcess::getLicenseTypeStr(byte_t flag) const
{
	std::string str;
	switch(flag)
	{
	case (pie::hac::es::ticket::LICENSE_PERMANENT):
		str = "Permanent";
		break;
	case (pie::hac::es::ticket::LICENSE_DEMO):
		str = "Demo";
		break;
	case (pie::hac::es::ticket::LICENSE_TRIAL):
		str = "Trial";
		break;
	case (pie::hac::es::ticket::LICENSE_RENTAL):
		str = "Rental";
		break;
	case (pie::hac::es::ticket::LICENSE_SUBSCRIPTION):
		str = "Subscription";
		break;
	case (pie::hac::es::ticket::LICENSE_SERVICE):
		str = "Service";
		break;
	default:
		str = fmt::format("Unknown ({:d})", flag);
		break;
	}
	return str;
}

std::string nstool::EsTikProcess::getPropertyFlagStr(byte_t flag) const
{
	std::string str;
	switch(flag)
	{
	case (pie::hac::es::ticket::FLAG_PRE_INSTALL):
		str = "PreInstall";
		break;
	case (pie::hac::es::ticket::FLAG_SHARED_TITLE):
		str = "SharedTitle";
		break;
	case (pie::hac::es::ticket::FLAG_ALLOW_ALL_CONTENT):
		str = "AllContent";
		break;
	case (pie::hac::es::ticket::FLAG_DEVICE_LINK_INDEPENDENT):
		str = "DeviceLinkIndependent";
		break;
	case (pie::hac::es::ticket::FLAG_VOLATILE):
		str = "Volatile";
		break;
	case (pie::hac::es::ticket::FLAG_ELICENSE_REQUIRED):
		str = "ELicenseRequired";
		break;
	default:
		str = fmt::format("Unknown ({:d})", flag);
		break;
	}
	return str;
}

std::string nstool::EsTikProcess::getTitleVersionStr(uint16_t version) const
{
	return fmt::format("{:d}.{:d}.{:d}", ((version>>10) & 0x3f), ((version>>4) & 0x3f), ((version>>0) & 0xf));
}