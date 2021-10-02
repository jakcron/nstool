#include "Settings.h"
#include "types.h"
#include "version.h"
#include "util.h"

#include <tc/cli.h>
#include <tc/os/Environment.h>
#include <tc/ArgumentException.h>
#include <tc/io/FileStream.h>
#include <tc/io/StreamSource.h>



#include <nn/hac/ContentArchiveUtil.h>
#include <nn/hac/AesKeygen.h>
#include <nn/hac/define/gc.h>
#include <nn/hac/define/pfs.h>
#include <nn/hac/define/nca.h>
#include <nn/hac/define/meta.h>
#include <nn/hac/define/romfs.h>
#include <nn/hac/define/cnmt.h>
#include <nn/hac/define/nacp.h>
#include <nn/hac/define/nso.h>
#include <nn/hac/define/nro.h>
#include <nn/hac/define/ini.h>
#include <nn/hac/define/kip.h>
#include <nn/hac/define/aset.h>
#include <nn/pki/SignedData.h>
#include <nn/pki/CertificateBody.h>
#include <nn/pki/SignUtils.h>
#include <nn/es/TicketBody_V2.h>


class UnkOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	UnkOptionHandler(const std::string& module_label) : mModuleLabel(module_label)
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		throw tc::InvalidOperationException("getOptionStrings() not defined for UnkOptionHandler.");
	}

	void processOption(const std::string& option, const std::vector<std::string>& params)
	{
		throw tc::Exception(mModuleLabel, "Unrecognized option: \"" + option + "\"");
	}
private:
	std::string mModuleLabel;
};

class DeprecatedOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	DeprecatedOptionHandler(const std::string& warn_message, const std::vector<std::string>& opts) : 
		mWarnMessage(warn_message),
		mOptStrings(opts)
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	void processOption(const std::string& option, const std::vector<std::string>& params)
	{
		fmt::print("[WARNING] Option \"{}\" is deprecated.{}{}\n", option, (mWarnMessage.empty() ? "" : " "), mWarnMessage);
	}
private:
	std::string mWarnMessage;
	std::vector<std::string> mOptStrings;
};

class FlagOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	FlagOptionHandler(bool& flag, const std::vector<std::string>& opts) : 
		mFlag(flag),
		mOptStrings(opts)
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	void processOption(const std::string& option, const std::vector<std::string>& params)
	{
		if (params.size() != 0)
		{
			throw tc::ArgumentOutOfRangeException(fmt::format("Option \"{:s}\" is a flag, that takes no parameters.", option));
		}

		mFlag = true;
	}
private:
	bool& mFlag;
	std::vector<std::string> mOptStrings;
};

class SingleParamStringOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	SingleParamStringOptionHandler(tc::Optional<std::string>& param, const std::vector<std::string>& opts) : 
		mParam(param),
		mOptStrings(opts)
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	void processOption(const std::string& option, const std::vector<std::string>& params)
	{
		if (params.size() != 1)
		{
			throw tc::ArgumentOutOfRangeException(fmt::format("Option \"{:s}\" requires a parameter.", option));
		}

		mParam = params[0];
	}
private:
	tc::Optional<std::string>& mParam;
	std::vector<std::string> mOptStrings;
};

class SingleParamPathOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	SingleParamPathOptionHandler(tc::Optional<tc::io::Path>& param, const std::vector<std::string>& opts) : 
		mParam(param),
		mOptStrings(opts)
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	void processOption(const std::string& option, const std::vector<std::string>& params)
	{
		if (params.size() != 1)
		{
			throw tc::ArgumentOutOfRangeException(fmt::format("Option \"{:s}\" requires a parameter.", option));
		}

		mParam = params[0];
	}
private:
	tc::Optional<tc::io::Path>& mParam;
	std::vector<std::string> mOptStrings;
};

class SingleParamSizetOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	SingleParamSizetOptionHandler(size_t& param, const std::vector<std::string>& opts) : 
		mParam(param),
		mOptStrings(opts)
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	void processOption(const std::string& option, const std::vector<std::string>& params)
	{
		if (params.size() != 1)
		{
			throw tc::ArgumentOutOfRangeException(fmt::format("Option \"{:s}\" requires a parameter.", option));
		}

		mParam = strtoul(params[0].c_str(), nullptr, 0);
	}
private:
	size_t& mParam;
	std::vector<std::string> mOptStrings;
};

class SingleParamAesKeyOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	SingleParamAesKeyOptionHandler(tc::Optional<nstool::KeyBag::aes128_key_t>& param, const std::vector<std::string>& opts) :
		mParam(param),
		mOptStrings(opts)
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	void processOption(const std::string& option, const std::vector<std::string>& params)
	{
		if (params.size() != 1)
		{
			throw tc::ArgumentOutOfRangeException(fmt::format("Option \"{:s}\" requires a parameter.", option));
		}

		tc::ByteData key_raw = tc::cli::FormatUtil::hexStringToBytes(params[0]);
		if (key_raw.size() != sizeof(nstool::KeyBag::aes128_key_t))
		{
			throw tc::ArgumentOutOfRangeException(fmt::format("Option: \"{:s}\", requires an AES128 key as the parameter (must be 32 hex chars).", option));
		}

		nstool::KeyBag::aes128_key_t key_tmp;
		memcpy(key_tmp.data(), key_raw.data(), key_tmp.size());

		mParam = key_tmp;
	}
