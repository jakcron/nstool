#include "KeyBag.h"

#include "util.h"
#include <tc/cli/FormatUtil.h>

#include <nn/hac/define/gc.h>
#include <nn/hac/AesKeygen.h>

#include <nn/pki/SignUtils.h>
#include <nn/pki/SignedData.h>
#include <nn/pki/CertificateBody.h>
#include <nn/es/TicketBody_V2.h>

nstool::KeyBagInitializer::KeyBagInitializer(bool isDev, const tc::Optional<tc::io::Path>& keyfile_path, const tc::Optional<tc::io::Path>& tik_path, const tc::Optional<tc::io::Path>& cert_path)
{
	if (keyfile_path.isSet())
	{
		importBaseKeyFile(keyfile_path.get(), isDev);
	}
	if (cert_path.isSet())
	{
		importCertificateChain(cert_path.get());
	}
	if (tik_path.isSet())
	{
		importTicket(tik_path.get());
	}
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

	static const size_t kMasterKeyMax = 0x20;
	/**/

	// import key data
	for (size_t name_idx = 0; name_idx < kNameVariantNum; name_idx++)
	{
		/* internal key sources */
		if (name_idx < kMasterBase.size())
		{
			for (size_t mkey_rev = 0; mkey_rev < kMasterKeyMax; mkey_rev++)
			{
				// std::map<byte_t, aes128_key_t> master_key;
				//fmt::print("{:s}_key_{:02x}\n", kMasterBase[name_idx], mkey_rev);
				_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:02x}", kMasterBase[name_idx], kKeyStr, mkey_rev), master_key[mkey_rev]);
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
			for (size_t mkey_rev = 0; mkey_rev < kMasterKeyMax; mkey_rev++)
			{
				//fmt::print("{:s}_key_{:02x}\n", kPkg1Base[name_idx], mkey_rev);
				_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:02x}", kPkg1Base[name_idx], kKeyStr, mkey_rev), pkg1_key[mkey_rev]);
			}
		}

		/* package2 */
		if (name_idx < kPkg2Base.size())
		{
			// package2_key_xx
			for (size_t mkey_rev = 0; mkey_rev < kMasterKeyMax; mkey_rev++)
			{
				//fmt::print("{:s}_key_{:02x}\n", kPkg2Base[name_idx], mkey_rev);
				_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:02x}", kPkg2Base[name_idx], kKeyStr, mkey_rev), pkg2_key[mkey_rev]);
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
			for (size_t mkey_rev = 0; mkey_rev < kMasterKeyMax; mkey_rev++)
			{
				//fmt::print("{:s}_{:02x}\n", kTicketCommonKeyBase[name_idx], mkey_rev);
				_SAVE_AES128KEY(fmt::format("{:s}_{:02x}", kTicketCommonKeyBase[name_idx], mkey_rev), etik_common_key[mkey_rev]);
			}
		}

		/* NCA keys */
		if (name_idx < kContentArchiveHeaderBase.size())
		{
			// nca header key
			//fmt::print("{:s}_{:s}\n", kContentArchiveHeaderBase[name_idx], kKeyStr);
			//_SAVE_AES128XTSKEY(fmt::format("{:s}_{:s}", kContentArchiveHeaderBase[name_idx], kKeyStr), nca_header_key);
			
			// nca header sign0 key (generations)
			for (size_t mkey_rev = 0; mkey_rev < kMasterKeyMax; mkey_rev++)
			{
				//fmt::print("{:s}_{:s}_{:02x}_{:s}\n", kContentArchiveHeaderBase[name_idx], kSignKey, mkey_rev, kPrivateStr);
				//fmt::print("{:s}_{:s}_{:02x}_{:s}\n", kContentArchiveHeaderBase[name_idx], kSignKey, mkey_rev, kModulusStr);
				_SAVE_RSAKEY(fmt::format("{:s}_{:s}_{:02x}", kContentArchiveHeaderBase[name_idx], kSignKey, mkey_rev), nca_header_sign0_key[mkey_rev], 2048);
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
			for (size_t mkey_rev = 0; mkey_rev < kMasterKeyMax; mkey_rev++)
			{
				for (size_t keak_idx = 0; keak_idx < kNcaKeyAreaKeyIndexStr.size(); keak_idx++)
				{
					//fmt::print("{:s}_{:s}_{:02x}\n", kNcaKeyAreaEncKeyBase[name_idx], kNcaKeyAreaKeyIndexStr[keak_idx], mkey_rev);
					_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:02x}", kNcaKeyAreaEncKeyBase[name_idx], kNcaKeyAreaKeyIndexStr[keak_idx], mkey_rev), nca_key_area_encryption_key[keak_idx][mkey_rev]);
				}
			}
		}
		// nca key area "hw" encryption keys
		if (name_idx < kNcaKeyAreaEncKeyHwBase.size())
		{
			for (size_t mkey_rev = 0; mkey_rev < kMasterKeyMax; mkey_rev++)
			{
				for (size_t keak_idx = 0; keak_idx < kNcaKeyAreaKeyIndexStr.size(); keak_idx++)
				{
					//fmt::print("{:s}_{:s}_{:02x}\n", kNcaKeyAreaEncKeyHwBase[name_idx], kNcaKeyAreaKeyIndexStr[keak_idx], mkey_rev);
					_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:02x}", kNcaKeyAreaEncKeyHwBase[name_idx], kNcaKeyAreaKeyIndexStr[keak_idx], mkey_rev), nca_key_area_encryption_key_hw[keak_idx][mkey_rev]);
				}
			}
		}

		/* ACID */
		if (name_idx < kAcidBase.size())
		{
			// acid sign key (generations)
			for (size_t mkey_rev = 0; mkey_rev < kMasterKeyMax; mkey_rev++)
			{
				//fmt::print("{:s}_{:s}_{:02x}_{:s}\n", kAcidBase[name_idx], kSignKey, mkey_rev, kPrivateStr);
				//fmt::print("{:s}_{:s}_{:02x}_{:s}\n", kAcidBase[name_idx], kSignKey, mkey_rev, kModulusStr);
				_SAVE_RSAKEY(fmt::format("{:s}_{:s}_{:02x}", kAcidBase[name_idx], kSignKey, mkey_rev), acid_sign_key[mkey_rev], 2048);
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
			for (size_t mkey_rev = 0; mkey_rev < kMasterKeyMax; mkey_rev++)
			{
				//fmt::print("{:s}_{:s}_{:02x}_{:s}\n", kNrrCertBase[name_idx], kSignKey, mkey_rev, kPrivateStr);
				//fmt::print("{:s}_{:s}_{:02x}_{:s}\n", kNrrCertBase[name_idx], kSignKey, mkey_rev, kModulusStr);
				_SAVE_RSAKEY(fmt::format("{:s}_{:s}_{:02x}", kNrrCertBase[name_idx], kSignKey, mkey_rev), nrr_certificate_sign_key[mkey_rev], 2048);
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
			for (size_t kek_index = 0; kek_index < 8; kek_index++)
			{
				//fmt::print("{:s}_{:s}_{:02x}\n", kXciHeaderBase[name_idx], kKeyStr, kek_index);
				_SAVE_AES128KEY(fmt::format("{:s}_{:s}_{:02x}", kXciHeaderBase[name_idx], kKeyStr, kek_index), xci_header_key[kek_index]);
			}
			// xci header key (old lable)
			//fmt::print("{:s}_{:s}\n", kXciHeaderBase[name_idx], kKeyStr);
			_SAVE_AES128KEY(fmt::format("{:s}_{:s}", kXciHeaderBase[name_idx], kKeyStr), xci_header_key[isDev ? nn::hac::gc::KEK_DEV : nn::hac::gc::KEK_PROD]);

			// xci header sign key
			//fmt::print("{:s}_{:s}_{:s}\n", kXciHeaderBase[name_idx], kSignKey, kPrivateStr);
			//fmt::print("{:s}_{:s}_{:s}\n", kXciHeaderBase[name_idx], kSignKey, kModulusStr);
			_SAVE_RSAKEY(fmt::format("{:s}_{:s}", kXciHeaderBase[name_idx], kSignKey), xci_header_sign_key, 2048);
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
					nn::hac::AesKeygen::generateKey(nca_header_kek_tmp.data(), aes_kek_generation_source.get().data(), nca_header_kek_source.get().data(), aes_key_generation_source.get().data(), itr->second.data());
					
					aes128_xtskey_t nca_header_key_tmp;
					nn::hac::AesKeygen::generateKey(nca_header_key_tmp[0].data(), nca_header_key_source.get()[0].data(), nca_header_kek_tmp.data());
					nn::hac::AesKeygen::generateKey(nca_header_key_tmp[1].data(), nca_header_key_source.get()[1].data(), nca_header_kek_tmp.data());

					nca_header_key = nca_header_key_tmp;
				}
			}

			for (size_t keak_idx = 0; keak_idx < nn::hac::nca::kKeyAreaEncryptionKeyNum; keak_idx++)
			{
				if (key_area_key_source[keak_idx].isSet() && nca_key_area_encryption_key[keak_idx].find(itr->first) != nca_key_area_encryption_key[keak_idx].end())
				{
					aes128_key_t nca_key_area_encryption_key_tmp;
					nn::hac::AesKeygen::generateKey(nca_key_area_encryption_key_tmp.data(), aes_kek_generation_source.get().data(), key_area_key_source[keak_idx].get().data(), aes_key_generation_source.get().data(), itr->second.data());
					nca_key_area_encryption_key[keak_idx][itr->first] = nca_key_area_encryption_key_tmp;
				}
			}
		}
		if (ticket_titlekek_source.isSet() && etik_common_key.find(itr->first) == etik_common_key.end())
		{
			aes128_key_t etik_common_key_tmp;
			nn::hac::AesKeygen::generateKey(etik_common_key_tmp.data(), ticket_titlekek_source.get().data(), itr->second.data());
			etik_common_key[itr->first] = etik_common_key_tmp;
		}
		if (package2_key_source.isSet() && pkg2_key.find(itr->first) == pkg2_key.end())
		{
			aes128_key_t pkg2_key_tmp;
			nn::hac::AesKeygen::generateKey(pkg2_key_tmp.data(), package2_key_source.get().data(), itr->second.data());
			pkg2_key[itr->first] = pkg2_key_tmp;
		}
	}

	// Save PKI Root Key
	if (pki_root_sign_key.isSet())
	{
		broadon_rsa_signer["Root"] = { tc::ByteData(), pki_root_sign_key.get() };
	}
}

