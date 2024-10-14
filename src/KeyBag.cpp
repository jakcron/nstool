#include "KeyBag.h"

#include "util.h"
#include <tc/cli/FormatUtil.h>

#include <pietendo/hac/define/types.h>
#include <pietendo/hac/define/gc.h>
#include <pietendo/hac/AesKeygen.h>

#include <pietendo/hac/es/SignUtils.h>
#include <pietendo/hac/es/SignedData.h>
#include <pietendo/hac/es/CertificateBody.h>
#include <pietendo/hac/es/TicketBody_V2.h>

nstool::KeyBagInitializer::KeyBagInitializer(bool isDev, const tc::Optional<tc::io::Path>& keyfile_path, const tc::Optional<tc::io::Path>& titlekeyfile_path, const std::vector<tc::io::Path>& tik_path_list, const tc::Optional<tc::io::Path>& cert_path)
{
	if (keyfile_path.isSet())
	{
		importBaseKeyFile(keyfile_path.get(), isDev);
	}
	if (titlekeyfile_path.isSet())
	{
		importTitleKeyFile(titlekeyfile_path.get());
	}
	if (cert_path.isSet())
	{
		importCertificateChain(cert_path.get());
	}
	if (!tik_path_list.empty())
	{
		for (auto itr = tik_path_list.begin(); itr != tik_path_list.end(); itr++)
			importTicket(*itr);
	}

	// this will populate known keys if they aren't supplied by the user provided keyfiles.
	importKnownKeys(isDev);
}

void nstool::KeyBagInitializer::importBaseKeyFile(const tc::io::Path& keyfile_path, bool isDev)
{
	std::shared_ptr<tc::io::FileStream> keyfile_stream = std::make_shared<tc::io::FileStream>(tc::io::FileStream(keyfile_path, tc::io::FileMode::Open, tc::io::FileAccess::Read));

	// import keyfile into a dictionary
	std::map<std::string, std::string> keyfile_dict;
	processResFile(keyfile_stream, keyfile_dict);

	// sources for key derivation
	std::map<byte_t, aes128_key_t> master_key;
	tc::Optional<aes128_key_t> package2_key_source;
	tc::Optional<aes128_key_t> ticket_titlekek_source;
	std::array<tc::Optional<aes128_key_t>, 3> key_area_key_source;
	tc::Optional<aes128_key_t> aes_kek_generation_source;
	tc::Optional<aes128_key_t> aes_key_generation_source;
	tc::Optional<aes128_key_t> nca_header_kek_source;
	tc::Optional<aes128_xtskey_t> nca_header_key_source;
	tc::Optional<rsa_key_t> pki_root_sign_key;

	// macros for importing
	
#define _SAVE_AES128KEY(key_name, dst) \
	{ \
	std::string key,val; \
	tc::ByteData dec_val; \
	aes128_key_t tmp_aes128_key; \
	key = (key_name); \
	val = keyfile_dict[key]; \
	if (val.empty() == false) { \
		dec_val = tc::cli::FormatUtil::hexStringToBytes(val); \
		if (dec_val.size() != tmp_aes128_key.size()) \
			throw tc::ArgumentException("nstool::KeyBagInitializer", "Key: \"" + key_name + "\" has incorrect length"); \
		memcpy(tmp_aes128_key.data(), dec_val.data(), tmp_aes128_key.size()); \
		(dst) = tmp_aes128_key; \
	} \
	}

#define _SAVE_AES128XTSKEY(key_name, dst) \
	{ \
	std::string key,val; \
	tc::ByteData dec_val; \
	aes128_xtskey_t tmp_aes128_xtskey; \
	key = (key_name); \
	val = keyfile_dict[key]; \
	if (val.empty() == false) { \
		dec_val = tc::cli::FormatUtil::hexStringToBytes(val); \
		if (dec_val.size() != sizeof(tmp_aes128_xtskey)) \
			throw tc::ArgumentException("nstool::KeyBagInitializer", "Key: \"" + key_name + "\" has incorrect length"); \
		memcpy(tmp_aes128_xtskey[0].data(), dec_val.data(), tmp_aes128_xtskey[0].size()); \
		memcpy(tmp_aes128_xtskey[1].data(), dec_val.data()+tmp_aes128_xtskey[0].size(), tmp_aes128_xtskey[1].size()); \
		(dst) = tmp_aes128_xtskey; \
	} \
	}

#define _SAVE_RSAKEY(key_name, dst, bitsize) \
	{ \
	std::string key_mod,key_prv,val_mod,val_prv; \
	tc::ByteData dec_val; \
	rsa_key_t tmp_rsa_key; \
	key_mod = fmt::format("{:s}_modulus", (key_name)); \
	key_prv = fmt::format("{:s}_private", (key_name)); \
	val_mod = keyfile_dict[key_mod]; \
	val_prv = keyfile_dict[key_prv]; \
	if (val_mod.empty() == false) { \
		dec_val = tc::cli::FormatUtil::hexStringToBytes(val_mod); \
		if (dec_val.size() == (bitsize) >> 3) { \
			tmp_rsa_key.n = dec_val; \
			if (val_prv.empty() == false) { \
				dec_val = tc::cli::FormatUtil::hexStringToBytes(val_prv); \
				if (dec_val.size() == (bitsize) >> 3) { \
					tmp_rsa_key.d = dec_val; \
					(dst) = tc::crypto::RsaPrivateKey(tmp_rsa_key.n.data(), tmp_rsa_key.n.size(), tmp_rsa_key.d.data(), tmp_rsa_key.d.size()); \
				} \
				else { \
					fmt::print("[WARNING] Key: \"{:s}\" has incorrect length (was: {:d}, expected {:d})\n", key_prv, val_prv.size(), ((bitsize) >> 3)*2); \
				} \
			} \
			else { \
				(dst) = tc::crypto::RsaPublicKey(tmp_rsa_key.n.data(), tmp_rsa_key.n.size()); \
			} \
		} \
		else {\
			fmt::print("[WARNING] Key: \"{:s}\" has incorrect length (was: {:d}, expected {:d})\n", key_mod, val_mod.size(), ((bitsize) >> 3)*2); \
		} \
	} \
	}
	
	// keynames
	enum NameVariantIndex
	{
		NNTOOLS,
		LEGACY_HACTOOL,
		LEGACY_0
	};
	
	static const size_t kNameVariantNum = 3;
	
	std::vector<std::string> kMasterBase = { "master" };
	std::vector<std::string> kPkg1Base = { "package1" };
	std::vector<std::string> kPkg2Base = { "package2" };
	std::vector<std::string> kXciHeaderBase = { "xci_header" };
	std::vector<std::string> kXciInitialDataBase = { "xci_initial_data" };
	std::vector<std::string> kXciCertBase = { "xci_cert" };
	std::vector<std::string> kContentArchiveHeaderBase = { "nca_header", "header" };
	std::vector<std::string> kAcidBase = { "acid" };
	std::vector<std::string> kNrrCertBase = { "nrr_certificate" };
	std::vector<std::string> kPkiRootBase = { "pki_root" };
	std::vector<std::string> kTicketCommonKeyBase = { "ticket_commonkey", "titlekek" };
	std::vector<std::string> kNcaKeyAreaEncKeyBase = { "nca_key_area_key", "key_area_key", "nca_body_keak" };
	std::vector<std::string> kNcaKeyAreaEncKeyHwBase = { "nca_key_area_key_hw", "key_area_hw_key" };
	std::vector<std::string> kKekGenBase = { "aes_kek_generation" };
	std::vector<std::string> kKeyGenBase = { "aes_key_generation" };
	
	// misc str
	const std::string kKeyStr = "key";
	const std::string kKekStr = "kek";
	const std::string kSourceStr = "source";
	const std::string kSignKey = "sign_key";
	const std::string kModulusStr = "modulus";
	const std::string kPrivateStr = "private";
	std::vector<std::string> kNcaKeyAreaKeyIndexStr = { "application", "ocean", "system" };

	static const size_t kKeyGenerationNum = 0x100;
	/**/

	// import key data
	for (size_t name_idx = 0; name_idx < kNameVariantNum; name_idx++)
	{
		/* internal key sources */
		if (name_idx < kMasterBase.size())
		{
			for (size_t keygen_rev = 0; keygen_rev < kKeyGenerationNum; keygen_rev++)
			{
				// std::map<byte_t, aes128_key_t> master_key;
				//fmt::print("{:s}_key_{:02x}\n", kMasterBase[name_idx], keygen_rev);
				_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:02x}", kMasterBase[name_idx], kKeyStr, keygen_rev), master_key[(byte_t)keygen_rev]);
			}
		}

		if (name_idx < kPkg2Base.size())
		{
			// tc::Optional<aes128_key_t> package2_key_source;
			//fmt::print("{:s}_key_source\n", kPkg2Base[name_idx]);
			_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:s}", kPkg2Base[name_idx], kKeyStr, kSourceStr), package2_key_source);
		}

		if (name_idx < kTicketCommonKeyBase.size())
		{
			// tc::Optional<aes128_key_t> ticket_titlekek_source;
			//fmt::print("{:s}_source\n", kTicketCommonKeyBase[name_idx]);
			_SAVE_AES128KEY(fmt::format("{:s}_{:s}", kTicketCommonKeyBase[name_idx], kSourceStr), ticket_titlekek_source);
		}

		if (name_idx < kNcaKeyAreaEncKeyBase.size())
		{
			// std::array<tc::Optional<aes128_key_t>, 3> key_area_key_source;

			for (size_t keak_idx = 0; keak_idx < kNcaKeyAreaKeyIndexStr.size(); keak_idx++)
			{
				//fmt::print("{:s}_{:s}_source\n", kNcaKeyAreaEncKeyBase[name_idx], kNcaKeyAreaKeyIndexStr[keak_idx]);
				_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:s}", kNcaKeyAreaEncKeyBase[name_idx], kNcaKeyAreaKeyIndexStr[keak_idx], kSourceStr), key_area_key_source[keak_idx]);
			}
		}
		
		if (name_idx < kKekGenBase.size())
		{
			// tc::Optional<aes128_key_t> aes_kek_generation_source;
			//fmt::print("{:s}_source\n", kKekGenBase[name_idx]);
			_SAVE_AES128KEY(fmt::format("{:s}_{:s}", kKekGenBase[name_idx], kSourceStr), aes_kek_generation_source);
		}

		if (name_idx < kKeyGenBase.size())
		{
			// tc::Optional<aes128_key_t> aes_key_generation_source;
			//fmt::print("{:s}_source\n", kKeyGenBase[name_idx]);
			_SAVE_AES128KEY(fmt::format("{:s}_{:s}", kKeyGenBase[name_idx], kSourceStr), aes_key_generation_source);
		}

		if (name_idx < kContentArchiveHeaderBase.size())
		{
			// tc::Optional<aes128_key_t> nca_header_kek_source;
			//fmt::print("{:s}_kek_source\n", kContentArchiveHeaderBase[name_idx]);
			_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:s}", kContentArchiveHeaderBase[name_idx], kKekStr, kSourceStr), nca_header_kek_source);
		}

		if (name_idx < kContentArchiveHeaderBase.size())
		{
			// tc::Optional<aes128_xtskey_t> nca_header_key_source;
			//fmt::print("{:s}_key_source\n", kContentArchiveHeaderBase[name_idx]);
			_SAVE_AES128XTSKEY(fmt::format("{:s}_{:s}_{:s}", kContentArchiveHeaderBase[name_idx], kKeyStr, kSourceStr), nca_header_key_source);
		}

		/* package1 */ 
		// package1_key_xx
		if (name_idx < kPkg1Base.size())
		{
			for (size_t keygen_rev = 0; keygen_rev < kKeyGenerationNum; keygen_rev++)
			{
				//fmt::print("{:s}_key_{:02x}\n", kPkg1Base[name_idx], keygen_rev);
				_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:02x}", kPkg1Base[name_idx], kKeyStr, keygen_rev), pkg1_key[(byte_t)keygen_rev]);
			}
		}

		/* package2 */
		if (name_idx < kPkg2Base.size())
		{
			// package2_key_xx
			for (size_t keygen_rev = 0; keygen_rev < kKeyGenerationNum; keygen_rev++)
			{
				//fmt::print("{:s}_key_{:02x}\n", kPkg2Base[name_idx], keygen_rev);
				_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:02x}", kPkg2Base[name_idx], kKeyStr, keygen_rev), pkg2_key[(byte_t)keygen_rev]);
			}

			// package2_sign_key
			//fmt::print("{:s}_{:s}_{:s}\n", kPkg2Base[name_idx], kSignKey, kPrivateStr);
			//fmt::print("{:s}_{:s}_{:s}\n", kPkg2Base[name_idx], kSignKey, kModulusStr);
			_SAVE_RSAKEY(fmt::format("{:s}_{:s}", kPkg2Base[name_idx], kSignKey), pkg2_sign_key, 2048);
		}

		/* eticket */
		// ticket common key
		if (name_idx < kTicketCommonKeyBase.size())
		{
			for (size_t keygen_rev = 0; keygen_rev < kKeyGenerationNum; keygen_rev++)
			{
				//fmt::print("{:s}_{:02x}\n", kTicketCommonKeyBase[name_idx], keygen_rev);
				_SAVE_AES128KEY(fmt::format("{:s}_{:02x}", kTicketCommonKeyBase[name_idx], keygen_rev), etik_common_key[(byte_t)keygen_rev]);
			}
		}

		/* NCA keys */
		if (name_idx < kContentArchiveHeaderBase.size())
		{
			// nca header key
			//fmt::print("{:s}_{:s}\n", kContentArchiveHeaderBase[name_idx], kKeyStr);
			_SAVE_AES128XTSKEY(fmt::format("{:s}_{:s}", kContentArchiveHeaderBase[name_idx], kKeyStr), nca_header_key);
			
			// nca header sign0 key (generations)
			for (size_t keygen_rev = 0; keygen_rev < kKeyGenerationNum; keygen_rev++)
			{
				//fmt::print("{:s}_{:s}_{:02x}_{:s}\n", kContentArchiveHeaderBase[name_idx], kSignKey, keygen_rev, kPrivateStr);
				//fmt::print("{:s}_{:s}_{:02x}_{:s}\n", kContentArchiveHeaderBase[name_idx], kSignKey, keygen_rev, kModulusStr);
				_SAVE_RSAKEY(fmt::format("{:s}_{:s}_{:02x}", kContentArchiveHeaderBase[name_idx], kSignKey, keygen_rev), nca_header_sign0_key[(byte_t)keygen_rev], 2048);
			}
			// nca header sign0 key (generation 0)
			//fmt::print("{:s}_{:s}_{:s}\n", kContentArchiveHeaderBase[name_idx], kSignKey, kPrivateStr);
			//fmt::print("{:s}_{:s}_{:s}\n", kContentArchiveHeaderBase[name_idx], kSignKey, kModulusStr);
			_SAVE_RSAKEY(fmt::format("{:s}_{:s}", kContentArchiveHeaderBase[name_idx], kSignKey), nca_header_sign0_key[0], 2048);
			
		}

		// nca body key (unused since prototype format)

		// nca key area encryption keys
		if (name_idx < kNcaKeyAreaEncKeyBase.size())
		{
			for (size_t keygen_rev = 0; keygen_rev < kKeyGenerationNum; keygen_rev++)
			{
				for (size_t keak_idx = 0; keak_idx < kNcaKeyAreaKeyIndexStr.size(); keak_idx++)
				{
					//fmt::print("{:s}_{:s}_{:02x}\n", kNcaKeyAreaEncKeyBase[name_idx], kNcaKeyAreaKeyIndexStr[keak_idx], keygen_rev);
					_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:02x}", kNcaKeyAreaEncKeyBase[name_idx], kNcaKeyAreaKeyIndexStr[keak_idx], keygen_rev), nca_key_area_encryption_key[keak_idx][(byte_t)keygen_rev]);
				}
			}
		}
		// nca key area "hw" encryption keys
		if (name_idx < kNcaKeyAreaEncKeyHwBase.size())
		{
			for (size_t keygen_rev = 0; keygen_rev < kKeyGenerationNum; keygen_rev++)
			{
				for (size_t keak_idx = 0; keak_idx < kNcaKeyAreaKeyIndexStr.size(); keak_idx++)
				{
					//fmt::print("{:s}_{:s}_{:02x}\n", kNcaKeyAreaEncKeyHwBase[name_idx], kNcaKeyAreaKeyIndexStr[keak_idx], keygen_rev);
					_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:02x}", kNcaKeyAreaEncKeyHwBase[name_idx], kNcaKeyAreaKeyIndexStr[keak_idx], keygen_rev), nca_key_area_encryption_key_hw[keak_idx][(byte_t)keygen_rev]);
				}
			}
		}

		/* ACID */
		if (name_idx < kAcidBase.size())
		{
			// acid sign key (generations)
			for (size_t keygen_rev = 0; keygen_rev < kKeyGenerationNum; keygen_rev++)
			{
				//fmt::print("{:s}_{:s}_{:02x}_{:s}\n", kAcidBase[name_idx], kSignKey, keygen_rev, kPrivateStr);
				//fmt::print("{:s}_{:s}_{:02x}_{:s}\n", kAcidBase[name_idx], kSignKey, keygen_rev, kModulusStr);
				_SAVE_RSAKEY(fmt::format("{:s}_{:s}_{:02x}", kAcidBase[name_idx], kSignKey, keygen_rev), acid_sign_key[(byte_t)keygen_rev], 2048);
			}
			// acid sign key (generation 0)
			//fmt::print("{:s}_{:s}_{:s}\n", kAcidBase[name_idx], kSignKey, kPrivateStr);
			//fmt::print("{:s}_{:s}_{:s}\n", kAcidBase[name_idx], kSignKey, kModulusStr);
			_SAVE_RSAKEY(fmt::format("{:s}_{:s}", kAcidBase[name_idx], kSignKey), acid_sign_key[0], 2048);
		}

		/* NRR certificate */
		if (name_idx < kNrrCertBase.size())
		{
			// nrr certificate sign key (generations)
			for (size_t keygen_rev = 0; keygen_rev < kKeyGenerationNum; keygen_rev++)
			{
				//fmt::print("{:s}_{:s}_{:02x}_{:s}\n", kNrrCertBase[name_idx], kSignKey, keygen_rev, kPrivateStr);
				//fmt::print("{:s}_{:s}_{:02x}_{:s}\n", kNrrCertBase[name_idx], kSignKey, keygen_rev, kModulusStr);
				_SAVE_RSAKEY(fmt::format("{:s}_{:s}_{:02x}", kNrrCertBase[name_idx], kSignKey, keygen_rev), nrr_certificate_sign_key[(byte_t)keygen_rev], 2048);
			}
			// nrr certificate sign key (generation 0)
			//fmt::print("{:s}_{:s}_{:s}\n", kNrrCertBase[name_idx], kSignKey, kPrivateStr);
			//fmt::print("{:s}_{:s}_{:s}\n", kNrrCertBase[name_idx], kSignKey, kModulusStr);
			_SAVE_RSAKEY(fmt::format("{:s}_{:s}", kNrrCertBase[name_idx], kSignKey), nrr_certificate_sign_key[0], 2048);
		}

		/* XCI header */
		if (name_idx < kXciHeaderBase.size())
		{
			// xci header key (based on index)
			for (byte_t kek_index = 0; kek_index < 8; kek_index++)
			{
				//fmt::print("{:s}_{:s}_{:02x}\n", kXciHeaderBase[name_idx], kKeyStr, kek_index);
				_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:02x}", kXciHeaderBase[name_idx], kKeyStr, kek_index), xci_header_key[kek_index]);
			}
			// xci header key (old label, prod/dev keys are actually a fake distinction, the are different key indexes available to both?, so select correct index when importing)
			//fmt::print("{:s}_{:s}\n", kXciHeaderBase[name_idx], kKeyStr);
			_SAVE_AES128KEY(fmt::format("{:s}_{:s}", kXciHeaderBase[name_idx], kKeyStr), xci_header_key[isDev ? pie::hac::gc::KekIndex_Dev : pie::hac::gc::KekIndex_Prod]);

			// xci header sign key
			//fmt::print("{:s}_{:s}_{:s}\n", kXciHeaderBase[name_idx], kSignKey, kPrivateStr);
			//fmt::print("{:s}_{:s}_{:s}\n", kXciHeaderBase[name_idx], kSignKey, kModulusStr);
			_SAVE_RSAKEY(fmt::format("{:s}_{:s}", kXciHeaderBase[name_idx], kSignKey), xci_header_sign_key, 2048);
		}

		/* XCI InitialData */
		if (name_idx < kXciInitialDataBase.size())
		{
			// xci initial data key (based on index)
			for (byte_t kek_index = 0; kek_index < 8; kek_index++)
			{
				//fmt::print("{:s}_{:s}_{:02x}\n", kXciInitialDataBase[name_idx], kKekStr, kek_index);
				_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:02x}", kXciInitialDataBase[name_idx], kKekStr, kek_index), xci_initial_data_kek[kek_index]);
			}
		}

		/* XCI cert */
		if (name_idx < kXciCertBase.size())
		{
			// xci cert sign key
			_SAVE_RSAKEY(fmt::format("{:s}_{:s}", kXciCertBase[name_idx], kSignKey), xci_cert_sign_key, 2048);
		}

		/* PKI */
		if (name_idx < kPkiRootBase.size())
		{
			// tc::Optional<rsa_key_t> pki_root_sign_key;
			//fmt::print("{:s}_{:s}_{:s}\n", kPkiRootBase[name_idx], kSignKey, kPrivateStr);
			//fmt::print("{:s}_{:s}_{:s}\n", kPkiRootBase[name_idx], kSignKey, kModulusStr);
			_SAVE_RSAKEY(fmt::format("{:s}_{:s}", kPkiRootBase[name_idx], kSignKey), pki_root_sign_key, 4096);
		}

		
	}

