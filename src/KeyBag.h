#pragma once
#include <string>
#include <vector>
#include <array>
#include <map>
#include <tc/Optional.h>
#include <tc/io.h>
#include <pietendo/hac/es/SignUtils.h>
#include <pietendo/hac/define/types.h>
#include <pietendo/hac/define/nca.h>

namespace nstool {

struct KeyBag
{
	using aes128_key_t = pie::hac::detail::aes128_key_t;
	using aes128_xtskey_t = pie::hac::detail::aes128_xtskey_t;
	using rsa_key_t = tc::crypto::RsaKey;
	//using ecc_key_t = tc::crypto::EccKey;
	using rights_id_t = pie::hac::detail::rights_id_t;
	using key_generation_t = byte_t;
	using broadon_issuer_t = std::string;
	static const size_t kNcaKeakNum = pie::hac::nca::kKeyAreaEncryptionKeyNum;


	// acid
	std::map<key_generation_t, rsa_key_t> acid_sign_key;

	// pkg1 and pkg2
	std::map<key_generation_t, aes128_key_t> pkg1_key;
	std::map<key_generation_t, aes128_key_t> pkg2_key;
	tc::Optional<rsa_key_t> pkg2_sign_key;

	// nca
	tc::Optional<aes128_xtskey_t> nca_header_key;
	std::map<key_generation_t, rsa_key_t> nca_header_sign0_key;
	std::array<std::map<key_generation_t, aes128_key_t>, kNcaKeakNum> nca_key_area_encryption_key;
	std::array<std::map<key_generation_t, aes128_key_t>, kNcaKeakNum> nca_key_area_encryption_key_hw;

	// external content keys (nca<->ticket)
	std::map<rights_id_t, aes128_key_t> external_content_keys;
	std::map<rights_id_t, aes128_key_t> external_enc_content_keys; // encrypted content key list to be used when external_content_keys does not have the required content key (usually taken raw from ticket)
	tc::Optional<aes128_key_t> fallback_enc_content_key; // encrypted content key to be used when external_content_keys does not have the required content key (usually taken raw from ticket)
	tc::Optional<aes128_key_t> fallback_content_key; // content key to be used when external_content_keys does not have the required content key (usually already decrypted from ticket)

	// nrr
	std::map<key_generation_t, rsa_key_t> nrr_certificate_sign_key;

	// xci
	tc::Optional<rsa_key_t> xci_header_sign_key;
	std::map<byte_t, aes128_key_t> xci_header_key;
	std::map<byte_t, aes128_key_t> xci_initial_data_kek;
	tc::Optional<rsa_key_t> xci_cert_sign_key;

	// ticket
	std::map<key_generation_t, aes128_key_t> etik_common_key;

	// BroadOn signer profiles (for es cert and es tik)
	// BroadOn Keys
	struct BroadOnSignerProfile
	{
		tc::ByteData certificate;

		pie::hac::es::sign::SignatureAlgo key_type;
		rsa_key_t rsa_key;
		// ecc_key_t ecc_key;
	};
	std::map<broadon_issuer_t, BroadOnSignerProfile> broadon_signer;
};

class KeyBagInitializer : public KeyBag
{
public:
	KeyBagInitializer(bool isDev, const tc::Optional<tc::io::Path>& keyfile_path, const tc::Optional<tc::io::Path>& titlekeyfile_path, const std::vector<tc::io::Path>& tik_path_list, const tc::Optional<tc::io::Path>& cert_path);
private:
	KeyBagInitializer();

	void importBaseKeyFile(const tc::io::Path& keyfile_path, bool isDev);
	void importTitleKeyFile(const tc::io::Path& keyfile_path);
	void importCertificateChain(const tc::io::Path& cert_path);
	void importTicket(const tc::io::Path& tik_path);

	void importKnownKeys(bool isDev);
};

}