void nstool::KeyBagInitializer::importTitleKeyFile(const tc::io::Path& keyfile_path)
{

}

void nstool::KeyBagInitializer::importCertificateChain(const tc::io::Path& cert_path)
{
	// save file path string for error messages
	std::string cert_path_str;
	tc::io::PathUtil::pathToUnixUTF8(cert_path, cert_path_str);

	// open cert file
	std::shared_ptr<tc::io::FileStream> certfile_stream;
	try {
		certfile_stream = std::make_shared<tc::io::FileStream>(tc::io::FileStream(cert_path, tc::io::FileMode::Open, tc::io::FileAccess::Read));
	}
	catch (tc::io::FileNotFoundException& e) {
		fmt::print("[WARNING] Failed to open certificate file \"{:s}\" ({:s}).\n", cert_path_str, e.error());
		return;
	}
	
	// check size
	size_t cert_raw_size = tc::io::IOUtil::castInt64ToSize(certfile_stream->length());
	if (cert_raw_size > 0x10000)
	{
		fmt::print("[WARNING] Certificate file \"{:s}\" was too large.\n", cert_path_str);
		return;
	}

	// import cert data
	tc::ByteData cert_raw = tc::ByteData(cert_raw_size);
	certfile_stream->seek(0, tc::io::SeekOrigin::Begin);
	certfile_stream->read(cert_raw.data(), cert_raw.size());

	nn::pki::SignedData<nn::pki::CertificateBody> cert;
	try {
		for (size_t f_pos = 0; f_pos < cert_raw.size(); f_pos += cert.getBytes().size())
		{
			cert.fromBytes(cert_raw.data() + f_pos, cert_raw.size() - f_pos);

			std::string cert_identity = fmt::format("{:s}-{:s}", cert.getBody().getIssuer(), cert.getBody().getSubject());

			switch (cert.getBody().getPublicKeyType()) {
				case nn::pki::cert::PublicKeyType::RSA2048:
					broadon_rsa_signer[cert_identity] = { cert.getBytes(), cert.getBody().getRsa2048PublicKey() };
					break;
				case nn::pki::cert::PublicKeyType::RSA4096:
					broadon_rsa_signer[cert_identity] = { cert.getBytes(), cert.getBody().getRsa4096PublicKey() };
					break;
				case nn::pki::cert::PublicKeyType::ECDSA240:
					fmt::print("[WARNING] Certificate {:s} will not be imported. ecc233 public keys are not supported yet.\n", cert_identity);
					break;
				default:
					fmt::print("[WARNING] Certificate {:s} will not be imported. Unknown public key type.\n", cert_identity);
			}
		}
	}
	catch (tc::Exception& e) {
		fmt::print("[WARNING] Certificate file \"{:s}\" is corrupted ({:s}).\n", cert_path_str, e.error());
		return;
	}
}