#undef _SAVE_RSAKEY
#undef _SAVE_AES128XTSKEY
#undef _SAVE_AES128KEY

	// Derive Keys
	for (auto itr = master_key.begin(); itr != master_key.end(); itr++)
	{
		if (aes_kek_generation_source.isSet() && aes_key_generation_source.isSet())
		{
			if (itr->first == 0 && nca_header_kek_source.isSet() && nca_header_key_source.isSet())
			{
				if (nca_header_key.isNull())
				{
					aes128_key_t nca_header_kek_tmp;
					pie::hac::AesKeygen::generateKey(nca_header_kek_tmp.data(), aes_kek_generation_source.get().data(), nca_header_kek_source.get().data(), aes_key_generation_source.get().data(), itr->second.data());
					
					aes128_xtskey_t nca_header_key_tmp;
					pie::hac::AesKeygen::generateKey(nca_header_key_tmp[0].data(), nca_header_key_source.get()[0].data(), nca_header_kek_tmp.data());
					pie::hac::AesKeygen::generateKey(nca_header_key_tmp[1].data(), nca_header_key_source.get()[1].data(), nca_header_kek_tmp.data());

					nca_header_key = nca_header_key_tmp;
				}
			}

			for (size_t keak_idx = 0; keak_idx < pie::hac::nca::kKeyAreaEncryptionKeyNum; keak_idx++)
			{
				if (key_area_key_source[keak_idx].isSet() && nca_key_area_encryption_key[keak_idx].find(itr->first) == nca_key_area_encryption_key[keak_idx].end())
				{
					aes128_key_t nca_key_area_encryption_key_tmp;
					pie::hac::AesKeygen::generateKey(nca_key_area_encryption_key_tmp.data(), aes_kek_generation_source.get().data(), key_area_key_source[keak_idx].get().data(), aes_key_generation_source.get().data(), itr->second.data());
					nca_key_area_encryption_key[keak_idx][itr->first] = nca_key_area_encryption_key_tmp;
				}
			}
		}
		if (ticket_titlekek_source.isSet() && etik_common_key.find(itr->first) == etik_common_key.end())
		{
			aes128_key_t etik_common_key_tmp;
			pie::hac::AesKeygen::generateKey(etik_common_key_tmp.data(), ticket_titlekek_source.get().data(), itr->second.data());
			etik_common_key[itr->first] = etik_common_key_tmp;
		}
		if (package2_key_source.isSet() && pkg2_key.find(itr->first) == pkg2_key.end())
		{
			aes128_key_t pkg2_key_tmp;
			pie::hac::AesKeygen::generateKey(pkg2_key_tmp.data(), package2_key_source.get().data(), itr->second.data());
			pkg2_key[itr->first] = pkg2_key_tmp;
		}
	}

	// Save PKI Root Key
	if (pki_root_sign_key.isSet())
	{
		broadon_signer["Root"] = { tc::ByteData(), pie::hac::es::sign::SIGN_ALGO_RSA4096, pki_root_sign_key.get() };
	}
}

void nstool::KeyBagInitializer::importTitleKeyFile(const tc::io::Path& keyfile_path)
{
	std::shared_ptr<tc::io::FileStream> keyfile_stream = std::make_shared<tc::io::FileStream>(tc::io::FileStream(keyfile_path, tc::io::FileMode::Open, tc::io::FileAccess::Read));

	// import keyfile into a dictionary
	std::map<std::string, std::string> keyfile_dict;
	processResFile(keyfile_stream, keyfile_dict);

	// process title keys
	tc::ByteData tmp;
	KeyBag::rights_id_t rights_id_tmp;
	KeyBag::aes128_key_t title_key_tmp;
	for (auto itr = keyfile_dict.begin(); itr != keyfile_dict.end(); itr++)
	{
		//fmt::print("RightsID[{:s}] = TitleKey[{:s}]\n", itr->first, itr->second);

		// parse the rights id
		tmp = tc::cli::FormatUtil::hexStringToBytes(itr->first);
		if (tmp.size() != rights_id_tmp.size())
		{
			fmt::print("[nstool::KeyBagInitializer WARNING] RightsID: \"{}\" has incorrect length. Skipping...\n", itr->first);
			continue;
		}
		memcpy(rights_id_tmp.data(), tmp.data(), rights_id_tmp.size());

		// parse the title key
		tmp = tc::cli::FormatUtil::hexStringToBytes(itr->second);
		if (tmp.size() != title_key_tmp.size())
		{
			fmt::print("[nstool::KeyBagInitializer WARNING] TitleKey for \"{}\": \"{}\" has incorrect length. Skipping...\n", itr->first, itr->second);
			continue;
		}
		memcpy(title_key_tmp.data(), tmp.data(), title_key_tmp.size());

		// save to encrypted key dict
		external_enc_content_keys[rights_id_tmp] = title_key_tmp;
	}
}

void nstool::KeyBagInitializer::importCertificateChain(const tc::io::Path& cert_path)
{
	// open cert file
	std::shared_ptr<tc::io::FileStream> certfile_stream;
	try {
		certfile_stream = std::make_shared<tc::io::FileStream>(tc::io::FileStream(cert_path, tc::io::FileMode::Open, tc::io::FileAccess::Read));
	}
	catch (tc::io::FileNotFoundException& e) {
		fmt::print("[WARNING] Failed to open certificate file \"{:s}\" ({:s}).\n", cert_path.to_string(), e.error());
		return;
	}
	
	// check size
	size_t cert_raw_size = tc::io::IOUtil::castInt64ToSize(certfile_stream->length());
	if (cert_raw_size > 0x10000)
	{
		fmt::print("[WARNING] Certificate file \"{:s}\" was too large.\n", cert_path.to_string());
		return;
	}

	// import cert data
	tc::ByteData cert_raw = tc::ByteData(cert_raw_size);
	certfile_stream->seek(0, tc::io::SeekOrigin::Begin);
	certfile_stream->read(cert_raw.data(), cert_raw.size());

	pie::hac::es::SignedData<pie::hac::es::CertificateBody> cert;
	try {
		for (size_t f_pos = 0; f_pos < cert_raw.size(); f_pos += cert.getBytes().size())
		{
			cert.fromBytes(cert_raw.data() + f_pos, cert_raw.size() - f_pos);

			std::string cert_identity = fmt::format("{:s}-{:s}", cert.getBody().getIssuer(), cert.getBody().getSubject());

			switch (cert.getBody().getPublicKeyType()) {
				case pie::hac::es::cert::PublicKeyType::RSA2048:
					broadon_signer[cert_identity] = { cert.getBytes(), pie::hac::es::sign::SIGN_ALGO_RSA2048, cert.getBody().getRsa2048PublicKey() };
					break;
				case pie::hac::es::cert::PublicKeyType::RSA4096:
					broadon_signer[cert_identity] = { cert.getBytes(), pie::hac::es::sign::SIGN_ALGO_RSA4096, cert.getBody().getRsa4096PublicKey() };
					break;
				case pie::hac::es::cert::PublicKeyType::ECDSA240:
					// broadon_signer[cert_identity] = { cert.getBytes(), pie::hac::es::sign::SIGN_ALGO_ECDSA240, cert.getBody().getRsa4096PublicKey() };
					fmt::print("[WARNING] Certificate {:s} will not be imported. ecc233 public keys are not supported yet.\n", cert_identity);
					break;
				default:
					fmt::print("[WARNING] Certificate {:s} will not be imported. Unknown public key type.\n", cert_identity);
			}
		}
	}
	catch (tc::Exception& e) {
		fmt::print("[WARNING] Certificate file \"{:s}\" is corrupted ({:s}).\n", cert_path.to_string(), e.error());
		return;
	}
}

