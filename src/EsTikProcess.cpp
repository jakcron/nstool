#include "EsTikProcess.h"
#include "PkiValidator.h"

#include <pietendo/hac/es/SignUtils.h>
#include <pietendo/hac/AesKeygen.h>


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
	decryptTitleKey();

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

void nstool::EsTikProcess::decryptTitleKey()
{
	const pie::hac::es::TicketBody_V2& body = mTik.getBody();

	tc::ByteData raw_title_key = tc::ByteData(body.getEncTitleKey(), 0x100);
	tc::ByteData depersonalised_key = tc::ByteData(0x10);
	tc::ByteData decrypted_key = tc::ByteData(0x10);

	// try to decrypt the title key
	try
	{
		// check if enc type is supported
		if (body.getTitleKeyEncType() != pie::hac::es::ticket::RSA2048 && body.getTitleKeyEncType() != pie::hac::es::ticket::AES128_CBC)
		{
			throw tc::Exception(fmt::format("Unsupported TitleKeyEncType: {:d}", (byte_t)body.getTitleKeyEncType()));
		}

		bool hasPersonalisedEnc = body.getTitleKeyEncType() == pie::hac::es::ticket::RSA2048;
		bool hasCommonEnc = body.getTitleKeyEncType() == pie::hac::es::ticket::RSA2048 || body.getTitleKeyEncType() == pie::hac::es::ticket::AES128_CBC;

		switch (body.getTitleKeyEncType())
		{
			case (pie::hac::es::ticket::AES128_CBC):
				hasPersonalisedEnc = false;
				hasCommonEnc = true;
				break;
			case (pie::hac::es::ticket::RSA2048):
				hasPersonalisedEnc = true;
				hasCommonEnc = true;
				break;
			default:
				throw tc::Exception(fmt::format("Unsupported TitleKeyEncType 0x{:x}", (uint32_t)body.getTitleKeyEncType()));
		}

		

		// strip personalised enc layer
		if (hasPersonalisedEnc)
		{
			if (mKeyCfg.etik_device_key.isSet())
			{
				tc::crypto::RsaKey console_eticket_rsa_key = mKeyCfg.etik_device_key.get();
				size_t dec_data_len = 0x10;	
				bool depersonalise_successful = false;
				
				depersonalise_successful = tc::crypto::DecryptRsa2048OaepSha2256(depersonalised_key.data(), dec_data_len, depersonalised_key.size(), raw_title_key.data(), console_eticket_rsa_key, nullptr, 0, false);

				if (depersonalise_successful != true)
				{
					throw tc::Exception("Decrypting personalisation layer failed, check ticket device RSA key is correct");
				}
			}
			else
			{
				throw tc::Exception("Decrypting personalisation layer failed, no ticket device RSA key was provided");
			}
		}

		// strip common enc layer
		if (hasCommonEnc)
		{
			// determine key to decrypt title key
			byte_t common_key_id = mTik.getBody().getCommonKeyId();

			// work around for bad scene tickets where they don't set the commonkey id field (detect scene ticket with ffff.... signature)
			if (common_key_id == 0 && *((uint64_t*)mTik.getSignature().getSignature().data()) == (uint64_t)0xffffffffffffffff)
			{
				const byte_t* rights_id = mTik.getBody().getRightsId();
				static const size_t kRightsIdSize = pie::hac::es::ticket::kRightsIdSize;

				fmt::print("[WARNING] Ticket \"{:s}\" is fake-signed, and title key decryption may fail if ticket was incorrectly generated.\n", tc::cli::FormatUtil::formatBytesAsString(rights_id, kRightsIdSize, true, ""));
				// the keygeneration was included in the rights_id from keygeneration 0x03 and onwards, so in those cases we can copy from there
				if (rights_id[15] >= 0x03)
					common_key_id = rights_id[15];
			}

			// convert key_generation to mkey_revision
			byte_t common_key_index = pie::hac::AesKeygen::getMasterKeyRevisionFromKeyGeneration(common_key_id);

			try 
			{
				if (mKeyCfg.etik_common_key.find(common_key_index) != mKeyCfg.etik_common_key.end())
				{
					// decrypt title key (this will throw an exception is something unexpected happens)
					tc::crypto::DecryptAes128Ecb(decrypted_key.data(), depersonalised_key.data(), sizeof(decrypted_key), mKeyCfg.etik_common_key[common_key_index].data(),  mKeyCfg.etik_common_key[common_key_index].size());					
				}
				else
				{
					throw tc::Exception(fmt::format("ticket_commonkey was not provided for master_key revision 0x{%02x}.\n", common_key_index));
				}
			} catch (const tc::Exception& e)
			{
				throw tc::Exception(fmt::format("Decrypting common layer failed, %s.\n", e.error()));
			}
		}

		// set class decrypted title key variables 
		mDepersonalisedTitleKey = tc::ByteData(depersonalised_key);
		mDecryptedTitleKey = tc::ByteData(decrypted_key);

	} catch (tc::Exception& e)
	{
		fmt::print("[WARNING] Ticket title key could not be decrypted ({:s})\n", e.error());
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

	// Format/present the titlekey
	std::string depersonalised_title_key_str = mDepersonalisedTitleKey.isNull() ? "<failed to depersonalise>" : tc::cli::FormatUtil::formatBytesAsString(mDepersonalisedTitleKey.get().data(), mDepersonalisedTitleKey.get().size(), true, "");
	std::string decrypted_title_key_str = mDecryptedTitleKey.isNull() ? "<failed to decrypt>" : tc::cli::FormatUtil::formatBytesAsString(mDecryptedTitleKey.get().data(), mDecryptedTitleKey.get().size(), true, "");

	if (body.getTitleKeyEncType() == pie::hac::es::ticket::RSA2048)
	{
		fmt::print("    Data:\n");
		fmt::print("      {:s}", tc::cli::FormatUtil::formatBytesAsStringWithLineLimit(body.getEncTitleKey(), 0x100, true, "", 0x10, 6, false));
		fmt::print("    Title Key Depersonalised (device encryption):\n");
		fmt::print("      {:s}\n", depersonalised_title_key_str);
		fmt::print("    Title Key Decrypted (common encryption):\n");
		fmt::print("      {:s}\n", decrypted_title_key_str);
	}
	else if (body.getTitleKeyEncType() == pie::hac::es::ticket::AES128_CBC)
	{
		fmt::print("    Data:\n");
		fmt::print("      {:s}\n", tc::cli::FormatUtil::formatBytesAsString(body.getEncTitleKey(), 0x10, true, ""));
		fmt::print("    Title Key Decrypted (common encryption):\n");
		fmt::print("      {:s}\n", decrypted_title_key_str);
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
	
	if (body.getAccountId() != 0 || mCliOutputMode.show_extended_info)
		fmt::print("  AccountId:        0x{:08x}\n", body.getAccountId());

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
		str = "Generic";
		break;
	case (pie::hac::es::ticket::RSA2048):
		str = "Personalised";
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