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
		// legacy
		bool show_fs_tree;
		std::vector<ExtractJob> extract_jobs;

		// new
		std::vector<ArchiveJob> archive_jobs;
	} fs;


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
		fs.archive_jobs = std::vector<ArchiveJob>();

		kip.extract_path = tc::Optional<tc::io::Path>();

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
	tc::Optional<KeyBag::aes128_key_t> mNcaEncryptedContentKey;
	tc::Optional<KeyBag::aes128_key_t> mNcaContentKey;
	tc::Optional<tc::io::Path> mTikPath;
	tc::Optional<tc::io::Path> mCertPath;

	bool determineValidNcaFromSample(const tc::ByteData& raw_data) const;
	bool determineValidEsCertFromSample(const tc::ByteData& raw_data) const;
	bool determineValidEsTikFromSample(const tc::ByteData& raw_data) const;
	bool determineValidCnmtFromSample(const tc::ByteData& raw_data) const;
	bool determineValidNacpFromSample(const tc::ByteData& raw_data) const;
};

}