private:
	tc::Optional<nstool::KeyBag::aes128_key_t>& mParam;
	std::vector<std::string> mOptStrings;
};

class FileTypeOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	FileTypeOptionHandler(nstool::Settings::FileType& param, const std::vector<std::string>& opts) : 
		mParam(param),
		mOptStrings(opts)
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	void processOption(const std::string& option, const std::vector<std::string>& params)
	{
		if (params.size() != 1)
		{
			throw tc::ArgumentOutOfRangeException(fmt::format("Option \"{:s}\" requires a parameter.", option));
		}

		if (params[0] == "gc" \
		 || params[0] == "gamecard" \
		 || params[0] == "xci" \
		 || params[0] == "xcie" \
		 || params[0] == "xcir")
		{
			mParam = nstool::Settings::FILE_TYPE_GAMECARD;
		}
		else if (params[0] == "nsp")
		{
			mParam = nstool::Settings::FILE_TYPE_NSP;
		}
		else if (params[0] == "partitionfs" || params[0] == "hashedpartitionfs" \
		 || params[0] == "pfs" || params[0] == "pfs0"  \
		 || params[0] == "hfs" || params[0] == "hfs0")
		{
			mParam = nstool::Settings::FILE_TYPE_PARTITIONFS;
		}
		else if (params[0] == "romfs")
		{
			mParam = nstool::Settings::FILE_TYPE_ROMFS;
		}
		else if (params[0] == "nca" || params[0] == "contentarchive")
		{
			mParam = nstool::Settings::FILE_TYPE_NCA;
		}
		else if (params[0] == "meta" || params[0] == "npdm")
		{
			mParam = nstool::Settings::FILE_TYPE_META;
		}
		else if (params[0] == "cnmt")
		{
			mParam = nstool::Settings::FILE_TYPE_CNMT;
		}
		else if (params[0] == "nso")
		{
			mParam = nstool::Settings::FILE_TYPE_NSO;
		}
		else if (params[0] == "nro")
		{
			mParam = nstool::Settings::FILE_TYPE_NRO;
		}
		else if (params[0] == "ini")
		{
			mParam = nstool::Settings::FILE_TYPE_INI;
		}
		else if (params[0] == "kip")
		{
			mParam = nstool::Settings::FILE_TYPE_KIP;
		}
		else if (params[0] == "nacp")
		{
			mParam = nstool::Settings::FILE_TYPE_NACP;
		}
		else if (params[0] == "cert")
		{
			mParam = nstool::Settings::FILE_TYPE_PKI_CERT;
		}
		else if (params[0] == "tik")
		{
			mParam = nstool::Settings::FILE_TYPE_ES_TIK;
		}
		else if (params[0] == "aset" || params[0] == "asset")
		{
			mParam = nstool::Settings::FILE_TYPE_HB_ASSET;
		}
		else
		{
			throw tc::ArgumentException(fmt::format("File type \"{}\" unrecognised.", params[0]));
		}
	}
private:
	nstool::Settings::FileType& mParam;
	std::vector<std::string> mOptStrings;
};

class InstructionTypeOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	InstructionTypeOptionHandler(bool& param, const std::vector<std::string>& opts) :
		mParam(param),
		mOptStrings(opts)
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	void processOption(const std::string& option, const std::vector<std::string>& params)
	{
		if (params.size() != 1)
		{
			throw tc::ArgumentOutOfRangeException(fmt::format("Option \"{:s}\" requires a parameter.", option));
		}

		if (params[0] == "32bit")
		{
			mParam = false;
		}
		else if (params[0] == "64bit")
		{
			mParam = true;
		}
		else
		{
			throw tc::ArgumentException(fmt::format("Instruction type \"{}\" unrecognised. Try \"32bit\" or \"64bit\"", params[0]));
		}
	}
private:
	bool& mParam;
	std::vector<std::string> mOptStrings;
};

