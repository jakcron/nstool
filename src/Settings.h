#pragma once
#include "types.h"
#include <string>
#include <vector>
#include <tc/Optional.h>
#include <tc/io.h>

#include "KeyBag.h"

namespace nstool {

struct Settings
{
	enum FileType
	{
		FILE_TYPE_ERROR,
		FILE_TYPE_GAMECARD,
		FILE_TYPE_NSP,
		FILE_TYPE_PARTITIONFS,
		FILE_TYPE_ROMFS,
		FILE_TYPE_NCA,
		FILE_TYPE_META,
		FILE_TYPE_CNMT,
		FILE_TYPE_NSO,
		FILE_TYPE_NRO,
		FILE_TYPE_NACP,
		FILE_TYPE_INI,
		FILE_TYPE_KIP,
		FILE_TYPE_ES_CERT,
		FILE_TYPE_ES_TIK,
		FILE_TYPE_HB_ASSET,
	};

	struct InputFileOptions
	{
		FileType filetype;
		tc::Optional<tc::io::Path> path;
	} infile;

	struct Options
	{
		CliOutputMode cli_output_mode;
		bool verify;
		bool is_dev;
		KeyBag keybag;
	} opt;

	// code options
	struct CodeOptions
	{
		bool list_api;
		bool list_symbols;
		bool is_64bit_instruction; // true=64bit, false=32bit
	} code;

	// Generic FS options
	struct FsOptions 
	{
		bool show_fs_tree;
		std::vector<ExtractJob> extract_jobs;
	} fs;

	// XCI options
	struct XciOptions
	{
		tc::Optional<tc::io::Path> update_extract_path;
		tc::Optional<tc::io::Path> logo_extract_path;
		tc::Optional<tc::io::Path> normal_extract_path;
		tc::Optional<tc::io::Path> secure_extract_path;
	} xci;

	// NCA options
	struct NcaOptions
	{
		tc::Optional<tc::io::Path> part0_extract_path;
		tc::Optional<tc::io::Path> part1_extract_path;
		tc::Optional<tc::io::Path> part2_extract_path;
		tc::Optional<tc::io::Path> part3_extract_path;
		tc::Optional<tc::io::Path> base_nca_path;
	} nca;

	// KIP options
	struct KipOptions
	{
		tc::Optional<tc::io::Path> extract_path;
	} kip;

	// ASET Options
	struct AsetOptions
	{
		tc::Optional<tc::io::Path> icon_extract_path;
		tc::Optional<tc::io::Path> nacp_extract_path;
	} aset;

	Settings()
	{
		infile.filetype = FILE_TYPE_ERROR;
		infile.path = tc::Optional<tc::io::Path>();

		opt.cli_output_mode = CliOutputMode();
		opt.verify = false;
		opt.is_dev = false;
		opt.keybag = KeyBag();

		code.list_api = false;
		code.list_symbols = false;
		code.is_64bit_instruction = true;

		fs.show_fs_tree = false;
		fs.extract_jobs = std::vector<ExtractJob>();

		kip.extract_path = tc::Optional<tc::io::Path>();

		nca.base_nca_path = tc::Optional<tc::io::Path>();

		aset.icon_extract_path = tc::Optional<tc::io::Path>();
		aset.nacp_extract_path = tc::Optional<tc::io::Path>();
	}
};

class SettingsInitializer : public Settings
{
public:
	SettingsInitializer(const std::vector<std::string>& args);
private:
	void parse_args(const std::vector<std::string>& args);
	void determine_filetype();
	void usage_text() const;
	void dump_keys() const;
	void dump_rsa_key(const KeyBag::rsa_key_t& key, const std::string& label, size_t indent, bool expanded_key_data) const;

	std::string mModuleLabel;

	bool mShowLayout;
	bool mShowKeydata;
	bool mVerbose;

	tc::Optional<tc::io::Path> mKeysetPath;
	tc::Optional<tc::io::Path> mTitleKeysetPath;
	tc::Optional<KeyBag::aes128_key_t> mNcaEncryptedContentKey;
	tc::Optional<KeyBag::aes128_key_t> mNcaContentKey;
	std::vector<tc::io::Path> mTikPathList;
	//tc::Optional<tc::io::Path> mTikPath;
	tc::Optional<tc::io::Path> mCertPath;

	void loadKeyFile(tc::Optional<tc::io::Path>& keyfile_path, const std::string& keyfile_name, const std::string& cli_hint);

	bool determineValidNcaFromSample(const tc::ByteData& raw_data) const;
	bool determineValidEsCertFromSample(const tc::ByteData& raw_data) const;
	bool determineValidEsTikFromSample(const tc::ByteData& raw_data) const;
	bool determineValidCnmtFromSample(const tc::ByteData& raw_data) const;
	bool determineValidNacpFromSample(const tc::ByteData& raw_data) const;
};

}