void nstool::KeyBagInitializer::importTicket(const tc::io::Path& tik_path)
{
	// save file path string for error messages
	std::string tik_path_str;
	tc::io::PathUtil::pathToUnixUTF8(tik_path, tik_path_str);

	// open cert file
	std::shared_ptr<tc::io::FileStream> tik_stream;
	try {
		tik_stream = std::make_shared<tc::io::FileStream>(tc::io::FileStream(tik_path, tc::io::FileMode::Open, tc::io::FileAccess::Read));
	}
	catch (tc::io::FileNotFoundException& e) {
		fmt::print("[WARNING] Failed to open ticket \"{:s}\" ({:s}).\n", tik_path_str, e.error());
		return;
	}

	// check size
	size_t tik_raw_size = tc::io::IOUtil::castInt64ToSize(tik_stream->length());
	if (tik_raw_size > 0x10000)
	{
		fmt::print("[WARNING] Ticket \"{:s}\" was too large.\n", tik_path_str);
		return;
	}

	// import cert data
	tc::ByteData tik_raw = tc::ByteData(tik_raw_size);
	tik_stream->seek(0, tc::io::SeekOrigin::Begin);
	tik_stream->read(tik_raw.data(), tik_raw.size());

	nn::pki::SignedData<nn::es::TicketBody_V2> tik;
	try {
		// de serialise ticket
		tik.fromBytes(tik_raw.data(), tik_raw.size());
		
		// save rights id
		rights_id_t rights_id;
		memcpy(rights_id.data(), tik.getBody().getRightsId(), rights_id.size());
		
		// check ticket is not personalised
		if (tik.getBody().getTitleKeyEncType() != nn::es::ticket::AES128_CBC)
		{
			fmt::print("[WARNING] Ticket \"{:s}\" will not be imported. Personalised tickets are not supported.\n", tc::cli::FormatUtil::formatBytesAsString(rights_id.data(), rights_id.size(), true, ""));
			return;
		}

		// save enc title key
		aes128_key_t enc_title_key;
		memcpy(enc_title_key.data(), tik.getBody().getEncTitleKey(), enc_title_key.size());

		// save the encrypted title key as the fallback enc content key incase the ticket was malformed and workarounds to decrypt it in isolation fail
		if (fallback_enc_content_key.isNull())
		{
			fallback_enc_content_key = enc_title_key;
		}

		// determine key to decrypt title key
		byte_t common_key_index = tik.getBody().getCommonKeyId();

		// work around for bad scene tickets where they don't set the commonkey id field (detect scene ticket with ffff.... signature)
		if (common_key_index != rights_id[15] && tik.getSignature().getBytes()[0x00] == 0xff && tik.getSignature().getBytes()[0x01] == 0xff)
		{
			common_key_index = rights_id[15];
		}
		if (etik_common_key.find(tik.getBody().getCommonKeyId()) == etik_common_key.end())
		{
			fmt::print("[WARNING] Ticket \"{:s}\" will not be imported. Could not decrypt title key.\n", tc::cli::FormatUtil::formatBytesAsString(rights_id.data(), rights_id.size(), true, ""));
			return;
		}

		// decrypt title key
		aes128_key_t dec_title_key;
		tc::crypto::DecryptAes128Ecb(dec_title_key.data(), enc_title_key.data(), sizeof(aes128_key_t), etik_common_key[common_key_index].data(), sizeof(aes128_key_t));

		// add to key dict
		external_content_keys[rights_id] = dec_title_key;
		
	}
	catch (tc::Exception& e) {
		fmt::print("[WARNING] Ticket \"{:s}\" is corrupted ({:s}).\n", tik_path_str, e.error());
		return;
	}
}