nstool::SettingsInitializer::SettingsInitializer(const std::vector<std::string>& args) :
	Settings(),
	mModuleLabel("nstool::SettingsInitializer"),
	mShowLayout(false),
	mShowKeydata(false),
	mVerbose(false),
	mTitleKey(),
	mBodyKey(),
	mTikPath(),
	mCertPath()
{
	// parse input arguments
	parse_args(args);
	if (infile.path.isNull())
		throw tc::ArgumentException(mModuleLabel, "No input file was specified.");

	// determine CLI output mode
	opt.cli_output_mode.show_basic_info = true;
	if (mVerbose)
	{
		opt.cli_output_mode.show_extended_info = true;
		opt.cli_output_mode.show_layout = true;
		opt.cli_output_mode.show_keydata = true;
	}
	if (mShowKeydata)
	{
		opt.cli_output_mode.show_keydata = true;
	}
	if (mShowLayout)
	{
		opt.cli_output_mode.show_layout = true;
	}

	// locate key file, if not specfied
	if (mKeysetPath.isNull())
	{
		std::string home_path_str;
		if (tc::os::getEnvVar("HOME", home_path_str) || tc::os::getEnvVar("USERPROFILE", home_path_str))
		{
			tc::io::Path keyfile_path = tc::io::Path(home_path_str);
			keyfile_path.push_back(".switch");
			keyfile_path.push_back(opt.is_dev ? "dev.keys" : "prod.keys");

			try {
				tc::io::FileStream test = tc::io::FileStream(keyfile_path, tc::io::FileMode::Open, tc::io::FileAccess::Read);
				
				mKeysetPath = keyfile_path;
			}
			catch (tc::io::FileNotFoundException&) {
				fmt::print("[WARNING] Failed to load \"{}\" keyfile. Maybe specify it with \"-k <path>\"?\n", opt.is_dev ? "dev.keys" : "prod.keys");
			}
		}
		else {
			fmt::print("[WARNING] Failed to located \"{}\" keyfile. Maybe specify it with \"-k <path>\"?\n", opt.is_dev ? "dev.keys" : "prod.keys");
		}
	}

	// generate keybag
	opt.keybag = KeyBagInitializer(opt.is_dev, mKeysetPath, mTikPath, mCertPath);
	opt.keybag.fallback_enc_content_key = mTitleKey;
	opt.keybag.fallback_content_key = mBodyKey;

	// dump keys if requires
	if (opt.cli_output_mode.show_keydata)
	{
		dump_keys();
	}

	// determine filetype if not manually specified
	if (infile.filetype == FILE_TYPE_ERROR)
	{
		determine_filetype();
		if (infile.filetype == FILE_TYPE_ERROR)
		{
			throw tc::ArgumentException(mModuleLabel, "Input file type was undetermined.");
		}
	}
}

void nstool::SettingsInitializer::parse_args(const std::vector<std::string>& args)
{
	// check for minimum arguments
	if (args.size() < 2)
	{
		usage_text();
		throw tc::ArgumentException(mModuleLabel, "Not enough arguments.");
	}
	
	// detect request for help
	for (auto itr = ++(args.begin()); itr != args.end(); itr++)
	{
		if (*itr == "-h" || *itr == "--help" || *itr == "-help")
		{
			usage_text();
			throw tc::ArgumentException(mModuleLabel, "Help required.");
		}
	}

	// save input file
	infile.path = tc::io::Path(args.back());

	// test new option parser
	tc::cli::OptionParser opts;

	// register unk option handler
	opts.registerUnrecognisedOptionHandler(std::shared_ptr<UnkOptionHandler>(new UnkOptionHandler(mModuleLabel)));

	// register handler for deprecated options DeprecatedOptionHandler
	// none just yet

	// get option flags
	opts.registerOptionHandler(std::shared_ptr<FlagOptionHandler>(new FlagOptionHandler(mShowLayout, {"--showlayout"})));
	opts.registerOptionHandler(std::shared_ptr<FlagOptionHandler>(new FlagOptionHandler(mShowKeydata, { "--showkeys" })));
	opts.registerOptionHandler(std::shared_ptr<FlagOptionHandler>(new FlagOptionHandler(mVerbose, {"-v", "--verbose"})));
	opts.registerOptionHandler(std::shared_ptr<FlagOptionHandler>(new FlagOptionHandler(opt.verify, {"-y", "--verify"})));
	opts.registerOptionHandler(std::shared_ptr<FlagOptionHandler>(new FlagOptionHandler(opt.is_dev, {"-d", "--dev"})));

	// process input file type
	opts.registerOptionHandler(std::shared_ptr<FileTypeOptionHandler>(new FileTypeOptionHandler(infile.filetype, { "-t", "--intype" })));

	// get user-provided keydata
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(mKeysetPath, {"-k", "--keyset"})));
	opts.registerOptionHandler(std::shared_ptr<SingleParamAesKeyOptionHandler>(new SingleParamAesKeyOptionHandler(mTitleKey, {"--titlekey"})));
	opts.registerOptionHandler(std::shared_ptr<SingleParamAesKeyOptionHandler>(new SingleParamAesKeyOptionHandler(mBodyKey, {"--bodykey"})));
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(mTikPath, {"--tik"})));
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(mCertPath, {"--cert"})));

	// code options
	opts.registerOptionHandler(std::shared_ptr<FlagOptionHandler>(new FlagOptionHandler(code.list_api, { "--listapi" })));
	opts.registerOptionHandler(std::shared_ptr<FlagOptionHandler>(new FlagOptionHandler(code.list_symbols, { "--listsym" })));
	opts.registerOptionHandler(std::shared_ptr<InstructionTypeOptionHandler>(new InstructionTypeOptionHandler(code.is_64bit_instruction, { "--insttype" })));

	// fs options
	opts.registerOptionHandler(std::shared_ptr<FlagOptionHandler>(new FlagOptionHandler(fs.show_fs_tree, { "--listfs" })));
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(fs.extract_path, { "--fsdir" })));

	// xci options
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(xci.update_extract_path, { "--update" })));
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(xci.normal_extract_path, { "--normal" })));
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(xci.secure_extract_path, { "--secure" })));
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(xci.logo_extract_path, { "--logo" })));

	// nca options
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(nca.part0_extract_path, { "--part0" })));
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(nca.part1_extract_path, { "--part1" })));
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(nca.part2_extract_path, { "--part2" })));
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(nca.part3_extract_path, { "--part3" })));

	// kip options
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(kip.extract_path, { "--kipdir" })));
	
	// aset options
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(aset.icon_extract_path, { "--icon" })));
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(aset.nacp_extract_path, { "--nacp" })));

	
	// process option
	opts.processOptions(args, 1, args.size() - 2);
}