void nstool::KeyBagInitializer::importTicket(const tc::io::Path& tik_path)
{
	// open cert file
	std::shared_ptr<tc::io::FileStream> tik_stream;
	try {
		tik_stream = std::make_shared<tc::io::FileStream>(tc::io::FileStream(tik_path, tc::io::FileMode::Open, tc::io::FileAccess::Read));
	}
	catch (tc::io::FileNotFoundException& e) {
		fmt::print("[WARNING] Failed to open ticket \"{:s}\" ({:s}).\n", tik_path.to_string(), e.error());
		return;
	}

	// check size
	size_t tik_raw_size = tc::io::IOUtil::castInt64ToSize(tik_stream->length());
	if (tik_raw_size > 0x10000)
	{
		fmt::print("[WARNING] Ticket \"{:s}\" was too large.\n", tik_path.to_string());
		return;
	}

	// import cert data
	tc::ByteData tik_raw = tc::ByteData(tik_raw_size);
	tik_stream->seek(0, tc::io::SeekOrigin::Begin);
	tik_stream->read(tik_raw.data(), tik_raw.size());

	pie::hac::es::SignedData<pie::hac::es::TicketBody_V2> tik;
	try {
		// de serialise ticket
		tik.fromBytes(tik_raw.data(), tik_raw.size());
		
		// save rights id
		rights_id_t rights_id;
		memcpy(rights_id.data(), tik.getBody().getRightsId(), rights_id.size());
		
		// check ticket is not personalised
		if (tik.getBody().getTitleKeyEncType() != pie::hac::es::ticket::AES128_CBC)
		{
			fmt::print("[WARNING] Ticket \"{:s}\" will not be imported. Personalised tickets are not supported.\n", tc::cli::FormatUtil::formatBytesAsString(rights_id.data(), rights_id.size(), true, ""));
			return;
		}

		// save enc title key
		aes128_key_t enc_title_key;
		memcpy(enc_title_key.data(), tik.getBody().getEncTitleKey(), enc_title_key.size());

		// save the encrypted title key as the fallback enc content key incase the ticket was malformed and workarounds to decrypt it in isolation fail
		external_enc_content_keys[rights_id] = enc_title_key;

		// determine key to decrypt title key
		byte_t common_key_index = tik.getBody().getCommonKeyId();

		// work around for bad scene tickets where they don't set the commonkey id field (detect scene ticket with ffff.... signature)
		if (common_key_index == 0 && *((uint64_t*)tik.getSignature().getSignature().data()) == (uint64_t)0xffffffffffffffff)
		{
			fmt::print("[WARNING] Ticket \"{:s}\" is fake-signed, and NCA decryption may fail if ticket was incorrectly generated.\n", tc::cli::FormatUtil::formatBytesAsString(rights_id.data(), rights_id.size(), true, ""));
			// the keygeneration was included in the rights_id from keygeneration 0x03 and onwards, so in those cases we can copy from there
			if (rights_id[15] >= 0x03)
				common_key_index = rights_id[15];
		}

		// convert key_generation
		common_key_index = pie::hac::AesKeygen::getMasterKeyRevisionFromKeyGeneration(common_key_index);

		if (etik_common_key.find(common_key_index) == etik_common_key.end())
		{
			fmt::print("[WARNING] Ticket \"{:s}\" will not be imported. Could not decrypt title key.\n", tc::cli::FormatUtil::formatBytesAsString(rights_id.data(), rights_id.size(), true, ""));
			return;
		}

		// decrypt title key
		aes128_key_t dec_title_key;
		tc::crypto::DecryptAes128Ecb(dec_title_key.data(), enc_title_key.data(), sizeof(aes128_key_t), etik_common_key[common_key_index].data(), sizeof(aes128_key_t));

		// add to decrypted key dict
		external_content_keys[rights_id] = dec_title_key;
		
	}
	catch (tc::Exception& e) {
		fmt::print("[WARNING] Ticket \"{:s}\" is corrupted ({:s}).\n", tik_path.to_string(), e.error());
		return;
	}
}