void nstool::SettingsInitializer::determine_filetype()
{
	//std::string infile_path_str;
	//tc::io::PathUtil::pathToUnixUTF8(infile.path.get(), infile_path_str);
	//fmt::print("infile path = \"{}\"\n", infile_path_str);
	
	auto file = tc::io::StreamSource(std::make_shared<tc::io::FileStream>(tc::io::FileStream(infile.path.get(), tc::io::FileMode::Open, tc::io::FileAccess::Read)));

	auto raw_data = file.pullData(0, 0x5000);

#define _TYPE_PTR(st) ((st*)(raw_data.data()))
#define _ASSERT_FILE_SIZE(sz) (file.length() >= (sz))

	// do easy tests

	// detect "scene" XCI
	if (_ASSERT_FILE_SIZE(sizeof(nn::hac::sGcHeader_Rsa2048Signed))
	 && _TYPE_PTR(nn::hac::sGcHeader_Rsa2048Signed)->header.st_magic.unwrap() == nn::hac::gc::kGcHeaderStructMagic)
	{
		infile.filetype = FILE_TYPE_GAMECARD;
	}
	// detect "SDK" XCI
	else if (_ASSERT_FILE_SIZE(sizeof(nn::hac::sSdkGcHeader))
		&& _TYPE_PTR(nn::hac::sSdkGcHeader)->signed_header.header.st_magic.unwrap() == nn::hac::gc::kGcHeaderStructMagic)
	{
		infile.filetype = FILE_TYPE_GAMECARD;
	}
	// detect PFS0
	else if (_ASSERT_FILE_SIZE(sizeof(nn::hac::sPfsHeader))
	      && _TYPE_PTR(nn::hac::sPfsHeader)->st_magic.unwrap() == nn::hac::pfs::kPfsStructMagic)
	{
		infile.filetype = FILE_TYPE_PARTITIONFS;
	}
	// detect HFS0
	else if (_ASSERT_FILE_SIZE(sizeof(nn::hac::sPfsHeader))
		&& _TYPE_PTR(nn::hac::sPfsHeader)->st_magic.unwrap() == nn::hac::pfs::kHashedPfsStructMagic)
	{
		infile.filetype = FILE_TYPE_PARTITIONFS;
	}
	// detect ROMFS
	else if (_ASSERT_FILE_SIZE(sizeof(nn::hac::sRomfsHeader))
		&& _TYPE_PTR(nn::hac::sRomfsHeader)->header_size.unwrap() == sizeof(nn::hac::sRomfsHeader)
		&& _TYPE_PTR(nn::hac::sRomfsHeader)->sections[1].offset.unwrap() == (_TYPE_PTR(nn::hac::sRomfsHeader)->sections[0].offset.unwrap() + _TYPE_PTR(nn::hac::sRomfsHeader)->sections[0].size.unwrap()))
	{
		infile.filetype = FILE_TYPE_ROMFS;
	}
	// detect NPDM
	else if (_ASSERT_FILE_SIZE(sizeof(nn::hac::sMetaHeader))
		&& _TYPE_PTR(nn::hac::sMetaHeader)->st_magic.unwrap() == nn::hac::meta::kMetaStructMagic)
	{
		infile.filetype = FILE_TYPE_META;
	}
	// detect NSO
	else if (_ASSERT_FILE_SIZE(sizeof(nn::hac::sNsoHeader))
		&& _TYPE_PTR(nn::hac::sNsoHeader)->st_magic.unwrap() == nn::hac::nso::kNsoStructMagic)
	{
		infile.filetype = FILE_TYPE_NSO;
	}
	// detect NRO
	else if (_ASSERT_FILE_SIZE(sizeof(nn::hac::sNroHeader))
		&& _TYPE_PTR(nn::hac::sNroHeader)->st_magic.unwrap() == nn::hac::nro::kNroStructMagic)
	{
		infile.filetype = FILE_TYPE_NRO;
	}
	// detect INI
	else if (_ASSERT_FILE_SIZE(sizeof(nn::hac::sIniHeader))
		&& _TYPE_PTR(nn::hac::sIniHeader)->st_magic.unwrap() == nn::hac::ini::kIniStructMagic)
	{
		infile.filetype = FILE_TYPE_INI;
	}
	// detect KIP
	else if (_ASSERT_FILE_SIZE(sizeof(nn::hac::sKipHeader))
		&& _TYPE_PTR(nn::hac::sKipHeader)->st_magic.unwrap() == nn::hac::kip::kKipStructMagic)
	{
		infile.filetype = FILE_TYPE_KIP;
	}
	// detect HB ASET
	else if (_ASSERT_FILE_SIZE(sizeof(nn::hac::sAssetHeader))
		&& _TYPE_PTR(nn::hac::sAssetHeader)->st_magic.unwrap() == nn::hac::aset::kAssetStructMagic)
	{
		infile.filetype = FILE_TYPE_KIP;
	}

	// more complicated tests

	// detect NCA
	else if (determineValidNcaFromSample(raw_data))
	{
		infile.filetype = FILE_TYPE_NCA;
	}
	// detect Certificate
	else if (determineValidEsCertFromSample(raw_data))
	{
		infile.filetype = FILE_TYPE_PKI_CERT;
	}
	// detect Ticket
	else if (determineValidEsTikFromSample(raw_data))
	{
		infile.filetype = FILE_TYPE_ES_TIK;
	}
	// detect Ticket
	else if (determineValidCnmtFromSample(raw_data))
	{
		infile.filetype = FILE_TYPE_CNMT;
	}
	// detect Ticket
	else if (determineValidNacpFromSample(raw_data))
	{
		infile.filetype = FILE_TYPE_NACP;
	}
#undef _TYPE_PTR
#undef _ASSERT_FILE_SIZE
}

void nstool::SettingsInitializer::usage_text() const
{
	fmt::print("{:s} v{:d}.{:d}.{:d} (C) {:s}\n", APP_NAME, VER_MAJOR, VER_MINOR, VER_PATCH, AUTHORS);
	fmt::print("Built: {:s} {:s}\n\n", __TIME__, __DATE__);
	fmt::print("Usage: {:s} [options... ] <file>\n", BIN_NAME);
	fmt::print("\n  General Options:\n");
	fmt::print("      -d, --dev       Use devkit keyset.\n");
	fmt::print("      -k, --keyset    Specify keyset file.\n");
	fmt::print("      -t, --type      Specify input file type. [xci, pfs, romfs, nca, meta, cnmt, nso, nro, ini, kip, nacp, aset, cert, tik]\n");
	fmt::print("      -y, --verify    Verify file.\n");
	fmt::print("\n  Output Options:\n");
	fmt::print("      --showkeys      Show keys generated.\n");
	fmt::print("      --showlayout    Show layout metadata.\n");
	fmt::print("      -v, --verbose   Verbose output.\n");
	fmt::print("\n  XCI (GameCard Image)\n");
	fmt::print("    {:s} [--listfs] [--update <dir> --logo <dir> --normal <dir> --secure <dir>] <.xci file>\n", BIN_NAME);
	fmt::print("      --listfs        Print file system in embedded partitions.\n");
	fmt::print("      --update        Extract \"update\" partition to directory.\n");
	fmt::print("      --logo          Extract \"logo\" partition to directory.\n");
	fmt::print("      --normal        Extract \"normal\" partition to directory.\n");
	fmt::print("      --secure        Extract \"secure\" partition to directory.\n");
	fmt::print("\n  PFS0/HFS0 (PartitionFs), RomFs, NSP (Ninendo Submission Package)\n");
	fmt::print("    {:s} [--listfs] [--fsdir <dir>] <file>\n", BIN_NAME);
	fmt::print("      --listfs        Print file system.\n");
	fmt::print("      --fsdir         Extract file system to directory.\n");
	fmt::print("\n  NCA (Nintendo Content Archive)\n");
	fmt::print("    {:s} [--listfs] [--bodykey <key> --titlekey <key>] [--part0 <dir> ...] <.nca file>\n", BIN_NAME);
	fmt::print("      --listfs        Print file system in embedded partitions.\n");
	fmt::print("      --titlekey      Specify title key extracted from ticket.\n");
	fmt::print("      --bodykey       Specify body encryption key.\n");
	fmt::print("      --tik           Specify ticket to source title key.\n");
	fmt::print("      --cert          Specify certificate chain to verify ticket.\n");
	fmt::print("      --part0         Extract \"partition 0\" to directory.\n");
	fmt::print("      --part1         Extract \"partition 1\" to directory.\n");
	fmt::print("      --part2         Extract \"partition 2\" to directory.\n");
	fmt::print("      --part3         Extract \"partition 3\" to directory.\n");
	fmt::print("\n  NSO (Nintendo Software Object), NRO (Nintendo Relocatable Object)\n");
	fmt::print("    {:s} [--listapi --listsym] [--insttype <inst. type>] <file>\n", BIN_NAME);
	fmt::print("      --listapi       Print SDK API List.\n");
	fmt::print("      --listsym       Print Code Symbols.\n");
	fmt::print("      --insttype      Specify instruction type [64bit|32bit] (64bit is assumed).\n");
	fmt::print("\n  INI (Initial Process List Blob)\n");
	fmt::print("    {:s} [--kipdir <dir>] <file>\n", BIN_NAME);
	fmt::print("      --kipdir        Extract embedded KIPs to directory.\n");
	fmt::print("\n  ASET (Homebrew Asset Blob)\n");
	fmt::print("    {:s} [--listfs] [--icon <file> --nacp <file> --fsdir <dir>] <file>\n", BIN_NAME);
	fmt::print("      --listfs        Print filesystem in embedded RomFS partition.\n");
	fmt::print("      --icon          Extract icon partition to file.\n");
	fmt::print("      --nacp          Extract NACP partition to file.\n");
	fmt::print("      --fsdir         Extract RomFS partition to directory.\n");
}