void nstool::KeyBagInitializer::importKnownKeys(bool isDev)
{
	static const pie::hac::detail::rsa2048_block_t kXciHeaderSignModulus = {
		0x98, 0xC7, 0x26, 0xB6, 0x0D, 0x0A, 0x50, 0xA7, 0x39, 0x21, 0x0A, 0xE3, 0x2F, 0xE4, 0x3E, 0x2E,
		0x5B, 0xA2, 0x86, 0x75, 0xAA, 0x5C, 0xEE, 0x34, 0xF1, 0xA3, 0x3A, 0x7E, 0xBD, 0x90, 0x4E, 0xF7,
		0x8D, 0xFA, 0x17, 0xAA, 0x6B, 0xC6, 0x36, 0x6D, 0x4C, 0x9A, 0x6D, 0x57, 0x2F, 0x80, 0xA2, 0xBC,
		0x38, 0x4D, 0xDA, 0x99, 0xA1, 0xD8, 0xC3, 0xE2, 0x99, 0x79, 0x36, 0x71, 0x90, 0x20, 0x25, 0x9D,
		0x4D, 0x11, 0xB8, 0x2E, 0x63, 0x6B, 0x5A, 0xFA, 0x1E, 0x9C, 0x04, 0xD1, 0xC5, 0xF0, 0x9C, 0xB1,
		0x0F, 0xB8, 0xC1, 0x7B, 0xBF, 0xE8, 0xB0, 0xD2, 0x2B, 0x47, 0x01, 0x22, 0x6B, 0x23, 0xC9, 0xD0,
		0xBC, 0xEB, 0x75, 0x6E, 0x41, 0x7D, 0x4C, 0x26, 0xA4, 0x73, 0x21, 0xB4, 0xF0, 0x14, 0xE5, 0xD9,
		0x8D, 0xB3, 0x64, 0xEE, 0xA8, 0xFA, 0x84, 0x1B, 0xB8, 0xB8, 0x7C, 0x88, 0x6B, 0xEF, 0xCC, 0x97,
		0x04, 0x04, 0x9A, 0x67, 0x2F, 0xDF, 0xEC, 0x0D, 0xB2, 0x5F, 0xB5, 0xB2, 0xBD, 0xB5, 0x4B, 0xDE,
		0x0E, 0x88, 0xA3, 0xBA, 0xD1, 0xB4, 0xE0, 0x91, 0x81, 0xA7, 0x84, 0xEB, 0x77, 0x85, 0x8B, 0xEF,
		0xA5, 0xE3, 0x27, 0xB2, 0xF2, 0x82, 0x2B, 0x29, 0xF1, 0x75, 0x2D, 0xCE, 0xCC, 0xAE, 0x9B, 0x8D,
		0xED, 0x5C, 0xF1, 0x8E, 0xDB, 0x9A, 0xD7, 0xAF, 0x42, 0x14, 0x52, 0xCD, 0xE3, 0xC5, 0xDD, 0xCE,
		0x08, 0x12, 0x17, 0xD0, 0x7F, 0x1A, 0xAA, 0x1F, 0x7D, 0xE0, 0x93, 0x54, 0xC8, 0xBC, 0x73, 0x8A,
		0xCB, 0xAD, 0x6E, 0x93, 0xE2, 0x19, 0x72, 0x6B, 0xD3, 0x45, 0xF8, 0x73, 0x3D, 0x2B, 0x6A, 0x55,
		0xD2, 0x3A, 0x8B, 0xB0, 0x8A, 0x42, 0xE3, 0x3D, 0xF1, 0x92, 0x23, 0x42, 0x2E, 0xBA, 0xCC, 0x9C,
		0x9A, 0xC1, 0xDD, 0x62, 0x86, 0x9C, 0x2E, 0xE1, 0x2D, 0x6F, 0x62, 0x67, 0x51, 0x08, 0x0E, 0xCF
	};

	static const pie::hac::detail::rsa2048_block_t kXciCertSignModulus = {
		0xCD, 0xF3, 0x2C, 0xB0, 0xF5, 0x14, 0x78, 0x34, 0xE5, 0x02, 0xD0, 0x29, 0x6A, 0xA5, 0xFD, 0x97,
		0x6A, 0xE0, 0xB0, 0xBB, 0xB0, 0x3B, 0x1A, 0x80, 0xB7, 0xD7, 0x58, 0x92, 0x79, 0x84, 0xC0, 0x36,
		0xB1, 0x55, 0x23, 0xD8, 0xA5, 0x60, 0x91, 0x26, 0x48, 0x1A, 0x80, 0x4A, 0xEA, 0x00, 0x98, 0x2A,
		0xEC, 0x52, 0x17, 0x72, 0x92, 0x4D, 0xF5, 0x42, 0xA7, 0x8A, 0x6F, 0x7F, 0xD2, 0x48, 0x51, 0x8E,
		0xDF, 0xCB, 0xBF, 0x77, 0xF6, 0x18, 0xBD, 0xE5, 0x00, 0xD9, 0x70, 0x8C, 0xEF, 0x57, 0xB2, 0x96,
		0xD0, 0x36, 0x83, 0x88, 0x9C, 0xC5, 0xFB, 0xA0, 0x33, 0x81, 0xA2, 0x12, 0x23, 0xC6, 0xC7, 0x86,
		0x0A, 0x98, 0x57, 0x4D, 0x2E, 0xB5, 0xAE, 0x64, 0xE4, 0x6F, 0xC2, 0xC5, 0xAC, 0x6A, 0x1D, 0xDB,
		0xA5, 0xAF, 0x12, 0x22, 0xAB, 0x1F, 0x51, 0xC8, 0x0E, 0x0D, 0xC9, 0xF5, 0x03, 0xE8, 0xD2, 0xFC,
		0x84, 0x62, 0x26, 0x55, 0xA4, 0xC3, 0xE2, 0xA8, 0x98, 0x05, 0x67, 0x23, 0xFD, 0xA5, 0x46, 0x40,
		0x78, 0x51, 0x09, 0x3D, 0x91, 0x74, 0xD6, 0xD0, 0x54, 0x23, 0x0D, 0xA0, 0xFB, 0x07, 0xD0, 0xAA,
		0x9D, 0x50, 0x4E, 0x2B, 0x26, 0x9A, 0x14, 0xE5, 0x6C, 0x73, 0x66, 0x24, 0x18, 0xA1, 0x93, 0x9C,
		0x2A, 0x40, 0x40, 0x05, 0x6B, 0xF1, 0x45, 0xDF, 0x22, 0x8B, 0x40, 0x61, 0xA4, 0x11, 0x06, 0x03,
		0xA5, 0x53, 0x84, 0xC0, 0x12, 0xE1, 0x88, 0x9D, 0x55, 0x55, 0x07, 0x40, 0x88, 0x01, 0x8C, 0xAB,
		0xA2, 0xFD, 0xFD, 0x19, 0x48, 0x25, 0xAB, 0x59, 0x59, 0x28, 0x63, 0x68, 0x69, 0x1B, 0x99, 0x73,
		0x8D, 0xAB, 0x5A, 0xFA, 0x71, 0x60, 0x1B, 0x12, 0xE7, 0x99, 0x70, 0xF1, 0x99, 0x2A, 0x50, 0x18,
		0x8B, 0x6B, 0x61, 0x90, 0xE2, 0x7E, 0x8B, 0x90, 0xD4, 0xD5, 0xC0, 0xCB, 0x7C, 0x08, 0x06, 0xD9
	};
	
	/* Keydata for very early beta NCA0 archives' RSA-OAEP. */
	/*
	static const pie::hac::detail::rsa2048_block_t beta_nca0_modulus = {
		0xAD, 0x58, 0xEE, 0x97, 0xF9, 0x47, 0x90, 0x7D, 0xF9, 0x29, 0x5F, 0x1F, 0x39, 0x68, 0xEE, 0x49,
		0x4C, 0x1E, 0x8D, 0x84, 0x91, 0x31, 0x5D, 0xE5, 0x96, 0x27, 0xB2, 0xB3, 0x59, 0x7B, 0xDE, 0xFD,
		0xB7, 0xEB, 0x40, 0xA1, 0xE7, 0xEB, 0xDC, 0x60, 0xD0, 0x3D, 0xC5, 0x50, 0x92, 0xAD, 0x3D, 0xC4,
		0x8C, 0x17, 0xD2, 0x37, 0x66, 0xE3, 0xF7, 0x14, 0x34, 0x38, 0x6B, 0xA7, 0x2B, 0x21, 0x10, 0x9B,
		0x73, 0x49, 0x15, 0xD9, 0x2A, 0x90, 0x86, 0x76, 0x81, 0x6A, 0x10, 0xBD, 0x74, 0xC4, 0x20, 0x55,
		0x25, 0xA8, 0x02, 0xC5, 0xA0, 0x34, 0x36, 0x7B, 0x66, 0x47, 0x2C, 0x7E, 0x47, 0x82, 0xA5, 0xD4,
		0xA3, 0x42, 0x45, 0xE8, 0xFD, 0x65, 0x72, 0x48, 0xA1, 0xB0, 0x44, 0x10, 0xEF, 0xAC, 0x1D, 0x0F,
		0xB5, 0x12, 0x19, 0xA8, 0x41, 0x0B, 0x76, 0x3B, 0xBC, 0xF1, 0x4A, 0x10, 0x46, 0x22, 0xB8, 0xF1,
		0xBC, 0x21, 0x81, 0x69, 0x9B, 0x63, 0x6F, 0xD7, 0xB9, 0x60, 0x2A, 0x9A, 0xE5, 0x2C, 0x47, 0x72,
		0x59, 0x65, 0xA2, 0x21, 0x60, 0xC4, 0xFC, 0xB0, 0xD7, 0x6F, 0x42, 0xC9, 0x0C, 0xF5, 0x76, 0x7D,
		0xF2, 0x5C, 0xE0, 0x80, 0x0F, 0xEE, 0x45, 0x7E, 0x4E, 0x3A, 0x8D, 0x9C, 0x5B, 0x5B, 0xD9, 0xD1,
		0x43, 0x94, 0x2C, 0xC7, 0x2E, 0xB9, 0x4A, 0xE5, 0x3E, 0x15, 0xDD, 0x43, 0x00, 0xF7, 0x78, 0xE7,
		0x7C, 0x39, 0xB0, 0x4D, 0xC5, 0xD1, 0x1C, 0xF2, 0xB4, 0x7A, 0x2A, 0xEA, 0x0A, 0x8E, 0xB9, 0x13,
		0xB4, 0x4F, 0xD7, 0x5B, 0x4D, 0x7B, 0x43, 0xB0, 0x3A, 0x9A, 0x60, 0x22, 0x47, 0x91, 0x78, 0xC7,
		0x10, 0x64, 0xE0, 0x2C, 0x69, 0xD1, 0x66, 0x3C, 0x42, 0x2E, 0xEF, 0x19, 0x21, 0x89, 0x8E, 0xE1,
		0xB0, 0xB4, 0xD0, 0x17, 0xA1, 0x0F, 0x73, 0x98, 0x5A, 0xF6, 0xEE, 0xC0, 0x2F, 0x9E, 0xCE, 0xC5
	};

	static const pie::hac::detail::sha256_hash_t beta_nca0_label_hash = {
		0xE3, 0xB0, 0xC4, 0x42, 0x98, 0xFC, 0x1C, 0x14, 0x9A, 0xFB, 0xF4, 0xC8, 0x99, 0x6F, 0xB9, 0x24,
		0x27, 0xAE, 0x41, 0xE4, 0x64, 0x9B, 0x93, 0x4C, 0xA4, 0x95, 0x99, 0x1B, 0x78, 0x52, 0xB8, 0x55
	};
	*/

	struct sRsaKeyForGeneration {
		byte_t generation;
		pie::hac::detail::rsa2048_block_t modulus;
	};

	static const pie::hac::detail::rsa2048_block_t kProdPackage2HeaderModulus = {
		0x8D, 0x13, 0xA7, 0x77, 0x6A, 0xE5, 0xDC, 0xC0, 0x3B, 0x25, 0xD0, 0x58, 0xE4, 0x20, 0x69, 0x59,
		0x55, 0x4B, 0xAB, 0x70, 0x40, 0x08, 0x28, 0x07, 0xA8, 0xA7, 0xFD, 0x0F, 0x31, 0x2E, 0x11, 0xFE,
		0x47, 0xA0, 0xF9, 0x9D, 0xDF, 0x80, 0xDB, 0x86, 0x5A, 0x27, 0x89, 0xCD, 0x97, 0x6C, 0x85, 0xC5,
		0x6C, 0x39, 0x7F, 0x41, 0xF2, 0xFF, 0x24, 0x20, 0xC3, 0x95, 0xA6, 0xF7, 0x9D, 0x4A, 0x45, 0x74,
		0x8B, 0x5D, 0x28, 0x8A, 0xC6, 0x99, 0x35, 0x68, 0x85, 0xA5, 0x64, 0x32, 0x80, 0x9F, 0xD3, 0x48,
		0x39, 0xA2, 0x1D, 0x24, 0x67, 0x69, 0xDF, 0x75, 0xAC, 0x12, 0xB5, 0xBD, 0xC3, 0x29, 0x90, 0xBE,
		0x37, 0xE4, 0xA0, 0x80, 0x9A, 0xBE, 0x36, 0xBF, 0x1F, 0x2C, 0xAB, 0x2B, 0xAD, 0xF5, 0x97, 0x32,
		0x9A, 0x42, 0x9D, 0x09, 0x8B, 0x08, 0xF0, 0x63, 0x47, 0xA3, 0xE9, 0x1B, 0x36, 0xD8, 0x2D, 0x8A,
		0xD7, 0xE1, 0x54, 0x11, 0x95, 0xE4, 0x45, 0x88, 0x69, 0x8A, 0x2B, 0x35, 0xCE, 0xD0, 0xA5, 0x0B,
		0xD5, 0x5D, 0xAC, 0xDB, 0xAF, 0x11, 0x4D, 0xCA, 0xB8, 0x1E, 0xE7, 0x01, 0x9E, 0xF4, 0x46, 0xA3,
		0x8A, 0x94, 0x6D, 0x76, 0xBD, 0x8A, 0xC8, 0x3B, 0xD2, 0x31, 0x58, 0x0C, 0x79, 0xA8, 0x26, 0xE9,
		0xD1, 0x79, 0x9C, 0xCB, 0xD4, 0x2B, 0x6A, 0x4F, 0xC6, 0xCC, 0xCF, 0x90, 0xA7, 0xB9, 0x98, 0x47,
		0xFD, 0xFA, 0x4C, 0x6C, 0x6F, 0x81, 0x87, 0x3B, 0xCA, 0xB8, 0x50, 0xF6, 0x3E, 0x39, 0x5D, 0x4D,
		0x97, 0x3F, 0x0F, 0x35, 0x39, 0x53, 0xFB, 0xFA, 0xCD, 0xAB, 0xA8, 0x7A, 0x62, 0x9A, 0x3F, 0xF2,
		0x09, 0x27, 0x96, 0x3F, 0x07, 0x9A, 0x91, 0xF7, 0x16, 0xBF, 0xC6, 0x3A, 0x82, 0x5A, 0x4B, 0xCF,
		0x49, 0x50, 0x95, 0x8C, 0x55, 0x80, 0x7E, 0x39, 0xB1, 0x48, 0x05, 0x1E, 0x21, 0xC7, 0x24, 0x4F
	};

	static const std::vector<sRsaKeyForGeneration> kProdNcaHeaderSign0Modulus = 
	{
		{
			0x00, 
			{0xBF, 0xBE, 0x40, 0x6C, 0xF4, 0xA7, 0x80, 0xE9, 0xF0, 0x7D, 0x0C, 0x99, 0x61, 0x1D, 0x77, 0x2F,
			0x96, 0xBC, 0x4B, 0x9E, 0x58, 0x38, 0x1B, 0x03, 0xAB, 0xB1, 0x75, 0x49, 0x9F, 0x2B, 0x4D, 0x58,
			0x34, 0xB0, 0x05, 0xA3, 0x75, 0x22, 0xBE, 0x1A, 0x3F, 0x03, 0x73, 0xAC, 0x70, 0x68, 0xD1, 0x16,
			0xB9, 0x04, 0x46, 0x5E, 0xB7, 0x07, 0x91, 0x2F, 0x07, 0x8B, 0x26, 0xDE, 0xF6, 0x00, 0x07, 0xB2,
			0xB4, 0x51, 0xF8, 0x0D, 0x0A, 0x5E, 0x58, 0xAD, 0xEB, 0xBC, 0x9A, 0xD6, 0x49, 0xB9, 0x64, 0xEF,
			0xA7, 0x82, 0xB5, 0xCF, 0x6D, 0x70, 0x13, 0xB0, 0x0F, 0x85, 0xF6, 0xA9, 0x08, 0xAA, 0x4D, 0x67,
			0x66, 0x87, 0xFA, 0x89, 0xFF, 0x75, 0x90, 0x18, 0x1E, 0x6B, 0x3D, 0xE9, 0x8A, 0x68, 0xC9, 0x26,
			0x04, 0xD9, 0x80, 0xCE, 0x3F, 0x5E, 0x92, 0xCE, 0x01, 0xFF, 0x06, 0x3B, 0xF2, 0xC1, 0xA9, 0x0C,
			0xCE, 0x02, 0x6F, 0x16, 0xBC, 0x92, 0x42, 0x0A, 0x41, 0x64, 0xCD, 0x52, 0xB6, 0x34, 0x4D, 0xAE,
			0xC0, 0x2E, 0xDE, 0xA4, 0xDF, 0x27, 0x68, 0x3C, 0xC1, 0xA0, 0x60, 0xAD, 0x43, 0xF3, 0xFC, 0x86,
			0xC1, 0x3E, 0x6C, 0x46, 0xF7, 0x7C, 0x29, 0x9F, 0xFA, 0xFD, 0xF0, 0xE3, 0xCE, 0x64, 0xE7, 0x35,
			0xF2, 0xF6, 0x56, 0x56, 0x6F, 0x6D, 0xF1, 0xE2, 0x42, 0xB0, 0x83, 0x40, 0xA5, 0xC3, 0x20, 0x2B,
			0xCC, 0x9A, 0xAE, 0xCA, 0xED, 0x4D, 0x70, 0x30, 0xA8, 0x70, 0x1C, 0x70, 0xFD, 0x13, 0x63, 0x29,
			0x02, 0x79, 0xEA, 0xD2, 0xA7, 0xAF, 0x35, 0x28, 0x32, 0x1C, 0x7B, 0xE6, 0x2F, 0x1A, 0xAA, 0x40,
			0x7E, 0x32, 0x8C, 0x27, 0x42, 0xFE, 0x82, 0x78, 0xEC, 0x0D, 0xEB, 0xE6, 0x83, 0x4B, 0x6D, 0x81,
			0x04, 0x40, 0x1A, 0x9E, 0x9A, 0x67, 0xF6, 0x72, 0x29, 0xFA, 0x04, 0xF0, 0x9D, 0xE4, 0xF4, 0x03,}
		},
		{
			0x01, 
			{0xAD, 0xE3, 0xE1, 0xFA, 0x04, 0x35, 0xE5, 0xB6, 0xDD, 0x49, 0xEA, 0x89, 0x29, 0xB1, 0xFF, 0xB6,
			0x43, 0xDF, 0xCA, 0x96, 0xA0, 0x4A, 0x13, 0xDF, 0x43, 0xD9, 0x94, 0x97, 0x96, 0x43, 0x65, 0x48,
			0x70, 0x58, 0x33, 0xA2, 0x7D, 0x35, 0x7B, 0x96, 0x74, 0x5E, 0x0B, 0x5C, 0x32, 0x18, 0x14, 0x24,
			0xC2, 0x58, 0xB3, 0x6C, 0x22, 0x7A, 0xA1, 0xB7, 0xCB, 0x90, 0xA7, 0xA3, 0xF9, 0x7D, 0x45, 0x16,
			0xA5, 0xC8, 0xED, 0x8F, 0xAD, 0x39, 0x5E, 0x9E, 0x4B, 0x51, 0x68, 0x7D, 0xF8, 0x0C, 0x35, 0xC6,
			0x3F, 0x91, 0xAE, 0x44, 0xA5, 0x92, 0x30, 0x0D, 0x46, 0xF8, 0x40, 0xFF, 0xD0, 0xFF, 0x06, 0xD2,
			0x1C, 0x7F, 0x96, 0x18, 0xDC, 0xB7, 0x1D, 0x66, 0x3E, 0xD1, 0x73, 0xBC, 0x15, 0x8A, 0x2F, 0x94,
			0xF3, 0x00, 0xC1, 0x83, 0xF1, 0xCD, 0xD7, 0x81, 0x88, 0xAB, 0xDF, 0x8C, 0xEF, 0x97, 0xDD, 0x1B,
			0x17, 0x5F, 0x58, 0xF6, 0x9A, 0xE9, 0xE8, 0xC2, 0x2F, 0x38, 0x15, 0xF5, 0x21, 0x07, 0xF8, 0x37,
			0x90, 0x5D, 0x2E, 0x02, 0x40, 0x24, 0x15, 0x0D, 0x25, 0xB7, 0x26, 0x5D, 0x09, 0xCC, 0x4C, 0xF4,
			0xF2, 0x1B, 0x94, 0x70, 0x5A, 0x9E, 0xEE, 0xED, 0x77, 0x77, 0xD4, 0x51, 0x99, 0xF5, 0xDC, 0x76,
			0x1E, 0xE3, 0x6C, 0x8C, 0xD1, 0x12, 0xD4, 0x57, 0xD1, 0xB6, 0x83, 0xE4, 0xE4, 0xFE, 0xDA, 0xE9,
			0xB4, 0x3B, 0x33, 0xE5, 0x37, 0x8A, 0xDF, 0xB5, 0x7F, 0x89, 0xF1, 0x9B, 0x9E, 0xB0, 0x15, 0xB2,
			0x3A, 0xFE, 0xEA, 0x61, 0x84, 0x5B, 0x7D, 0x4B, 0x23, 0x12, 0x0B, 0x83, 0x12, 0xF2, 0x22, 0x6B,
			0xB9, 0x22, 0x96, 0x4B, 0x26, 0x0B, 0x63, 0x5E, 0x96, 0x57, 0x52, 0xA3, 0x67, 0x64, 0x22, 0xCA,
			0xD0, 0x56, 0x3E, 0x74, 0xB5, 0x98, 0x1F, 0x0D, 0xF8, 0xB3, 0x34, 0xE6, 0x98, 0x68, 0x5A, 0xAD,}
		},
	};

	static const std::vector<sRsaKeyForGeneration> kProdAcidSignModulus = 
	{
		{
			0x00, 
			{0xDD, 0xC8, 0xDD, 0xF2, 0x4E, 0x6D, 0xF0, 0xCA, 0x9E, 0xC7, 0x5D, 0xC7, 0x7B, 0xAD, 0xFE, 0x7D,
			0x23, 0x89, 0x69, 0xB6, 0xF2, 0x06, 0xA2, 0x02, 0x88, 0xE1, 0x55, 0x91, 0xAB, 0xCB, 0x4D, 0x50,
			0x2E, 0xFC, 0x9D, 0x94, 0x76, 0xD6, 0x4C, 0xD8, 0xFF, 0x10, 0xFA, 0x5E, 0x93, 0x0A, 0xB4, 0x57,
			0xAC, 0x51, 0xC7, 0x16, 0x66, 0xF4, 0x1A, 0x54, 0xC2, 0xC5, 0x04, 0x3D, 0x1B, 0xFE, 0x30, 0x20,
			0x8A, 0xAC, 0x6F, 0x6F, 0xF5, 0xC7, 0xB6, 0x68, 0xB8, 0xC9, 0x40, 0x6B, 0x42, 0xAD, 0x11, 0x21,
			0xE7, 0x8B, 0xE9, 0x75, 0x01, 0x86, 0xE4, 0x48, 0x9B, 0x0A, 0x0A, 0xF8, 0x7F, 0xE8, 0x87, 0xF2,
			0x82, 0x01, 0xE6, 0xA3, 0x0F, 0xE4, 0x66, 0xAE, 0x83, 0x3F, 0x4E, 0x9F, 0x5E, 0x01, 0x30, 0xA4,
			0x00, 0xB9, 0x9A, 0xAE, 0x5F, 0x03, 0xCC, 0x18, 0x60, 0xE5, 0xEF, 0x3B, 0x5E, 0x15, 0x16, 0xFE,
			0x1C, 0x82, 0x78, 0xB5, 0x2F, 0x47, 0x7C, 0x06, 0x66, 0x88, 0x5D, 0x35, 0xA2, 0x67, 0x20, 0x10,
			0xE7, 0x6C, 0x43, 0x68, 0xD3, 0xE4, 0x5A, 0x68, 0x2A, 0x5A, 0xE2, 0x6D, 0x73, 0xB0, 0x31, 0x53,
			0x1C, 0x20, 0x09, 0x44, 0xF5, 0x1A, 0x9D, 0x22, 0xBE, 0x12, 0xA1, 0x77, 0x11, 0xE2, 0xA1, 0xCD,
			0x40, 0x9A, 0xA2, 0x8B, 0x60, 0x9B, 0xEF, 0xA0, 0xD3, 0x48, 0x63, 0xA2, 0xF8, 0xA3, 0x2C, 0x08,
			0x56, 0x52, 0x2E, 0x60, 0x19, 0x67, 0x5A, 0xA7, 0x9F, 0xDC, 0x3F, 0x3F, 0x69, 0x2B, 0x31, 0x6A,
			0xB7, 0x88, 0x4A, 0x14, 0x84, 0x80, 0x33, 0x3C, 0x9D, 0x44, 0xB7, 0x3F, 0x4C, 0xE1, 0x75, 0xEA,
			0x37, 0xEA, 0xE8, 0x1E, 0x7C, 0x77, 0xB7, 0xC6, 0x1A, 0xA2, 0xF0, 0x9F, 0x10, 0x61, 0xCD, 0x7B,
			0x5B, 0x32, 0x4C, 0x37, 0xEF, 0xB1, 0x71, 0x68, 0x53, 0x0A, 0xED, 0x51, 0x7D, 0x35, 0x22, 0xFD,}
		},
		{
			0x01, 
			{0xE7, 0xAA, 0x25, 0xC8, 0x01, 0xA5, 0x14, 0x6B, 0x01, 0x60, 0x3E, 0xD9, 0x96, 0x5A, 0xBF, 0x90,
			0xAC, 0xA7, 0xFD, 0x9B, 0x5B, 0xBD, 0x8A, 0x26, 0xB0, 0xCB, 0x20, 0x28, 0x9A, 0x72, 0x12, 0xF5,
			0x20, 0x65, 0xB3, 0xB9, 0x84, 0x58, 0x1F, 0x27, 0xBC, 0x7C, 0xA2, 0xC9, 0x9E, 0x18, 0x95, 0xCF,
			0xC2, 0x73, 0x2E, 0x74, 0x8C, 0x66, 0xE5, 0x9E, 0x79, 0x2B, 0xB8, 0x07, 0x0C, 0xB0, 0x4E, 0x8E,
			0xAB, 0x85, 0x21, 0x42, 0xC4, 0xC5, 0x6D, 0x88, 0x9C, 0xDB, 0x15, 0x95, 0x3F, 0x80, 0xDB, 0x7A,
			0x9A, 0x7D, 0x41, 0x56, 0x25, 0x17, 0x18, 0x42, 0x4D, 0x8C, 0xAC, 0xA5, 0x7B, 0xDB, 0x42, 0x5D,
			0x59, 0x35, 0x45, 0x5D, 0x8A, 0x02, 0xB5, 0x70, 0xC0, 0x72, 0x35, 0x46, 0xD0, 0x1D, 0x60, 0x01,
			0x4A, 0xCC, 0x1C, 0x46, 0xD3, 0xD6, 0x35, 0x52, 0xD6, 0xE1, 0xF8, 0x3B, 0x5D, 0xEA, 0xDD, 0xB8,
			0xFE, 0x7D, 0x50, 0xCB, 0x35, 0x23, 0x67, 0x8B, 0xB6, 0xE4, 0x74, 0xD2, 0x60, 0xFC, 0xFD, 0x43,
			0xBF, 0x91, 0x08, 0x81, 0xC5, 0x4F, 0x5D, 0x16, 0x9A, 0xC4, 0x9A, 0xC6, 0xF6, 0xF3, 0xE1, 0xF6,
			0x5C, 0x07, 0xAA, 0x71, 0x6C, 0x13, 0xA4, 0xB1, 0xB3, 0x66, 0xBF, 0x90, 0x4C, 0x3D, 0xA2, 0xC4,
			0x0B, 0xB8, 0x3D, 0x7A, 0x8C, 0x19, 0xFA, 0xFF, 0x6B, 0xB9, 0x1F, 0x02, 0xCC, 0xB6, 0xD3, 0x0C,
			0x7D, 0x19, 0x1F, 0x47, 0xF9, 0xC7, 0x40, 0x01, 0xFA, 0x46, 0xEA, 0x0B, 0xD4, 0x02, 0xE0, 0x3D,
			0x30, 0x9A, 0x1A, 0x0F, 0xEA, 0xA7, 0x66, 0x55, 0xF7, 0xCB, 0x28, 0xE2, 0xBB, 0x99, 0xE4, 0x83,
			0xC3, 0x43, 0x03, 0xEE, 0xDC, 0x1F, 0x02, 0x23, 0xDD, 0xD1, 0x2D, 0x39, 0xA4, 0x65, 0x75, 0x03,
			0xEF, 0x37, 0x9C, 0x06, 0xD6, 0xFA, 0xA1, 0x15, 0xF0, 0xDB, 0x17, 0x47, 0x26, 0x4F, 0x49, 0x03}
		},
	};

	static const pie::hac::detail::rsa2048_block_t kDevPackage2HeaderModulus = {
		0xB3, 0x65, 0x54, 0xFB, 0x0A, 0xB0, 0x1E, 0x85, 0xA7, 0xF6, 0xCF, 0x91, 0x8E, 0xBA, 0x96, 0x99,
		0x0D, 0x8B, 0x91, 0x69, 0x2A, 0xEE, 0x01, 0x20, 0x4F, 0x34, 0x5C, 0x2C, 0x4F, 0x4E, 0x37, 0xC7,
		0xF1, 0x0B, 0xD4, 0xCD, 0xA1, 0x7F, 0x93, 0xF1, 0x33, 0x59, 0xCE, 0xB1, 0xE9, 0xDD, 0x26, 0xE6,
		0xF3, 0xBB, 0x77, 0x87, 0x46, 0x7A, 0xD6, 0x4E, 0x47, 0x4A, 0xD1, 0x41, 0xB7, 0x79, 0x4A, 0x38,
		0x06, 0x6E, 0xCF, 0x61, 0x8F, 0xCD, 0xC1, 0x40, 0x0B, 0xFA, 0x26, 0xDC, 0xC0, 0x34, 0x51, 0x83,
		0xD9, 0x3B, 0x11, 0x54, 0x3B, 0x96, 0x27, 0x32, 0x9A, 0x95, 0xBE, 0x1E, 0x68, 0x11, 0x50, 0xA0,
		0x6B, 0x10, 0xA8, 0x83, 0x8B, 0xF5, 0xFC, 0xBC, 0x90, 0x84, 0x7A, 0x5A, 0x5C, 0x43, 0x52, 0xE6,
		0xC8, 0x26, 0xE9, 0xFE, 0x06, 0xA0, 0x8B, 0x53, 0x0F, 0xAF, 0x1E, 0xC4, 0x1C, 0x0B, 0xCF, 0x50,
		0x1A, 0xA4, 0xF3, 0x5C, 0xFB, 0xF0, 0x97, 0xE4, 0xDE, 0x32, 0x0A, 0x9F, 0xE3, 0x5A, 0xAA, 0xB7,
		0x44, 0x7F, 0x5C, 0x33, 0x60, 0xB9, 0x0F, 0x22, 0x2D, 0x33, 0x2A, 0xE9, 0x69, 0x79, 0x31, 0x42,
		0x8F, 0xE4, 0x3A, 0x13, 0x8B, 0xE7, 0x26, 0xBD, 0x08, 0x87, 0x6C, 0xA6, 0xF2, 0x73, 0xF6, 0x8E,
		0xA7, 0xF2, 0xFE, 0xFB, 0x6C, 0x28, 0x66, 0x0D, 0xBD, 0xD7, 0xEB, 0x42, 0xA8, 0x78, 0xE6, 0xB8,
		0x6B, 0xAE, 0xC7, 0xA9, 0xE2, 0x40, 0x6E, 0x89, 0x20, 0x82, 0x25, 0x8E, 0x3C, 0x6A, 0x60, 0xD7,
		0xF3, 0x56, 0x8E, 0xEC, 0x8D, 0x51, 0x8A, 0x63, 0x3C, 0x04, 0x78, 0x23, 0x0E, 0x90, 0x0C, 0xB4,
		0xE7, 0x86, 0x3B, 0x4F, 0x8E, 0x13, 0x09, 0x47, 0x32, 0x0E, 0x04, 0xB8, 0x4D, 0x5B, 0xB0, 0x46,
		0x71, 0xB0, 0x5C, 0xF4, 0xAD, 0x63, 0x4F, 0xC5, 0xE2, 0xAC, 0x1E, 0xC4, 0x33, 0x96, 0x09, 0x7B
	};

	static const std::vector<sRsaKeyForGeneration> kDevNcaHeaderSign0Modulus = 
	{
		{
			0x00, 
			{0xD8, 0xF1, 0x18, 0xEF, 0x32, 0x72, 0x4C, 0xA7, 0x47, 0x4C, 0xB9, 0xEA, 0xB3, 0x04, 0xA8, 0xA4,
			0xAC, 0x99, 0x08, 0x08, 0x04, 0xBF, 0x68, 0x57, 0xB8, 0x43, 0x94, 0x2B, 0xC7, 0xB9, 0x66, 0x49,
			0x85, 0xE5, 0x8A, 0x9B, 0xC1, 0x00, 0x9A, 0x6A, 0x8D, 0xD0, 0xEF, 0xCE, 0xFF, 0x86, 0xC8, 0x5C,
			0x5D, 0xE9, 0x53, 0x7B, 0x19, 0x2A, 0xA8, 0xC0, 0x22, 0xD1, 0xF3, 0x22, 0x0A, 0x50, 0xF2, 0x2B,
			0x65, 0x05, 0x1B, 0x9E, 0xEC, 0x61, 0xB5, 0x63, 0xA3, 0x6F, 0x3B, 0xBA, 0x63, 0x3A, 0x53, 0xF4,
			0x49, 0x2F, 0xCF, 0x03, 0xCC, 0xD7, 0x50, 0x82, 0x1B, 0x29, 0x4F, 0x08, 0xDE, 0x1B, 0x6D, 0x47,
			0x4F, 0xA8, 0xB6, 0x6A, 0x26, 0xA0, 0x83, 0x3F, 0x1A, 0xAF, 0x83, 0x8F, 0x0E, 0x17, 0x3F, 0xFE,
			0x44, 0x1C, 0x56, 0x94, 0x2E, 0x49, 0x83, 0x83, 0x03, 0xE9, 0xB6, 0xAD, 0xD5, 0xDE, 0xE3, 0x2D,
			0xA1, 0xD9, 0x66, 0x20, 0x5D, 0x1F, 0x5E, 0x96, 0x5D, 0x5B, 0x55, 0x0D, 0xD4, 0xB4, 0x77, 0x6E,
			0xAE, 0x1B, 0x69, 0xF3, 0xA6, 0x61, 0x0E, 0x51, 0x62, 0x39, 0x28, 0x63, 0x75, 0x76, 0xBF, 0xB0,
			0xD2, 0x22, 0xEF, 0x98, 0x25, 0x02, 0x05, 0xC0, 0xD7, 0x6A, 0x06, 0x2C, 0xA5, 0xD8, 0x5A, 0x9D,
			0x7A, 0xA4, 0x21, 0x55, 0x9F, 0xF9, 0x3E, 0xBF, 0x16, 0xF6, 0x07, 0xC2, 0xB9, 0x6E, 0x87, 0x9E,
			0xB5, 0x1C, 0xBE, 0x97, 0xFA, 0x82, 0x7E, 0xED, 0x30, 0xD4, 0x66, 0x3F, 0xDE, 0xD8, 0x1B, 0x4B,
			0x15, 0xD9, 0xFB, 0x2F, 0x50, 0xF0, 0x9D, 0x1D, 0x52, 0x4C, 0x1C, 0x4D, 0x8D, 0xAE, 0x85, 0x1E,
			0xEA, 0x7F, 0x86, 0xF3, 0x0B, 0x7B, 0x87, 0x81, 0x98, 0x23, 0x80, 0x63, 0x4F, 0x2F, 0xB0, 0x62,
			0xCC, 0x6E, 0xD2, 0x46, 0x13, 0x65, 0x2B, 0xD6, 0x44, 0x33, 0x59, 0xB5, 0x8F, 0xB9, 0x4A, 0xA9,}
		},
		{
			0x01, 
			{0x9A, 0xBC, 0x88, 0xBD, 0x0A, 0xBE, 0xD7, 0x0C, 0x9B, 0x42, 0x75, 0x65, 0x38, 0x5E, 0xD1, 0x01,
			0xCD, 0x12, 0xAE, 0xEA, 0xE9, 0x4B, 0xDB, 0xB4, 0x5E, 0x36, 0x10, 0x96, 0xDA, 0x3D, 0x2E, 0x66,
			0xD3, 0x99, 0x13, 0x8A, 0xBE, 0x67, 0x41, 0xC8, 0x93, 0xD9, 0x3E, 0x42, 0xCE, 0x34, 0xCE, 0x96,
			0xFA, 0x0B, 0x23, 0xCC, 0x2C, 0xDF, 0x07, 0x3F, 0x3B, 0x24, 0x4B, 0x12, 0x67, 0x3A, 0x29, 0x36,
			0xA3, 0xAA, 0x06, 0xF0, 0x65, 0xA5, 0x85, 0xBA, 0xFD, 0x12, 0xEC, 0xF1, 0x60, 0x67, 0xF0, 0x8F,
			0xD3, 0x5B, 0x01, 0x1B, 0x1E, 0x84, 0xA3, 0x5C, 0x65, 0x36, 0xF9, 0x23, 0x7E, 0xF3, 0x26, 0x38,
			0x64, 0x98, 0xBA, 0xE4, 0x19, 0x91, 0x4C, 0x02, 0xCF, 0xC9, 0x6D, 0x86, 0xEC, 0x1D, 0x41, 0x69,
			0xDD, 0x56, 0xEA, 0x5C, 0xA3, 0x2A, 0x58, 0xB4, 0x39, 0xCC, 0x40, 0x31, 0xFD, 0xFB, 0x42, 0x74,
			0xF8, 0xEC, 0xEA, 0x00, 0xF0, 0xD9, 0x28, 0xEA, 0xFA, 0x2D, 0x00, 0xE1, 0x43, 0x53, 0xC6, 0x32,
			0xF4, 0xA2, 0x07, 0xD4, 0x5F, 0xD4, 0xCB, 0xAC, 0xCA, 0xFF, 0xDF, 0x84, 0xD2, 0x86, 0x14, 0x3C,
			0xDE, 0x22, 0x75, 0xA5, 0x73, 0xFF, 0x68, 0x07, 0x4A, 0xF9, 0x7C, 0x2C, 0xCC, 0xDE, 0x45, 0xB6,
			0x54, 0x82, 0x90, 0x36, 0x1F, 0x2C, 0x51, 0x96, 0xC5, 0x0A, 0x53, 0x5B, 0xF0, 0x8B, 0x4A, 0xAA,
			0x3B, 0x68, 0x97, 0x19, 0x17, 0x1F, 0x01, 0xB8, 0xED, 0xB9, 0x9A, 0x5E, 0x08, 0xC5, 0x20, 0x1E,
			0x6A, 0x09, 0xF0, 0xE9, 0x73, 0xA3, 0xBE, 0x10, 0x06, 0x02, 0xE9, 0xFB, 0x85, 0xFA, 0x5F, 0x01,
			0xAC, 0x60, 0xE0, 0xED, 0x7D, 0xB9, 0x49, 0xA8, 0x9E, 0x98, 0x7D, 0x91, 0x40, 0x05, 0xCF, 0xF9,
			0x1A, 0xFC, 0x40, 0x22, 0xA8, 0x96, 0x5B, 0xB0, 0xDC, 0x7A, 0xF5, 0xB7, 0xE9, 0x91, 0x4C, 0x49,}
		},
	};

	static const std::vector<sRsaKeyForGeneration> kDevAcidSignModulus = 
	{
		{
			0x00, 
			{0xD6, 0x34, 0xA5, 0x78, 0x6C, 0x68, 0xCE, 0x5A, 0xC2, 0x37, 0x17, 0xF3, 0x82, 0x45, 0xC6, 0x89,
			0xE1, 0x2D, 0x06, 0x67, 0xBF, 0xB4, 0x06, 0x19, 0x55, 0x6B, 0x27, 0x66, 0x0C, 0xA4, 0xB5, 0x87,
			0x81, 0x25, 0xF4, 0x30, 0xBC, 0x53, 0x08, 0x68, 0xA2, 0x48, 0x49, 0x8C, 0x3F, 0x38, 0x40, 0x9C,
			0xC4, 0x26, 0xF4, 0x79, 0xE2, 0xA1, 0x85, 0xF5, 0x5C, 0x7F, 0x58, 0xBA, 0xA6, 0x1C, 0xA0, 0x8B,
			0x84, 0x16, 0x14, 0x6F, 0x85, 0xD9, 0x7C, 0xE1, 0x3C, 0x67, 0x22, 0x1E, 0xFB, 0xD8, 0xA7, 0xA5,
			0x9A, 0xBF, 0xEC, 0x0E, 0xCF, 0x96, 0x7E, 0x85, 0xC2, 0x1D, 0x49, 0x5D, 0x54, 0x26, 0xCB, 0x32,
			0x7C, 0xF6, 0xBB, 0x58, 0x03, 0x80, 0x2B, 0x5D, 0xF7, 0xFB, 0xD1, 0x9D, 0xC7, 0xC6, 0x2E, 0x53,
			0xC0, 0x6F, 0x39, 0x2C, 0x1F, 0xA9, 0x92, 0xF2, 0x4D, 0x7D, 0x4E, 0x74, 0xFF, 0xE4, 0xEF, 0xE4,
			0x7C, 0x3D, 0x34, 0x2A, 0x71, 0xA4, 0x97, 0x59, 0xFF, 0x4F, 0xA2, 0xF4, 0x66, 0x78, 0xD8, 0xBA,
			0x99, 0xE3, 0xE6, 0xDB, 0x54, 0xB9, 0xE9, 0x54, 0xA1, 0x70, 0xFC, 0x05, 0x1F, 0x11, 0x67, 0x4B,
			0x26, 0x8C, 0x0C, 0x3E, 0x03, 0xD2, 0xA3, 0x55, 0x5C, 0x7D, 0xC0, 0x5D, 0x9D, 0xFF, 0x13, 0x2F,
			0xFD, 0x19, 0xBF, 0xED, 0x44, 0xC3, 0x8C, 0xA7, 0x28, 0xCB, 0xE5, 0xE0, 0xB1, 0xA7, 0x9C, 0x33,
			0x8D, 0xB8, 0x6E, 0xDE, 0x87, 0x18, 0x22, 0x60, 0xC4, 0xAE, 0xF2, 0x87, 0x9F, 0xCE, 0x09, 0x5C,
			0xB5, 0x99, 0xA5, 0x9F, 0x49, 0xF2, 0xD7, 0x58, 0xFA, 0xF9, 0xC0, 0x25, 0x7D, 0xD6, 0xCB, 0xF3,
			0xD8, 0x6C, 0xA2, 0x69, 0x91, 0x68, 0x73, 0xB1, 0x94, 0x6F, 0xA3, 0xF3, 0xB9, 0x7D, 0xF8, 0xE0,
			0x72, 0x9E, 0x93, 0x7B, 0x7A, 0xA2, 0x57, 0x60, 0xB7, 0x5B, 0xA9, 0x84, 0xAE, 0x64, 0x88, 0x69}
		},
		{
			0x01, 
			{0xBC, 0xA5, 0x6A, 0x7E, 0xEA, 0x38, 0x34, 0x62, 0xA6, 0x10, 0x18, 0x3C, 0xE1, 0x63, 0x7B, 0xF0,
			0xD3, 0x08, 0x8C, 0xF5, 0xC5, 0xC4, 0xC7, 0x93, 0xE9, 0xD9, 0xE6, 0x32, 0xF3, 0xA0, 0xF6, 0x6E,
			0x8A, 0x98, 0x76, 0x47, 0x33, 0x47, 0x65, 0x02, 0x70, 0xDC, 0x86, 0x5F, 0x3D, 0x61, 0x5A, 0x70,
			0xBC, 0x5A, 0xCA, 0xCA, 0x50, 0xAD, 0x61, 0x7E, 0xC9, 0xEC, 0x27, 0xFF, 0xE8, 0x64, 0x42, 0x9A,
			0xEE, 0xBE, 0xC3, 0xD1, 0x0B, 0xC0, 0xE9, 0xBF, 0x83, 0x8D, 0xC0, 0x0C, 0xD8, 0x00, 0x5B, 0x76,
			0x90, 0xD2, 0x4B, 0x30, 0x84, 0x35, 0x8B, 0x1E, 0x20, 0xB7, 0xE4, 0xDC, 0x63, 0xE5, 0xDF, 0xCD,
			0x00, 0x5F, 0x81, 0x5F, 0x67, 0xC5, 0x8B, 0xDF, 0xFC, 0xE1, 0x37, 0x5F, 0x07, 0xD9, 0xDE, 0x4F,
			0xE6, 0x7B, 0xF1, 0xFB, 0xA1, 0x5A, 0x71, 0x40, 0xFE, 0xBA, 0x1E, 0xAE, 0x13, 0x22, 0xD2, 0xFE,
			0x37, 0xA2, 0xB6, 0x8B, 0xAB, 0xEB, 0x84, 0x81, 0x4E, 0x7C, 0x1E, 0x02, 0xD1, 0xFB, 0xD7, 0x5D,
			0x11, 0x84, 0x64, 0xD2, 0x4D, 0xBB, 0x50, 0x00, 0x67, 0x54, 0xE2, 0x77, 0x89, 0xBA, 0x0B, 0xE7,
			0x05, 0x57, 0x9A, 0x22, 0x5A, 0xEC, 0x76, 0x1C, 0xFD, 0xE8, 0xA8, 0x18, 0x16, 0x41, 0x65, 0x03,
			0xFA, 0xC4, 0xA6, 0x31, 0x5C, 0x1A, 0x7F, 0xAB, 0x11, 0xC8, 0x4A, 0x99, 0xB9, 0xE6, 0xCF, 0x62,
			0x21, 0xA6, 0x72, 0x47, 0xDB, 0xBA, 0x96, 0x26, 0x4E, 0x2E, 0xD4, 0x8C, 0x46, 0xD6, 0xA7, 0x1A,
			0x6C, 0x32, 0xA7, 0xDF, 0x85, 0x1C, 0x03, 0xC3, 0x6D, 0xA9, 0xE9, 0x68, 0xF4, 0x17, 0x1E, 0xB2,
			0x70, 0x2A, 0xA1, 0xE5, 0xE1, 0xF3, 0x8F, 0x6F, 0x63, 0xAC, 0xEB, 0x72, 0x0B, 0x4C, 0x4A, 0x36,
			0x3C, 0x60, 0x91, 0x9F, 0x6E, 0x1C, 0x71, 0xEA, 0xD0, 0x78, 0x78, 0xA0, 0x2E, 0xC6, 0x32, 0x6B}
		},
	};

	struct sBroadOnRsaKeyAndCert
	{
		std::string issuer;
		pie::hac::es::sign::SignatureAlgo key_type;
		tc::ByteData modulus;
		tc::ByteData certificate;
	};

	static const std::vector<sBroadOnRsaKeyAndCert> kProdBroadOnRsaKeyAndCert = 
	{
		{
			"Root",
			pie::hac::es::sign::SIGN_ALGO_RSA4096,
			tc::cli::FormatUtil::hexStringToBytes("F8246C58BAE7500301FBB7C2EBE0010571DA922378F0514EC0031DD0D21ED3D07EFC852069B5DE9BB951A8BC90A244926D379295AE9436AAA6A302510C7B1DEDD5FB20869D7F3016F6BE65D383A16DB3321B95351890B17002937EE193F57E99A2474E9D3824C7AEE38541F567E7518C7A0E38E7EBAF41191BCFF17B42A6B4EDE6CE8DE7318F7F5204B3990E226745AFD485B24493008B08C7F6B7E56B02B3E8FE0C9D859CB8B68223B8AB27EE5F6538078B2DB91E2A153E85818072A23B6DD93281054F6FB0F6F5AD283ECA0B7AF35455E03DA7B68326F3EC834AF314048AC6DF20D28508673CAB62A2C7BC131A533E0B66806B1C30664B372331BDC4B0CAD8D11EE7BBD9285548AAEC1F66E821B3C8A0476900C5E688E80CCE3C61D69CBBA137C6604F7A72DD8C7B3E3D51290DAA6A597B081F9D3633A3467A356109ACA7DD7D2E2FB2C1AEB8E20F4892D8B9F8B46F4E3C11F4F47D8B757DFEFEA3899C33595C5EFDEBCBABE8413E3A9A803C69356EB2B2AD5CC4C858455EF5F7B30644B47C64068CDF809F76025A2DB446E03D7CF62F34E702457B02A4CF5D9DD53CA53A7CA629788C67CA08BFECCA43A957AD16C94E1CD875CA107DCE7E0118F0DF6BFEE51DDBD991C26E60CD4858AA592C820075F29F526C917C6FE5403EA7D4A50CEC3B7384DE886E82D2EB4D4E42B5F2B149A81EA7CE7144DC2994CFC44E1F91CBD495"),
			tc::ByteData()
		},
		{
			"Root-CA00000003",
			pie::hac::es::sign::SIGN_ALGO_RSA2048,
			tc::cli::FormatUtil::hexStringToBytes("B279C9E2EEE121C6EAF44FF639F88F078B4B77ED9F9560B0358281B50E55AB721115A177703C7A30FE3AE9EF1C60BC1D974676B23A68CC04B198525BC968F11DE2DB50E4D9E7F071E562DAE2092233E9D363F61DD7C19FF3A4A91E8F6553D471DD7B84B9F1B8CE7335F0F5540563A1EAB83963E09BE901011F99546361287020E9CC0DAB487F140D6626A1836D27111F2068DE4772149151CF69C61BA60EF9D949A0F71F5499F2D39AD28C7005348293C431FFBD33F6BCA60DC7195EA2BCC56D200BAF6D06D09C41DB8DE9C720154CA4832B69C08C69CD3B073A0063602F462D338061A5EA6C915CD5623579C3EB64CE44EF586D14BAAA8834019B3EEBEED379"),
			tc::cli::FormatUtil::hexStringToBytes("00010003704138EFBBBDA16A987DD901326D1C9459484C88A2861B91A312587AE70EF6237EC50E1032DC39DDE89A96A8E859D76A98A6E7E36A0CFE352CA893058234FF833FCB3B03811E9F0DC0D9A52F8045B4B2F9411B67A51C44B5EF8CE77BD6D56BA75734A1856DE6D4BED6D3A242C7C8791B3422375E5C779ABF072F7695EFA0F75BCB83789FC30E3FE4CC8392207840638949C7F688565F649B74D63D8D58FFADDA571E9554426B1318FC468983D4C8A5628B06B6FC5D507C13E7A18AC1511EB6D62EA5448F83501447A9AFB3ECC2903C9DD52F922AC9ACDBEF58C6021848D96E208732D3D1D9D9EA440D91621C7A99DB8843C59C1F2E2C7D9B577D512C166D6F7E1AAD4A774A37447E78FE2021E14A95D112A068ADA019F463C7A55685AABB6888B9246483D18B9C806F474918331782344A4B8531334B26303263D9D2EB4F4BB99602B352F6AE4046C69A5E7E8E4A18EF9BC0A2DED61310417012FD824CC116CFB7C4C1F7EC7177A17446CBDE96F3EDD88FCD052F0B888A45FDAF2B631354F40D16E5FA9C2C4EDA98E798D15E6046DC5363F3096B2C607A9D8DD55B1502A6AC7D3CC8D8C575998E7D796910C804C495235057E91ECD2637C9C1845151AC6B9A0490AE3EC6F47740A0DB0BA36D075956CEE7354EA3E9A4F2720B26550C7D394324BC0CB7E9317D8A8661F42191FF10B08256CE3FD25B745E5194906B4D61CB4C2E000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000526F6F7400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001434130303030303030330000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000007BE8EF6CB279C9E2EEE121C6EAF44FF639F88F078B4B77ED9F9560B0358281B50E55AB721115A177703C7A30FE3AE9EF1C60BC1D974676B23A68CC04B198525BC968F11DE2DB50E4D9E7F071E562DAE2092233E9D363F61DD7C19FF3A4A91E8F6553D471DD7B84B9F1B8CE7335F0F5540563A1EAB83963E09BE901011F99546361287020E9CC0DAB487F140D6626A1836D27111F2068DE4772149151CF69C61BA60EF9D949A0F71F5499F2D39AD28C7005348293C431FFBD33F6BCA60DC7195EA2BCC56D200BAF6D06D09C41DB8DE9C720154CA4832B69C08C69CD3B073A0063602F462D338061A5EA6C915CD5623579C3EB64CE44EF586D14BAAA8834019B3EEBEED3790001000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"),
		},
		{
			"Root-CA00000003-XS00000020",
			pie::hac::es::sign::SIGN_ALGO_RSA2048,
			tc::cli::FormatUtil::hexStringToBytes("D21D3CE67C1069DA049D5E5310E76B907E18EEC80B337C4723E339573F4C664907DB2F0832D03DF5EA5F160A4AF24100D71AFAC2E3AE75AFA1228012A9A21616597DF71EAFCB65941470D1B40F5EF83A597E179FCB5B57C2EE17DA3BC3769864CB47856767229D67328141FC9AB1DF149E0C5C15AEB80BC58FC71BE18966642D68308B506934B8EF779F78E4DDF30A0DCF93FCAFBFA131A8839FD641949F47EE25CEECF814D55B0BE6E5677C1EFFEC6F29871EF29AA3ED9197B0D83852E050908031EF1ABBB5AFC8B3DD937A076FF6761AB362405C3F7D86A3B17A6170A659C16008950F7F5E06A5DE3E5998895EFA7DEEA060BE9575668F78AB1907B3BA1B7D"),
			tc::cli::FormatUtil::hexStringToBytes("00010004969FE8288DA6B9DD52C7BD63642A4A9AE5F053ECCB93613FDA37992087BD9199DA5E6797618D77098133FD5B05CD8288139E2E975CD2608003878CDAF020F51A0E5B7692780845561B31C61808E8A47C3462224D94F736E9A14E56ACBF71B7F11BBDEE38DDB846D6BD8F0AB4E4948C5434EAF9BF26529B7EB83671D3CE60A6D7A850DBE6801EC52A7B7A3E5A27BC675BA3C53377CFC372EBCE02062F59F37003AA23AE35D4880E0E4B69F982FB1BAC806C2F75BA29587F2815FD7783998C354D52B19E3FAD9FBEF444C48579288DB0978116AFC82CE54DACB9ED7E1BFD50938F22F85EECF3A4F426AE5FEB15B72F022FB36ECCE9314DAD131429BFC9675F58EE000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000526F6F742D4341303030303030303300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000015853303030303030323000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000D21D3CE67C1069DA049D5E5310E76B907E18EEC80B337C4723E339573F4C664907DB2F0832D03DF5EA5F160A4AF24100D71AFAC2E3AE75AFA1228012A9A21616597DF71EAFCB65941470D1B40F5EF83A597E179FCB5B57C2EE17DA3BC3769864CB47856767229D67328141FC9AB1DF149E0C5C15AEB80BC58FC71BE18966642D68308B506934B8EF779F78E4DDF30A0DCF93FCAFBFA131A8839FD641949F47EE25CEECF814D55B0BE6E5677C1EFFEC6F29871EF29AA3ED9197B0D83852E050908031EF1ABBB5AFC8B3DD937A076FF6761AB362405C3F7D86A3B17A6170A659C16008950F7F5E06A5DE3E5998895EFA7DEEA060BE9575668F78AB1907B3BA1B7D0001000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"),
		},
		{
			"Root-CA00000003-XS00000022",
			pie::hac::es::sign::SIGN_ALGO_RSA2048,
			tc::cli::FormatUtil::hexStringToBytes("E29775264ADC30C0FDE2DEFCFBBAC406A3B1C2ADDC2644C4C3E46DAD11E4F1ED7BC8B6E8CDBBE87E01020DF807520CE8A8F88BB53BE82CAFF4A1AF7FC5CD69EB34E24BEA74781203B611D09EBF56F82E3F21A494549F407FABA948545B34B64F487E1F27B031A83D337CE40496A3034AC6A2DB9E9353A1967EC7A07A4B3672B5B3F7934FF166D90EC131D8A75DC925ED629F8381A586589A82790489D4BF170F29F8E4BE50811ECBA9564D5517362D8DE777B01FC0A0AD8DED692CEAFD028A468CD6A6BF18A2767801AF8BB954EDB4DA3CED78337F3E9B6A1602B94752C85A53993678FDFA0300A4200CA752FA0D94E5A4B74B726FB61876146E49EC148508F5"),
			tc::cli::FormatUtil::hexStringToBytes("0001000458C7FFCDB0A1758925286C3C5029942683A8ED614A4E5C24EECE90548F99A0E59429C3D7D9CA37910CC1F5974E3AA2D7438627FBE456C9C830E5BF5B4440013CD41AA6F0AE02307E3EDEF3EDE0E6404A87234786FED37878F4BF4D964C2290C66AF167AC94869DC2F8378FA3FE0B764F776F9CE479B9E7A4EAE77FCA924B84035C9C06075F5F23BBF3A202AC1AF6A640C62BEEE4603925534783642663238A803D5FC87C5BEA58E09B0669CE9273509A87FACEDA94333FC6264D095708C18F9A50963F0E02A0771AEBC7174D4A89BE158C6CF407BD11658363E924F2808B15C7FE00267565CBD386E576540C906F9B2FFB3AFC64BA1198FA711A48F107E6A9AD000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000526F6F742D4341303030303030303300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000015853303030303030323200000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000E29775264ADC30C0FDE2DEFCFBBAC406A3B1C2ADDC2644C4C3E46DAD11E4F1ED7BC8B6E8CDBBE87E01020DF807520CE8A8F88BB53BE82CAFF4A1AF7FC5CD69EB34E24BEA74781203B611D09EBF56F82E3F21A494549F407FABA948545B34B64F487E1F27B031A83D337CE40496A3034AC6A2DB9E9353A1967EC7A07A4B3672B5B3F7934FF166D90EC131D8A75DC925ED629F8381A586589A82790489D4BF170F29F8E4BE50811ECBA9564D5517362D8DE777B01FC0A0AD8DED692CEAFD028A468CD6A6BF18A2767801AF8BB954EDB4DA3CED78337F3E9B6A1602B94752C85A53993678FDFA0300A4200CA752FA0D94E5A4B74B726FB61876146E49EC148508F50001000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"),
		},
		{
			"Root-CA00000003-XS00000024",
			pie::hac::es::sign::SIGN_ALGO_RSA2048,
			tc::cli::FormatUtil::hexStringToBytes("C694562BBDC74E83069876608EAD1EE7B1BC715036E9A6BDF58A6D22FD660ABD9CF360AA893D7746C92C5B9D20E480EAD1387786739956E060EBA79BD6ED7C06B2FCC6DBC1734ED12999AFEE5396FEE722FE3B96846D903F00A32671AC0CEDAAAB0C933C950304650A76152ED1B69547031793755599939B42F075CDF137F8C16D35870DE90D1BACBDB4B746F814AED0D3CB7F7C1B789A1FDF9947717B641BA972A6460D3258F5561CAA1D8E20085E8E3B9EBC92C9B6DCBB872A2AE02D74577F65EFA5AAFF29FD52EAA400D1408B2815C5C96CD2B54DF0F6708A755FB1DFA694A7794C4A0422F1BF8FAFE9E6870578133D50FD3D5356506CFCEC94EF79D6B639"),
			tc::cli::FormatUtil::hexStringToBytes("000100046EEED0424FF97D659DA43F85DF01E0997BA1FD1593997145BDE95703AABBDB7EFDE315F8CFE8AEC0F710FFF002938AEF8CEF7EBF90F725B6AC4EDA66DCF85132435E974DDBBE8F8C725CD114CE38C83404DFC0123BB4137C676C5C86EB4E211B7C35A03AB0C7E7637F1BF2486FDF0A5CCE5B047483841B5EA6BDC09AE770951ACA79EFB5497C46750585CF6C3FE4159ACE1CDFAB1BA8C00AB2446B57383F2F063344052E48E1557BD60BF973B252AC5AB4F5A12323AFCC1B06718775F3EDA4DD580A47A04D8063844235DC35D21A4FDED5A44EA401519920B73D15C66FDDF9F8286CF9122795EB9D5EA95C6E51603F6E62AA1631549EBE9E272B4BC3AEBC0D89000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000526F6F742D4341303030303030303300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000015853303030303030323400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000C694562BBDC74E83069876608EAD1EE7B1BC715036E9A6BDF58A6D22FD660ABD9CF360AA893D7746C92C5B9D20E480EAD1387786739956E060EBA79BD6ED7C06B2FCC6DBC1734ED12999AFEE5396FEE722FE3B96846D903F00A32671AC0CEDAAAB0C933C950304650A76152ED1B69547031793755599939B42F075CDF137F8C16D35870DE90D1BACBDB4B746F814AED0D3CB7F7C1B789A1FDF9947717B641BA972A6460D3258F5561CAA1D8E20085E8E3B9EBC92C9B6DCBB872A2AE02D74577F65EFA5AAFF29FD52EAA400D1408B2815C5C96CD2B54DF0F6708A755FB1DFA694A7794C4A0422F1BF8FAFE9E6870578133D50FD3D5356506CFCEC94EF79D6B6390001000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"),
		}
	};

	static const std::vector<sBroadOnRsaKeyAndCert> kDevBroadOnRsaKeyAndCert = 
	{
		{
			"Root",
			pie::hac::es::sign::SIGN_ALGO_RSA4096,
			tc::cli::FormatUtil::hexStringToBytes("D01FE100D43556B24B56DAE971B5A5D384B93003BE1BBF28A2305B060645467D5B0251D2561A274F9E9F9CEC646150AB3D2AE3366866ACA4BAE81AE3D79AA6B04A8BCBA7E6FB648945EBDFDB85BA091FD7D114B5A3A780E3A22E6ECD87B5A4C6F910E4032208814B0CEEA1A17DF739695F617EF63528DB949637A056037F7B32413895C0A8F1982E1565E38EEDC22E590EE2677B8609F48C2E303FBC405CAC18042F822084E4936803DA7F41349248562B8EE12F78F803246330BC7BE7EE724AF458A472E7AB46A1A7C10C2F18FA07C3DDD89806A11C9CC130B247A33C8D47DE67F29E5577B11C43493D5BBA7634A7E4E71531B7DF5981FE24A114554CBD8F005CE1DB35085CCFC77806B6DE254068A26CB5492D4580438FE1E5A9ED75C5ED451DCE789439CCC3BA28A2312A1B8719EF0F73B713950C02591A7462A607F37C0AA7A18FA943A36D752A5F4192F0136100AA9CB41BBE14BEB1F9FC692FDFA09446DE5A9DDE2CA5F68C1C0C21429287CB2DAAA3D263752F73E09FAF4479D2817429F69800AFDE6B592DC19882BDF581CCABF2CB91029EF35C4CFDBBFF49C1FA1B2FE31DE7A560ECB47EBCFE32425B956F81B69917487E3B789151DB2E78B1FD2EBE7E626B3EA165B4FB00CCB751AF507329C4A3939EA6DD9C50A0E7386B0145796B41AF61F78555944F3BC22DC3BD0D00F8798A42B1AAA08320659AC7395AB4F329"),
			tc::ByteData()
		},
		{
			"Root-CA00000004",
			pie::hac::es::sign::SIGN_ALGO_RSA2048,
			tc::cli::FormatUtil::hexStringToBytes("C9CC2DC4DF2930E4DF3F8C70A078948775AD5E9AA604C5B4D8EAFF2AA1D214676564EFCA28CC00154554A1A3EA1379E9E6CAACED1593FE88D89AC6B8ACCCAB6E207CEB7CCA29809E2980440662B7D4382A15DA43085745A9AAE59AA05BDB32F66869A2DD4295386C87ECDD3508A2CF60D01E23EC2FE698F470D6001549A2F06759131E534C7006057DEF1D18A83F0AC79CFE80FF5A91F2BED4A0837061190A0329902165403C9A908FB615739F3CE33BF1BAEA16C25BCED7963FACC9D24D9C0AD76FC020B2C4B84C10A741A2CC7D9BAC3AACCCA3529BAC316A9AA75D2A26C7D7D288CBA466C5FE5F454AE679744A90A15772DB3B0E47A49AF031D16DBEAB332B"),
			tc::cli::FormatUtil::hexStringToBytes("000100031949429D1E58A62E7E8B56D1B76AE302FD8B97491F778745F75388C4DD0BEB1DF122FB9642151497764A53CF78151845E42CA8FDE486FD2A4F53F8A1BA008A7485FF73B3BF7E3C980729D0656B693219ADE835EB5FFFFCCB7CBB5E307FE0688B888EF2D2053FB7E791E985FD15EF10D79CCA88D6BB15E8E4714A98EE09BF7B8AF053232B6450E6D5FDFFC20A6D1EA6A23812E1014525D56D4082703B86986959A73CD1A14364D2C2DAEA96B095F76C46E4FF4155465E70EF1ED31053D97011E010CC93E7914013687FA3A802996D1E557B1CCC7A7E8F5865C1742E28E26DEF38A93AB5D82D43ECCCBF0BEF22E1FD57E2864333582FEDEABC012F986DDFC3E9447973470308455BDC57AA170B84427F73A29B48F6DA135F66C745C142A84AFB0E6A5EED85D7B9719936F8CE2B621F395F40DC03BEF8854C1117FF0C128641CC7843B97B4346DB226F6026ACB56C278B8E0EA79A2D65EF798E1078AD80ED4B9604D2F08B2CD64A23A3DB270833B402F80851F35BED3EE4577C6660FBF16D9413E09C917A49D42C6DA375BC27F0230DB98F8973AB027B522CD57EC03D25E8B3FC3494C97FB108FE18C68A4336E46C26B6F280D27E34BE287C3E4687BC9D776B76D928D1B6352EC0347D7294AA9360268D26F5F652064AF240D7D00C7C5EA3C32DE62D9B5C4B4CAB6FD7BD371D57C2166095910E4AD8E9ED181EF761936153892D77000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000526F6F74000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000014341303030303030303400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000081122A46C9CC2DC4DF2930E4DF3F8C70A078948775AD5E9AA604C5B4D8EAFF2AA1D214676564EFCA28CC00154554A1A3EA1379E9E6CAACED1593FE88D89AC6B8ACCCAB6E207CEB7CCA29809E2980440662B7D4382A15DA43085745A9AAE59AA05BDB32F66869A2DD4295386C87ECDD3508A2CF60D01E23EC2FE698F470D6001549A2F06759131E534C7006057DEF1D18A83F0AC79CFE80FF5A91F2BED4A0837061190A0329902165403C9A908FB615739F3CE33BF1BAEA16C25BCED7963FACC9D24D9C0AD76FC020B2C4B84C10A741A2CC7D9BAC3AACCCA3529BAC316A9AA75D2A26C7D7D288CBA466C5FE5F454AE679744A90A15772DB3B0E47A49AF031D16DBEAB332B0001000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"),
		},
		{
			"Root-CA00000004-XS00000020",
			pie::hac::es::sign::SIGN_ALGO_RSA2048,
			tc::cli::FormatUtil::hexStringToBytes("BA278428765D879A7F215404C6EE4E0A0D3F66C33BC7F8A32FD898E52CB7B63443CED8B00527D89DEDC6BBF60AD1C5C9923021DD555F9BAD4BE0C0C406D3702915E5B34AC2D2ABE503C32A3A23B43834C9157B9A0AF2E4E9C03BEDE2B4C115F5353DFC66BD04A6C079970E38CBDD5A50A2B98FF2D765FCF83381E9E0E849C3573578378FF65951613E95F75EE8EF26183A40AAE4A76D7384EA478D2CDCE80FBA0321A6BF8D69983C3AA7AE5443B72BFE410BF1323CBD88C3560EA13D17D38A2E34041DE7AAF389DE4375225CA87EE349C760C7D99BE7E737C6261CC74E25AE46527AC9619F939057088D7435A6DE7B25AB82DD5410F2579CAEF1491A909D302B"),
			tc::cli::FormatUtil::hexStringToBytes("00010004603483F7F4EF3C863FF7B46821E9D83983EFB0BA29C45B10DFB2E8A268AE06084DFEB0B9680B6D40D6DF82CAE7D651E28F31364F5AB567DD9DCCFCBB5EFF8F1CEA92489C046E8BED2AF0D7F9CA0BD50F683633D681B08D1AF417D7B7F8762AD88E3AF0D1C45DAC5407B9D8F24F167AEDC7C5DBCF4E0E176EC341D4F9BA8ADFE749CF37F45688653886566A9045ED89AD353DD289E32631867BDFD7FC67738BDF0E1E858F3BF9DEE25B02206F0B7FFB61F866DA4D862D77DDBDA9668BB61CBDEFACB2583CE5C9FAB22FC90EF8EB06954743E5A32A37A3513C6C795BC8E0C65E2F170B435A8C447765381C4050427F88E9828E6E94144D28441C7F9C7F6D92FE5B000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000526F6F742D4341303030303030303400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000015853303030303030323000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000BA278428765D879A7F215404C6EE4E0A0D3F66C33BC7F8A32FD898E52CB7B63443CED8B00527D89DEDC6BBF60AD1C5C9923021DD555F9BAD4BE0C0C406D3702915E5B34AC2D2ABE503C32A3A23B43834C9157B9A0AF2E4E9C03BEDE2B4C115F5353DFC66BD04A6C079970E38CBDD5A50A2B98FF2D765FCF83381E9E0E849C3573578378FF65951613E95F75EE8EF26183A40AAE4A76D7384EA478D2CDCE80FBA0321A6BF8D69983C3AA7AE5443B72BFE410BF1323CBD88C3560EA13D17D38A2E34041DE7AAF389DE4375225CA87EE349C760C7D99BE7E737C6261CC74E25AE46527AC9619F939057088D7435A6DE7B25AB82DD5410F2579CAEF1491A909D302B0001000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"),
		},
		{
			"Root-CA00000004-XS00000021",
			pie::hac::es::sign::SIGN_ALGO_RSA2048,
			tc::cli::FormatUtil::hexStringToBytes("CA303087732E88FB2736A4644B6A84DF02393786EBE3DBE914F92224126AB3C38D7CA67322B7D3107B9AC0E00AED9AF13DEDDC62BCB8AC889A47B07196B45BC97AC5DD333CC63E872E30CDB52BD35E055FBA56C2521AFC8BBFC8B06390522E99699750B457C8647A42D6318BE5EAA192ADA35DA7023FC348215613141919821FD5599B93497893CA58E7A88C8D6E6E9EB0A57567BF8C12EB25241BB6082BED7A696658A58DDB66E7CC2562F2EA061E5373E6A5397BF6299F7EA9065B0047AA408F24A5D6F6E4050B99DA86831F663792E08D96F182400182D9BECB917AE315A238CA89741B6AEAF52E238590CDA60FC5B74DC638F9D6FCAF95D2CA7AE005A7B1"),
			tc::cli::FormatUtil::hexStringToBytes("000100041282394569ED63B5C4E54747B31E3B32C68B0EB90712E3C04662101CA77A551A0394CC36CB28E369DFBBB9FD78C9C503C3B1F184729C2B05B4D1691581A877AF82F8EC3D70E95B27F459039B65D8221D4864CCB282FF721EBDD6C46863F8C95591C1BF36D0AC118C0027FA3E2D434821A5FEF123C65F17885321F9A520C22B9EE6FC533EEF582706A95DD0CEABACE87B0169E136336E2E9CC0399637CE4CC77B6DA418671A277614F80A0A909569018F624DE0C5E1A67DB134C63F1FFE30BB64C201FDD8DAD42F4C88B247C1C6133CC554DE6B15684320D6AA1E09CB59EAD15F37140C9DD4BE69D87CC90DD8F733C946FBA6C576C70533601AF4E8294A3B5DD4000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000526F6F742D4341303030303030303400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000015853303030303030323100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000CA303087732E88FB2736A4644B6A84DF02393786EBE3DBE914F92224126AB3C38D7CA67322B7D3107B9AC0E00AED9AF13DEDDC62BCB8AC889A47B07196B45BC97AC5DD333CC63E872E30CDB52BD35E055FBA56C2521AFC8BBFC8B06390522E99699750B457C8647A42D6318BE5EAA192ADA35DA7023FC348215613141919821FD5599B93497893CA58E7A88C8D6E6E9EB0A57567BF8C12EB25241BB6082BED7A696658A58DDB66E7CC2562F2EA061E5373E6A5397BF6299F7EA9065B0047AA408F24A5D6F6E4050B99DA86831F663792E08D96F182400182D9BECB917AE315A238CA89741B6AEAF52E238590CDA60FC5B74DC638F9D6FCAF95D2CA7AE005A7B10001000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"),
		},
		{
			"Root-CA00000004-XS00000024",
			pie::hac::es::sign::SIGN_ALGO_RSA2048,
			tc::cli::FormatUtil::hexStringToBytes("E5E262EA591EEB885DE5A3CA451CE9E4FEA0E89DAA9BBCD9EC6C23E3FEFCE31BE7C7217DAC3E1F5A8331EF3F0C738E9310E765E71AAAB92C6E18D36071E7F93446E06E2581A28483AE9EB9C8217C1D68A21FD098E186C71B6A1E1E2626C80B69FFD562EAD515C77D616554F6B74B34DEEB0DE905F44631F56A6FF12B47AC0190578267559592FAB173146071ECFBB014951512E45299A21C0B8B3377E55DCD95625130E533877BF11157585D43DB00F61DFE77CFA44F5F29F44945DBF4214F37214921C16ED7D74455B2AE7C8CA15DFBD20A4D5E6501965B78C2C542727D7A91C071C56403738F07E858239CD150DB304BA5ACD8751145A9A91D3E97085AD217"),
			tc::cli::FormatUtil::hexStringToBytes("000100049EA82934BD64EDCED4D506BB68BF1F217BAD912BEF134B687C8E590F2A9355D8EDD9BE00D85735A83820F69978B3E2876FE07BBB7F356B25CDCC98EEEA17BFA9EF649769E921BFDA7867D0570D2F0A7A223E7ACD8D9AE1F304F8EEC6C96B11805436CF686FBD05969E4C0D19A63EF89DA00899222D2CF006776FF027118E96CB027114AABBA268CB1775022B25EAF045C9E4A328A55FF85CFFFFFEAEFB78F1CD184ED9042D23A06F0161E5FBADE92638A483A1A3680DC9994A03D02F1D76EB0EE793843D31CCC172A3993C7344B2C5E634F2B89C47F85FD8B74D21745AEBCFD55E32FC03F6BCD40421499261F8AB93AC8A07E6EF5436036283A3B86192890A78000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000526F6F742D4341303030303030303400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000015853303030303030323400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000E5E262EA591EEB885DE5A3CA451CE9E4FEA0E89DAA9BBCD9EC6C23E3FEFCE31BE7C7217DAC3E1F5A8331EF3F0C738E9310E765E71AAAB92C6E18D36071E7F93446E06E2581A28483AE9EB9C8217C1D68A21FD098E186C71B6A1E1E2626C80B69FFD562EAD515C77D616554F6B74B34DEEB0DE905F44631F56A6FF12B47AC0190578267559592FAB173146071ECFBB014951512E45299A21C0B8B3377E55DCD95625130E533877BF11157585D43DB00F61DFE77CFA44F5F29F44945DBF4214F37214921C16ED7D74455B2AE7C8CA15DFBD20A4D5E6501965B78C2C542727D7A91C071C56403738F07E858239CD150DB304BA5ACD8751145A9A91D3E97085AD2170001000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"),
		}
	};

	if (isDev)
	{
		if (xci_header_sign_key.isNull())
			xci_header_sign_key = tc::crypto::RsaPublicKey(kXciHeaderSignModulus.data(), kXciHeaderSignModulus.size());

		if (xci_cert_sign_key.isNull())
			xci_cert_sign_key = tc::crypto::RsaPublicKey(kXciCertSignModulus.data(), kXciCertSignModulus.size());

		if (pkg2_sign_key.isNull())
			pkg2_sign_key = tc::crypto::RsaPublicKey(kDevPackage2HeaderModulus.data(), kDevPackage2HeaderModulus.size());

		for (auto itr = kDevNcaHeaderSign0Modulus.begin(); itr != kDevNcaHeaderSign0Modulus.end(); itr++)
		{
			if (nca_header_sign0_key.find(itr->generation) == nca_header_sign0_key.end())
				nca_header_sign0_key[itr->generation] = tc::crypto::RsaPublicKey(itr->modulus.data(), itr->modulus.size());
		}

		for (auto itr = kDevAcidSignModulus.begin(); itr != kDevAcidSignModulus.end(); itr++)
		{
			if (acid_sign_key.find(itr->generation) == acid_sign_key.end())
				acid_sign_key[itr->generation] = tc::crypto::RsaPublicKey(itr->modulus.data(), itr->modulus.size());
		}

		for (auto itr = kDevBroadOnRsaKeyAndCert.begin(); itr != kDevBroadOnRsaKeyAndCert.end(); itr++)
		{
			if (broadon_signer.find(itr->issuer) == broadon_signer.end())
				broadon_signer[itr->issuer] = {itr->certificate, itr->key_type, tc::crypto::RsaPublicKey(itr->modulus.data(), itr->modulus.size())};
		}
	}
	else
	{
		if (xci_header_sign_key.isNull())
			xci_header_sign_key = tc::crypto::RsaPublicKey(kXciHeaderSignModulus.data(), kXciHeaderSignModulus.size());

		if (xci_cert_sign_key.isNull())
			xci_cert_sign_key = tc::crypto::RsaPublicKey(kXciCertSignModulus.data(), kXciCertSignModulus.size());

		if (pkg2_sign_key.isNull())
			pkg2_sign_key = tc::crypto::RsaPublicKey(kProdPackage2HeaderModulus.data(), kProdPackage2HeaderModulus.size());

		for (auto itr = kProdNcaHeaderSign0Modulus.begin(); itr != kProdNcaHeaderSign0Modulus.end(); itr++)
		{
			if (nca_header_sign0_key.find(itr->generation) == nca_header_sign0_key.end())
				nca_header_sign0_key[itr->generation] = tc::crypto::RsaPublicKey(itr->modulus.data(), itr->modulus.size());
		}

		for (auto itr = kProdAcidSignModulus.begin(); itr != kProdAcidSignModulus.end(); itr++)
		{
			if (acid_sign_key.find(itr->generation) == acid_sign_key.end())
				acid_sign_key[itr->generation] = tc::crypto::RsaPublicKey(itr->modulus.data(), itr->modulus.size());
		}

		for (auto itr = kProdBroadOnRsaKeyAndCert.begin(); itr != kProdBroadOnRsaKeyAndCert.end(); itr++)
		{
			if (broadon_signer.find(itr->issuer) == broadon_signer.end())
				broadon_signer[itr->issuer] = {itr->certificate, itr->key_type, tc::crypto::RsaPublicKey(itr->modulus.data(), itr->modulus.size())};
		}
	}
}