void nstool::SettingsInitializer::dump_keys() const
{
	fmt::print("[KeyConfiguration]\n");
	fmt::print("  NCA Keys:\n");
	for (auto itr = opt.keybag.nca_header_sign0_key.begin(); itr != opt.keybag.nca_header_sign0_key.end(); itr++)
	{
		dump_rsa_key(itr->second, fmt::format("Header0-SignatureKey-{:02x}", itr->first), 4, opt.cli_output_mode.show_extended_info);
	}
	for (auto itr = opt.keybag.acid_sign_key.begin(); itr != opt.keybag.acid_sign_key.end(); itr++)
	{
		dump_rsa_key(itr->second, fmt::format("Acid-SignatureKey-{:02x}", itr->first), 4, opt.cli_output_mode.show_extended_info);
	}
	if (opt.keybag.nca_header_key.isSet())
	{
		fmt::print("    Header-EncryptionKey:\n");
		fmt::print("      Key0: {:s}\n", tc::cli::FormatUtil::formatBytesAsString(opt.keybag.nca_header_key.get()[0].data(), opt.keybag.nca_header_key.get()[0].size(), true, ":"));
		fmt::print("      Key1: {:s}\n", tc::cli::FormatUtil::formatBytesAsString(opt.keybag.nca_header_key.get()[1].data(), opt.keybag.nca_header_key.get()[1].size(), true, ":"));
	}
	std::vector<std::string> kaek_label = {"Application", "Ocean", "System"};
	for (size_t kaek_index = 0; kaek_index < opt.keybag.nca_key_area_encryption_key.size(); kaek_index++)
	{
		for (auto itr = opt.keybag.nca_key_area_encryption_key[kaek_index].begin(); itr != opt.keybag.nca_key_area_encryption_key[kaek_index].end(); itr++)
		{
			fmt::print("    KeyAreaEncryptionKey-{:s}-{:02x}:\n      {:s}\n", kaek_label[kaek_index], itr->first, tc::cli::FormatUtil::formatBytesAsString(itr->second.data(), itr->second.size(), true, ":"));
		}
	}
	for (size_t kaek_index = 0; kaek_index < opt.keybag.nca_key_area_encryption_key_hw.size(); kaek_index++)
	{
		for (auto itr = opt.keybag.nca_key_area_encryption_key_hw[kaek_index].begin(); itr != opt.keybag.nca_key_area_encryption_key_hw[kaek_index].end(); itr++)
		{
			fmt::print("    KeyAreaEncryptionKeyHw-{:s}-{:02x}:\n      {:s}\n", kaek_label[kaek_index], itr->first, tc::cli::FormatUtil::formatBytesAsString(itr->second.data(), itr->second.size(), true, ":"));
		}
	}
	fmt::print("  NRR Keys:\n");
	for (auto itr = opt.keybag.nrr_certificate_sign_key.begin(); itr != opt.keybag.nrr_certificate_sign_key.end(); itr++)
	{
		dump_rsa_key(itr->second, fmt::format("Certificate-SignatureKey-{:02x}", itr->first), 4, opt.cli_output_mode.show_extended_info);
	}
	fmt::print("  XCI Keys:\n");
	if (opt.keybag.xci_header_sign_key.isSet())
	{
		dump_rsa_key(opt.keybag.xci_header_sign_key.get(), fmt::format("Header-SignatureKey"), 4, opt.cli_output_mode.show_extended_info);
	}
	for (auto itr = opt.keybag.xci_header_key.begin(); itr != opt.keybag.xci_header_key.end(); itr++)
	{
		fmt::print("    ExtendedHeader-EncryptionKey-{:02x}:\n      {:s}\n", itr->first, tc::cli::FormatUtil::formatBytesAsString(itr->second.data(), itr->second.size(), true, ":"));
	}
	if (opt.keybag.xci_cert_sign_key.isSet())
	{
		dump_rsa_key(opt.keybag.xci_cert_sign_key.get(), fmt::format("CERT-SignatureKey"), 4, opt.cli_output_mode.show_extended_info);
	}

	fmt::print("  Package1 Keys:\n");
	for (auto itr = opt.keybag.pkg1_key.begin(); itr != opt.keybag.pkg1_key.end(); itr++)
	{
		fmt::print("    EncryptionKey-{:02x}:\n      {:s}\n", itr->first, tc::cli::FormatUtil::formatBytesAsString(itr->second.data(), itr->second.size(), true, ":"));
	}

	fmt::print("  Package2 Keys:\n");
	if (opt.keybag.pkg2_sign_key.isSet())
	{
		dump_rsa_key(opt.keybag.pkg2_sign_key.get(), fmt::format("Header-SignatureKey"), 4, opt.cli_output_mode.show_extended_info);
	}
	for (auto itr = opt.keybag.pkg2_key.begin(); itr != opt.keybag.pkg2_key.end(); itr++)
	{
		fmt::print("    EncryptionKey-{:02x}:\n      {:s}\n", itr->first, tc::cli::FormatUtil::formatBytesAsString(itr->second.data(), itr->second.size(), true, ":"));
	}

	fmt::print("  ETicket Keys:\n");
	for (auto itr = opt.keybag.etik_common_key.begin(); itr != opt.keybag.etik_common_key.end(); itr++)
	{
		fmt::print("    CommonKey-{:02x}:\n      {:s}\n", itr->first, tc::cli::FormatUtil::formatBytesAsString(itr->second.data(), itr->second.size(), true, ":"));
	}

	fmt::print("  BroadOn Signer Profiles:\n");
	for (auto itr = opt.keybag.broadon_signer.begin(); itr != opt.keybag.broadon_signer.end(); itr++)
	{
		fmt::print("    {:s}:\n", itr->first);
		fmt::print("      SignType: ");
		switch(itr->second.key_type) {
			case nn::pki::sign::SIGN_ALGO_RSA2048:
				fmt::print("RSA-2048\n");
				break;
			case nn::pki::sign::SIGN_ALGO_RSA4096:
				fmt::print("RSA-4096\n");
				break;
			case nn::pki::sign::SIGN_ALGO_ECDSA240:
				fmt::print("ECDSA-240\n");
				break;
			default:
				fmt::print("Unknown\n");
		}
		switch(itr->second.key_type) {
			case nn::pki::sign::SIGN_ALGO_RSA2048:
			case nn::pki::sign::SIGN_ALGO_RSA4096:
				dump_rsa_key(itr->second.rsa_key, "RsaKey", 6, opt.cli_output_mode.show_extended_info);
				break;
			case nn::pki::sign::SIGN_ALGO_ECDSA240:
			default:
				break;
		}
	}
}

void nstool::SettingsInitializer::dump_rsa_key(const KeyBag::rsa_key_t& key, const std::string& label, size_t indent, bool expanded_key_data) const
{
	std::string indent_str;

	indent_str.clear();
	for (size_t i = 0; i < indent; i++)
	{
		indent_str += " ";
	}

	fmt::print("{:s}{:s}:\n", indent_str, label);
	if (key.n.size() > 0)
	{
		if (expanded_key_data)
		{
			fmt::print("{:s}  Modulus:\n", indent_str);
			fmt::print("{:s}    {:s}", indent_str, tc::cli::FormatUtil::formatBytesAsStringWithLineLimit(key.n.data(), key.n.size(), true, ":", 0x10, indent + 4, false));
		}
		else
		{
			fmt::print("{:s}  Modulus: {:s}\n", indent_str, getTruncatedBytesString(key.n.data(), key.n.size()));
		}
	}
	if (key.d.size() > 0)
	{
		if (expanded_key_data)
		{
			fmt::print("{:s}  Private Exponent:\n", indent_str);
			fmt::print("{:s}    {:s}", indent_str, tc::cli::FormatUtil::formatBytesAsStringWithLineLimit(key.d.data(), key.d.size(), true, ":", 0x10, indent + 4, false));
		}
		else
		{
			fmt::print("{:s}  Private Exponent: {:s}\n", indent_str, getTruncatedBytesString(key.d.data(), key.d.size()));
		}
	}
}


bool nstool::SettingsInitializer::determineValidNcaFromSample(const tc::ByteData& sample) const
{
	if (sample.size() < nn::hac::nca::kHeaderSize)
	{
		return false;
	}
	
	if (opt.keybag.nca_header_key.isNull())
	{
		fmt::print("[WARNING] Failed to load NCA Header Key.\n");
		return false;
	}

	nn::hac::detail::aes128_xtskey_t key = opt.keybag.nca_header_key.get();

	//fmt::print("NCA header key: {} {}\n", tc::cli::FormatUtil::formatBytesAsString(opt.keybag.nca_header_key.get()[0].data(), opt.keybag.nca_header_key.get()[0].size(), true, ""), tc::cli::FormatUtil::formatBytesAsString(opt.keybag.nca_header_key.get()[1].data(), opt.keybag.nca_header_key.get()[1].size(), true, ""));

	// init aes-xts
	tc::crypto::Aes128XtsEncryptor enc;
	enc.initialize(key[0].data(), key[0].size(), key[1].data(), key[1].size(), nn::hac::nca::kSectorSize, false);

	// decrypt main header
	byte_t raw_hdr[nn::hac::nca::kSectorSize];
	enc.decrypt(raw_hdr, sample.data() + nn::hac::ContentArchiveUtil::sectorToOffset(1), nn::hac::nca::kSectorSize, 1);
	nn::hac::sContentArchiveHeader* hdr = (nn::hac::sContentArchiveHeader*)(raw_hdr);

	/*
	fmt::print("NCA Header Raw:\n");
	fmt::print("{:s}\n", tc::cli::FormatUtil::formatBytesAsHxdHexString(sample.data() + nn::hac::ContentArchiveUtil::sectorToOffset(1), nn::hac::nca::kSectorSize));
	fmt::print("{:s}\n", tc::cli::FormatUtil::formatBytesAsHxdHexString(raw_hdr, nn::hac::nca::kSectorSize));
	*/

	if (hdr->st_magic.unwrap() != nn::hac::nca::kNca2StructMagic && hdr->st_magic.unwrap() != nn::hac::nca::kNca3StructMagic)
	{
		return false;
	}

	return true;
}

bool nstool::SettingsInitializer::determineValidCnmtFromSample(const tc::ByteData& sample) const
{
	if (sample.size() < sizeof(nn::hac::sContentMetaHeader))
		return false;

	const nn::hac::sContentMetaHeader* data = (const nn::hac::sContentMetaHeader*)sample.data();

	size_t minimum_size = sizeof(nn::hac::sContentMetaHeader) + data->exhdr_size.unwrap() + data->content_count.unwrap() * sizeof(nn::hac::sContentInfo) + data->content_meta_count.unwrap() * sizeof(nn::hac::sContentMetaInfo) + nn::hac::cnmt::kDigestLen;

	if (sample.size() < minimum_size)
		return false;

	// include exthdr/data check if applicable
	if (data->exhdr_size.unwrap() > 0)
	{
		if (data->type == (byte_t)nn::hac::cnmt::ContentMetaType::Application)
		{
			const nn::hac::sApplicationMetaExtendedHeader* meta = (const nn::hac::sApplicationMetaExtendedHeader*)(sample.data() + sizeof(nn::hac::sContentMetaHeader));
			if ((meta->patch_id.unwrap() & data->id.unwrap()) != data->id.unwrap())
				return false;
		}
		else if (data->type == (byte_t)nn::hac::cnmt::ContentMetaType::Patch)
		{
			const nn::hac::sPatchMetaExtendedHeader* meta = (const nn::hac::sPatchMetaExtendedHeader*)(sample.data() + sizeof(nn::hac::sContentMetaHeader));
			if ((meta->application_id.unwrap() & data->id.unwrap()) != meta->application_id.unwrap())
				return false;

			minimum_size += meta->extended_data_size.unwrap();
		}
		else if (data->type == (byte_t)nn::hac::cnmt::ContentMetaType::AddOnContent)
		{
			const nn::hac::sAddOnContentMetaExtendedHeader* meta = (const nn::hac::sAddOnContentMetaExtendedHeader*)(sample.data() + sizeof(nn::hac::sContentMetaHeader));
			if ((meta->application_id.unwrap() & data->id.unwrap()) != meta->application_id.unwrap())
				return false;
		}
		else if (data->type == (byte_t)nn::hac::cnmt::ContentMetaType::Delta)
		{
			const nn::hac::sDeltaMetaExtendedHeader* meta = (const nn::hac::sDeltaMetaExtendedHeader*)(sample.data() + sizeof(nn::hac::sContentMetaHeader));
			if ((meta->application_id.unwrap() & data->id.unwrap()) != meta->application_id.unwrap())
				return false;

			minimum_size += meta->extended_data_size.unwrap();
		}
		else if (data->type == (byte_t)nn::hac::cnmt::ContentMetaType::SystemUpdate)
		{
			const nn::hac::sSystemUpdateMetaExtendedHeader* meta = (const nn::hac::sSystemUpdateMetaExtendedHeader*)(sample.data() + sizeof(nn::hac::sContentMetaHeader));

			minimum_size += meta->extended_data_size.unwrap();
		}
	}

	if (sample.size() != minimum_size)
		return false;

	return true;
}

bool nstool::SettingsInitializer::determineValidNacpFromSample(const tc::ByteData& sample) const
{
	if (sample.size() != sizeof(nn::hac::sApplicationControlProperty))
		return false;

	const nn::hac::sApplicationControlProperty* data = (const nn::hac::sApplicationControlProperty*)sample.data();

	if (data->logo_type > (byte_t)nn::hac::nacp::LogoType::Nintendo)
		return false;

	if (data->display_version[0] == 0)
		return false;

	if (data->user_account_save_data_size.unwrap() == 0 && data->user_account_save_data_journal_size.unwrap() != 0)
		return false;

	if (data->user_account_save_data_journal_size.unwrap() == 0 && data->user_account_save_data_size.unwrap() != 0)
		return false;

	if (*((uint32_t*)(&data->supported_language_flag)) == 0)
		return false;

	return true;
}

bool nstool::SettingsInitializer::determineValidEsCertFromSample(const tc::ByteData& sample) const
{
	nn::pki::SignatureBlock sign;

	try
	{
		sign.fromBytes(sample.data(), sample.size());
	}
	catch (...)
	{
		return false;
	}

	if (sign.isLittleEndian() == true)
		return false;

	if (sign.getSignType() != nn::pki::sign::SIGN_ID_RSA4096_SHA256 && sign.getSignType() != nn::pki::sign::SIGN_ID_RSA2048_SHA256 && sign.getSignType() != nn::pki::sign::SIGN_ID_ECDSA240_SHA256)
		return false;

	return true;
}

bool nstool::SettingsInitializer::determineValidEsTikFromSample(const tc::ByteData& sample) const
{
	nn::pki::SignatureBlock sign;

	try
	{
		sign.fromBytes(sample.data(), sample.size());
	}
	catch (...)
	{
		return false;
	}

	if (sign.isLittleEndian() == false)
		return false;

	if (sign.getSignType() != nn::pki::sign::SIGN_ID_RSA2048_SHA256)
		return false;

	const nn::es::sTicketBody_v2* body = (const nn::es::sTicketBody_v2*)(sample.data() + sign.getBytes().size());

	if ((body->issuer.str().substr(0, 5) == "Root-"
		&& body->issuer.str().substr(16, 2) == "XS") == false)
		return false;

	return true;
}