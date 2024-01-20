#include "Settings.h"
#include "types.h"
#include "version.h"
#include "util.h"

#include <tc/cli.h>
#include <tc/os/Environment.h>
#include <tc/ArgumentException.h>
#include <tc/io/FileStream.h>
#include <tc/io/StreamSource.h>

#include <pietendo/hac/ContentArchiveUtil.h>
#include <pietendo/hac/AesKeygen.h>
#include <pietendo/hac/define/gc.h>
#include <pietendo/hac/define/pfs.h>
#include <pietendo/hac/define/nca.h>
#include <pietendo/hac/define/meta.h>
#include <pietendo/hac/define/romfs.h>
#include <pietendo/hac/define/cnmt.h>
#include <pietendo/hac/define/nacp.h>
#include <pietendo/hac/define/nso.h>
#include <pietendo/hac/define/nro.h>
#include <pietendo/hac/define/ini.h>
#include <pietendo/hac/define/kip.h>
#include <pietendo/hac/define/aset.h>
#include <pietendo/hac/es/SignedData.h>
#include <pietendo/hac/es/CertificateBody.h>
#include <pietendo/hac/es/SignUtils.h>
#include <pietendo/hac/es/TicketBody_V2.h>

class UnkOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	UnkOptionHandler(const std::string& module_label) : mModuleLabel(module_label)
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		throw tc::InvalidOperationException("getOptionStrings() not defined for UnkOptionHandler.");
	}

	const std::vector<std::string>& getOptionRegexPatterns() const
	{
		throw tc::InvalidOperationException("getOptionRegexPatterns() not defined for UnkOptionHandler.");
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
		mOptStrings(opts),
		mOptRegex()
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	const std::vector<std::string>& getOptionRegexPatterns() const
	{
		return mOptRegex;
	}

	void processOption(const std::string& option, const std::vector<std::string>& params)
	{
		fmt::print("[WARNING] Option \"{}\" is deprecated.{}{}\n", option, (mWarnMessage.empty() ? "" : " "), mWarnMessage);
	}
private:
	std::string mWarnMessage;
	std::vector<std::string> mOptStrings;
	std::vector<std::string> mOptRegex;
};

class FlagOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	FlagOptionHandler(bool& flag, const std::vector<std::string>& opts) : 
		mFlag(flag),
		mOptStrings(opts),
		mOptRegex()
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	const std::vector<std::string>& getOptionRegexPatterns() const
	{
		return mOptRegex;
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
	std::vector<std::string> mOptRegex;
};

class SingleParamStringOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	SingleParamStringOptionHandler(tc::Optional<std::string>& param, const std::vector<std::string>& opts) : 
		mParam(param),
		mOptStrings(opts),
		mOptRegex()
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	const std::vector<std::string>& getOptionRegexPatterns() const
	{
		return mOptRegex;
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
	std::vector<std::string> mOptRegex;
};

class SingleParamPathOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	SingleParamPathOptionHandler(tc::Optional<tc::io::Path>& param, const std::vector<std::string>& opts) : 
		mParam(param),
		mOptStrings(opts),
		mOptRegex()
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	const std::vector<std::string>& getOptionRegexPatterns() const
	{
		return mOptRegex;
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
	std::vector<std::string> mOptRegex;
};

class SingleParamSizetOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	SingleParamSizetOptionHandler(size_t& param, const std::vector<std::string>& opts) : 
		mParam(param),
		mOptStrings(opts),
		mOptRegex()
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	const std::vector<std::string>& getOptionRegexPatterns() const
	{
		return mOptRegex;
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
	std::vector<std::string> mOptRegex;
};

class SingleParamAesKeyOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	SingleParamAesKeyOptionHandler(tc::Optional<nstool::KeyBag::aes128_key_t>& param, const std::vector<std::string>& opts) :
		mParam(param),
		mOptStrings(opts),
		mOptRegex()
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	const std::vector<std::string>& getOptionRegexPatterns() const
	{
		return mOptRegex;
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
	std::vector<std::string> mOptRegex;
};

class SingleParamPathArrayOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	SingleParamPathArrayOptionHandler(std::vector<tc::io::Path>& param, const std::vector<std::string>& opts) : 
		mParam(param),
		mOptStrings(opts),
		mOptRegex()
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	const std::vector<std::string>& getOptionRegexPatterns() const
	{
		return mOptRegex;
	}

	void processOption(const std::string& option, const std::vector<std::string>& params)
	{
		if (params.size() != 1)
		{
			throw tc::ArgumentOutOfRangeException(fmt::format("Option \"{:s}\" requires a parameter.", option));
		}

		mParam.push_back(params[0]);
	}
private:
	std::vector<tc::io::Path>& mParam;
	std::vector<std::string> mOptStrings;
	std::vector<std::string> mOptRegex;
};

class FileTypeOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	FileTypeOptionHandler(nstool::Settings::FileType& param, const std::vector<std::string>& opts) : 
		mParam(param),
		mOptStrings(opts),
		mOptRegex()
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	const std::vector<std::string>& getOptionRegexPatterns() const
	{
		return mOptRegex;
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
			mParam = nstool::Settings::FILE_TYPE_ES_CERT;
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
	std::vector<std::string> mOptRegex;
};

class InstructionTypeOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	InstructionTypeOptionHandler(bool& param, const std::vector<std::string>& opts) :
		mParam(param),
		mOptStrings(opts),
		mOptRegex()
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	const std::vector<std::string>& getOptionRegexPatterns() const
	{
		return mOptRegex;
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
	std::vector<std::string> mOptRegex;
};

class ExtractDataPathOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	ExtractDataPathOptionHandler(std::vector<nstool::ExtractJob>& jobs, const std::vector<std::string>& opts) : 
		mJobs(jobs),
		mOptStrings(opts),
		mOptRegex()
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	const std::vector<std::string>& getOptionRegexPatterns() const
	{
		return mOptRegex;
	}

	void processOption(const std::string& option, const std::vector<std::string>& params)
	{
		if (params.size() == 1)
		{
			mJobs.push_back({tc::io::Path("/"), tc::io::Path(params[0])});
		}
		else if (params.size() == 2)
		{
			mJobs.push_back({tc::io::Path(params[0]), tc::io::Path(params[1])});
		} 
		else
		{
			throw tc::ArgumentOutOfRangeException(fmt::format("Option \"{:s}\" requires parameters in the format \"[<internal path>] <extract path>\".", option));
		}
	}
private:
	std::vector<nstool::ExtractJob>& mJobs;
	std::vector<std::string> mOptStrings;
	std::vector<std::string> mOptRegex;
};

class CustomExtractDataPathOptionHandler : public tc::cli::OptionParser::IOptionHandler
{
public:
	CustomExtractDataPathOptionHandler(std::vector<nstool::ExtractJob>& jobs, const std::vector<std::string>& opts, const tc::io::Path& custom_path) : 
		mJobs(jobs),
		mOptStrings(opts),
		mOptRegex(),
		mCustomPath(custom_path)
	{}

	const std::vector<std::string>& getOptionStrings() const
	{
		return mOptStrings;
	}

	const std::vector<std::string>& getOptionRegexPatterns() const
	{
		return mOptRegex;
	}

	void processOption(const std::string& option, const std::vector<std::string>& params)
	{
		if (params.size() != 1)
		{
			throw tc::ArgumentOutOfRangeException(fmt::format("Option \"{:s}\" requires a parameter.", option));
		}

		fmt::print("[WARNING] \"{:s} {:s}\" is deprecated. ", option, params[0]);
		// if custom path is root path, use the shortened version of -x
		if (mCustomPath == tc::io::Path("/"))
		{
			fmt::print("Consider using \"-x {:s}\" instead.\n", params[0]);
		}
		else
		{
			fmt::print("Consider using \"-x {:s} {:s}\" instead.\n", mCustomPath.to_string(), params[0]);
		}
			

		mJobs.push_back({mCustomPath, tc::io::Path(params[0])});
	}
private:
	std::vector<nstool::ExtractJob>& mJobs;
	std::vector<std::string> mOptStrings;
	std::vector<std::string> mOptRegex;
	tc::io::Path mCustomPath;
};

nstool::SettingsInitializer::SettingsInitializer(const std::vector<std::string>& args) :
	Settings(),
	mModuleLabel("nstool::SettingsInitializer"),
	mShowLayout(false),
	mShowKeydata(false),
	mVerbose(false),
	mNcaEncryptedContentKey(),
	mNcaContentKey(),
	mTikPathList(),
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
		loadKeyFile(mKeysetPath, opt.is_dev ? "dev.keys" : "prod.keys", "Maybe specify it with \"-k <path>\"?\n");
	}
	// locate title key file, if not specfied
	if (mTitleKeysetPath.isNull())
	{
		loadKeyFile(mTitleKeysetPath, "title.keys", "");
	}

	// generate keybag
	opt.keybag = KeyBagInitializer(opt.is_dev, mKeysetPath, mTitleKeysetPath, mTikPathList, mCertPath);
	opt.keybag.fallback_enc_content_key = mNcaEncryptedContentKey;
	opt.keybag.fallback_content_key = mNcaContentKey;

	// dump keys if requires
	if (mShowKeydata) // but not opt.cli_output_mode.show_keydata, since this that enabled by toggling -v,--verbose, personally I don't think a summary of imported keydata should be included in verbose output.
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
	opts.registerOptionHandler(std::shared_ptr<FileTypeOptionHandler>(new FileTypeOptionHandler(infile.filetype, { "-t", "--type" })));

	// get user-provided keydata
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(mKeysetPath, {"-k", "--keyset"})));
	//opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(mTitleKeysetPath, {"--titlekeyset"})));
	opts.registerOptionHandler(std::shared_ptr<SingleParamAesKeyOptionHandler>(new SingleParamAesKeyOptionHandler(mNcaEncryptedContentKey, {"--titlekey"})));
	opts.registerOptionHandler(std::shared_ptr<SingleParamAesKeyOptionHandler>(new SingleParamAesKeyOptionHandler(mNcaContentKey, {"--contentkey", "--bodykey"})));
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathArrayOptionHandler>(new SingleParamPathArrayOptionHandler(mTikPathList, {"--tik"})));
	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(mCertPath, {"--cert"})));

	// code options
	opts.registerOptionHandler(std::shared_ptr<FlagOptionHandler>(new FlagOptionHandler(code.list_api, { "--listapi" })));
	opts.registerOptionHandler(std::shared_ptr<FlagOptionHandler>(new FlagOptionHandler(code.list_symbols, { "--listsym" })));
	opts.registerOptionHandler(std::shared_ptr<InstructionTypeOptionHandler>(new InstructionTypeOptionHandler(code.is_64bit_instruction, { "--insttype" })));

	// fs options
	opts.registerOptionHandler(std::shared_ptr<FlagOptionHandler>(new FlagOptionHandler(fs.show_fs_tree, { "--fstree", "--listfs" })));
	opts.registerOptionHandler(std::shared_ptr<ExtractDataPathOptionHandler>(new ExtractDataPathOptionHandler(fs.extract_jobs, { "-x", "--extract" })));
	opts.registerOptionHandler(std::shared_ptr<CustomExtractDataPathOptionHandler>(new CustomExtractDataPathOptionHandler(fs.extract_jobs, { "--fsdir" }, tc::io::Path("/"))));

	// xci options
	opts.registerOptionHandler(std::shared_ptr<CustomExtractDataPathOptionHandler>(new CustomExtractDataPathOptionHandler(fs.extract_jobs, { "--update" }, tc::io::Path("/update/"))));
	opts.registerOptionHandler(std::shared_ptr<CustomExtractDataPathOptionHandler>(new CustomExtractDataPathOptionHandler(fs.extract_jobs, { "--normal" }, tc::io::Path("/normal/"))));
	opts.registerOptionHandler(std::shared_ptr<CustomExtractDataPathOptionHandler>(new CustomExtractDataPathOptionHandler(fs.extract_jobs, { "--secure" }, tc::io::Path("/secure/"))));
	opts.registerOptionHandler(std::shared_ptr<CustomExtractDataPathOptionHandler>(new CustomExtractDataPathOptionHandler(fs.extract_jobs, { "--logo" }, tc::io::Path("/logo/"))));

	// nca options
	opts.registerOptionHandler(std::shared_ptr<CustomExtractDataPathOptionHandler>(new CustomExtractDataPathOptionHandler(fs.extract_jobs, { "--part0" }, tc::io::Path("/0/"))));
	opts.registerOptionHandler(std::shared_ptr<CustomExtractDataPathOptionHandler>(new CustomExtractDataPathOptionHandler(fs.extract_jobs, { "--part1" }, tc::io::Path("/1/"))));
	opts.registerOptionHandler(std::shared_ptr<CustomExtractDataPathOptionHandler>(new CustomExtractDataPathOptionHandler(fs.extract_jobs, { "--part2" }, tc::io::Path("/2/"))));
	opts.registerOptionHandler(std::shared_ptr<CustomExtractDataPathOptionHandler>(new CustomExtractDataPathOptionHandler(fs.extract_jobs, { "--part3" }, tc::io::Path("/3/"))));

	opts.registerOptionHandler(std::shared_ptr<SingleParamPathOptionHandler>(new SingleParamPathOptionHandler(nca.base_nca_path, { "--basenca" })));

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
	//fmt::print("infile path = \"{}\"\n", infile.path.get().to_string());
	
	auto file = tc::io::StreamSource(std::make_shared<tc::io::FileStream>(tc::io::FileStream(infile.path.get(), tc::io::FileMode::Open, tc::io::FileAccess::Read)));

	auto raw_data = file.pullData(0, 0x5000);

#define _TYPE_PTR(st) ((st*)(raw_data.data()))
#define _ASSERT_FILE_SIZE(sz) (file.length() >= tc::io::IOUtil::castSizeToInt64(sz))

	// do easy tests

	// detect "scene" XCI
	if (_ASSERT_FILE_SIZE(sizeof(pie::hac::sGcHeader_Rsa2048Signed))
	 && _TYPE_PTR(pie::hac::sGcHeader_Rsa2048Signed)->header.st_magic.unwrap() == pie::hac::gc::kGcHeaderStructMagic)
	{
		infile.filetype = FILE_TYPE_GAMECARD;
	}
	// detect "SDK" XCI
	else if (_ASSERT_FILE_SIZE(sizeof(pie::hac::sSdkGcHeader))
		&& _TYPE_PTR(pie::hac::sSdkGcHeader)->signed_header.header.st_magic.unwrap() == pie::hac::gc::kGcHeaderStructMagic)
	{
		infile.filetype = FILE_TYPE_GAMECARD;
	}
	// detect PFS0
	else if (_ASSERT_FILE_SIZE(sizeof(pie::hac::sPfsHeader))
	      && _TYPE_PTR(pie::hac::sPfsHeader)->st_magic.unwrap() == pie::hac::pfs::kPfsStructMagic)
	{
		infile.filetype = FILE_TYPE_PARTITIONFS;
	}
	// detect HFS0
	else if (_ASSERT_FILE_SIZE(sizeof(pie::hac::sPfsHeader))
		&& _TYPE_PTR(pie::hac::sPfsHeader)->st_magic.unwrap() == pie::hac::pfs::kHashedPfsStructMagic)
	{
		infile.filetype = FILE_TYPE_PARTITIONFS;
	}
	// detect ROMFS
	else if (_ASSERT_FILE_SIZE(sizeof(pie::hac::sRomfsHeader))
		&& _TYPE_PTR(pie::hac::sRomfsHeader)->header_size.unwrap() == sizeof(pie::hac::sRomfsHeader)
		&& _TYPE_PTR(pie::hac::sRomfsHeader)->dir_entry.offset.unwrap() == (_TYPE_PTR(pie::hac::sRomfsHeader)->dir_hash_bucket.offset.unwrap() + _TYPE_PTR(pie::hac::sRomfsHeader)->dir_hash_bucket.size.unwrap()))
	{
		infile.filetype = FILE_TYPE_ROMFS;
	}
	// detect NPDM
	else if (_ASSERT_FILE_SIZE(sizeof(pie::hac::sMetaHeader))
		&& _TYPE_PTR(pie::hac::sMetaHeader)->st_magic.unwrap() == pie::hac::meta::kMetaStructMagic)
	{
		infile.filetype = FILE_TYPE_META;
	}
	// detect NSO
	else if (_ASSERT_FILE_SIZE(sizeof(pie::hac::sNsoHeader))
		&& _TYPE_PTR(pie::hac::sNsoHeader)->st_magic.unwrap() == pie::hac::nso::kNsoStructMagic)
	{
		infile.filetype = FILE_TYPE_NSO;
	}
	// detect NRO
	else if (_ASSERT_FILE_SIZE(sizeof(pie::hac::sNroHeader))
		&& _TYPE_PTR(pie::hac::sNroHeader)->st_magic.unwrap() == pie::hac::nro::kNroStructMagic)
	{
		infile.filetype = FILE_TYPE_NRO;
	}
	// detect INI
	else if (_ASSERT_FILE_SIZE(sizeof(pie::hac::sIniHeader))
		&& _TYPE_PTR(pie::hac::sIniHeader)->st_magic.unwrap() == pie::hac::ini::kIniStructMagic)
	{
		infile.filetype = FILE_TYPE_INI;
	}
	// detect KIP
	else if (_ASSERT_FILE_SIZE(sizeof(pie::hac::sKipHeader))
		&& _TYPE_PTR(pie::hac::sKipHeader)->st_magic.unwrap() == pie::hac::kip::kKipStructMagic)
	{
		infile.filetype = FILE_TYPE_KIP;
	}
	// detect HB ASET
	else if (_ASSERT_FILE_SIZE(sizeof(pie::hac::sAssetHeader))
		&& _TYPE_PTR(pie::hac::sAssetHeader)->st_magic.unwrap() == pie::hac::aset::kAssetStructMagic)
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
		infile.filetype = FILE_TYPE_ES_CERT;
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
	fmt::print("\n  PFS0/HFS0 (PartitionFs), RomFs, NSP (Nintendo Submission Package)\n");
	fmt::print("    {:s} [--fstree] [-x [<virtual path>] <out path>] <file>\n", BIN_NAME);
	fmt::print("      --fstree        Print filesystem tree.\n");
	fmt::print("      -x, --extract   Extract a file or directory to local filesystem.\n");
	fmt::print("\n  XCI (GameCard Image)\n");
	fmt::print("    {:s} [--fstree] [-x [<virtual path>] <out path>] <.xci file>\n", BIN_NAME);
	fmt::print("      --fstree        Print filesystem tree.\n");
	fmt::print("      -x, --extract   Extract a file or directory to local filesystem.\n");
	fmt::print("      --update        Extract \"update\" partition to directory. (Alias for \"-x /update <out path>\")\n");
	fmt::print("      --logo          Extract \"logo\" partition to directory. (Alias for \"-x /logo <out path>\")\n");
	fmt::print("      --normal        Extract \"normal\" partition to directory. (Alias for \"-x /normal <out path>\")\n");
	fmt::print("      --secure        Extract \"secure\" partition to directory. (Alias for \"-x /secure <out path>\")\n");
	fmt::print("\n  NCA (Nintendo Content Archive)\n");
	fmt::print("    {:s} [--fstree] [-x [<virtual path>] <out path>] [--bodykey <key> --titlekey <key> -tik <tik path> --basenca <.nca file>] <.nca file>\n", BIN_NAME);
	fmt::print("      --fstree        Print filesystem tree.\n");
	fmt::print("      -x, --extract   Extract a file or directory to local filesystem.\n");
	fmt::print("      --titlekey      Specify (encrypted) title key extracted from ticket.\n");
	fmt::print("      --contentkey    Specify content key.\n");
	fmt::print("      --tik           Specify ticket to source title key.\n");
	fmt::print("      --cert          Specify certificate chain to verify ticket.\n");
	fmt::print("      --part0         Extract partition \"0\" to directory. (Alias for \"-x /0 <out path>\")\n");
	fmt::print("      --part1         Extract partition \"1\" to directory. (Alias for \"-x /1 <out path>\")\n");
	fmt::print("      --part2         Extract partition \"2\" to directory. (Alias for \"-x /2 <out path>\")\n");
	fmt::print("      --part3         Extract partition \"3\" to directory. (Alias for \"-x /3 <out path>\")\n");
	fmt::print("      --basenca       Specify base NCA file for update NCA files.\n");
	fmt::print("\n  NSO (Nintendo Shared Object), NRO (Nintendo Relocatable Object)\n");
	fmt::print("    {:s} [--listapi --listsym] [--insttype <inst. type>] <file>\n", BIN_NAME);
	fmt::print("      --listapi       Print SDK API List.\n");
	fmt::print("      --listsym       Print Code Symbols.\n");
	fmt::print("      --insttype      Specify instruction type [64bit|32bit] (64bit is assumed).\n");
	fmt::print("\n  INI (Initial Program Bundle)\n");
	fmt::print("    {:s} [--kipdir <dir>] <file>\n", BIN_NAME);
	fmt::print("      --kipdir        Extract embedded Initial Programs to directory.\n");
	fmt::print("\n  ASET (Homebrew Asset Blob)\n");
	fmt::print("    {:s} [--fstree] [-x [<virtual path>] <out path>] [--icon <file> --nacp <file>] <file>\n", BIN_NAME);
	fmt::print("      --fstree        Print RomFs filesystem tree.\n");
	fmt::print("      -x, --extract   Extract a file or directory from RomFs to local filesystem.\n");
	fmt::print("      --icon          Extract icon partition to file.\n");
	fmt::print("      --nacp          Extract NACP partition to file.\n");
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
		fmt::print("      Key0: {:s}\n", tc::cli::FormatUtil::formatBytesAsString(opt.keybag.nca_header_key.get()[0].data(), opt.keybag.nca_header_key.get()[0].size(), true, ""));
		fmt::print("      Key1: {:s}\n", tc::cli::FormatUtil::formatBytesAsString(opt.keybag.nca_header_key.get()[1].data(), opt.keybag.nca_header_key.get()[1].size(), true, ""));
	}
	std::vector<std::string> kaek_label = {"Application", "Ocean", "System"};
	for (size_t kaek_index = 0; kaek_index < opt.keybag.nca_key_area_encryption_key.size(); kaek_index++)
	{
		for (auto itr = opt.keybag.nca_key_area_encryption_key[kaek_index].begin(); itr != opt.keybag.nca_key_area_encryption_key[kaek_index].end(); itr++)
		{
			fmt::print("    KeyAreaEncryptionKey-{:s}-{:02x}:\n      {:s}\n", kaek_label[kaek_index], itr->first, tc::cli::FormatUtil::formatBytesAsString(itr->second.data(), itr->second.size(), true, ""));
		}
	}
	for (size_t kaek_index = 0; kaek_index < opt.keybag.nca_key_area_encryption_key_hw.size(); kaek_index++)
	{
		for (auto itr = opt.keybag.nca_key_area_encryption_key_hw[kaek_index].begin(); itr != opt.keybag.nca_key_area_encryption_key_hw[kaek_index].end(); itr++)
		{
			fmt::print("    KeyAreaEncryptionKeyHw-{:s}-{:02x}:\n      {:s}\n", kaek_label[kaek_index], itr->first, tc::cli::FormatUtil::formatBytesAsString(itr->second.data(), itr->second.size(), true, ""));
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
		fmt::print("    ExtendedHeader-EncryptionKey-{:02x}:\n      {:s}\n", itr->first, tc::cli::FormatUtil::formatBytesAsString(itr->second.data(), itr->second.size(), true, ""));
	}
	if (opt.keybag.xci_cert_sign_key.isSet())
	{
		dump_rsa_key(opt.keybag.xci_cert_sign_key.get(), fmt::format("CERT-SignatureKey"), 4, opt.cli_output_mode.show_extended_info);
	}

	fmt::print("  Package1 Keys:\n");
	for (auto itr = opt.keybag.pkg1_key.begin(); itr != opt.keybag.pkg1_key.end(); itr++)
	{
		fmt::print("    EncryptionKey-{:02x}:\n      {:s}\n", itr->first, tc::cli::FormatUtil::formatBytesAsString(itr->second.data(), itr->second.size(), true, ""));
	}

	fmt::print("  Package2 Keys:\n");
	if (opt.keybag.pkg2_sign_key.isSet())
	{
		dump_rsa_key(opt.keybag.pkg2_sign_key.get(), fmt::format("Header-SignatureKey"), 4, opt.cli_output_mode.show_extended_info);
	}
	for (auto itr = opt.keybag.pkg2_key.begin(); itr != opt.keybag.pkg2_key.end(); itr++)
	{
		fmt::print("    EncryptionKey-{:02x}:\n      {:s}\n", itr->first, tc::cli::FormatUtil::formatBytesAsString(itr->second.data(), itr->second.size(), true, ""));
	}

	fmt::print("  ETicket Keys:\n");
	for (auto itr = opt.keybag.etik_common_key.begin(); itr != opt.keybag.etik_common_key.end(); itr++)
	{
		fmt::print("    CommonKey-{:02x}:\n      {:s}\n", itr->first, tc::cli::FormatUtil::formatBytesAsString(itr->second.data(), itr->second.size(), true, ""));
	}

	fmt::print("  BroadOn Signer Profiles:\n");
	for (auto itr = opt.keybag.broadon_signer.begin(); itr != opt.keybag.broadon_signer.end(); itr++)
	{
		fmt::print("    {:s}:\n", itr->first);
		fmt::print("      SignType: ");
		switch(itr->second.key_type) {
			case pie::hac::es::sign::SIGN_ALGO_RSA2048:
				fmt::print("RSA-2048\n");
				break;
			case pie::hac::es::sign::SIGN_ALGO_RSA4096:
				fmt::print("RSA-4096\n");
				break;
			case pie::hac::es::sign::SIGN_ALGO_ECDSA240:
				fmt::print("ECDSA-240\n");
				break;
			default:
				fmt::print("Unknown\n");
		}
		switch(itr->second.key_type) {
			case pie::hac::es::sign::SIGN_ALGO_RSA2048:
			case pie::hac::es::sign::SIGN_ALGO_RSA4096:
				dump_rsa_key(itr->second.rsa_key, "RsaKey", 6, opt.cli_output_mode.show_extended_info);
				break;
			case pie::hac::es::sign::SIGN_ALGO_ECDSA240:
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
			fmt::print("{:s}    {:s}", indent_str, tc::cli::FormatUtil::formatBytesAsStringWithLineLimit(key.n.data(), key.n.size(), true, "", 0x10, indent + 4, false));
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
			fmt::print("{:s}    {:s}", indent_str, tc::cli::FormatUtil::formatBytesAsStringWithLineLimit(key.d.data(), key.d.size(), true, "", 0x10, indent + 4, false));
		}
		else
		{
			fmt::print("{:s}  Private Exponent: {:s}\n", indent_str, getTruncatedBytesString(key.d.data(), key.d.size()));
		}
	}
}

void nstool::SettingsInitializer::loadKeyFile(tc::Optional<tc::io::Path>& keyfile_path, const std::string& keyfile_name, const std::string& cli_hint)
{
	std::string home_path_str;
	if (tc::os::getEnvVar("HOME", home_path_str) || tc::os::getEnvVar("USERPROFILE", home_path_str))
	{
		tc::io::Path tmp_path = tc::io::Path(home_path_str);
		tmp_path.push_back(".switch");
		tmp_path.push_back(keyfile_name);

		try {
			tc::io::FileStream test = tc::io::FileStream(tmp_path, tc::io::FileMode::Open, tc::io::FileAccess::Read);
			
			keyfile_path = tmp_path;
		}
		catch (tc::io::FileNotFoundException&) {
			fmt::print("[WARNING] Failed to load \"{}\" keyfile.{}\n", keyfile_name, cli_hint);
		}
	}
	else {
		fmt::print("[WARNING] Failed to locate \"{}\" keyfile.{}\n", keyfile_name, cli_hint);
	}
	
}


bool nstool::SettingsInitializer::determineValidNcaFromSample(const tc::ByteData& sample) const
{
	if (sample.size() < pie::hac::nca::kHeaderSize)
	{
		return false;
	}
	
	if (opt.keybag.nca_header_key.isNull())
	{
		fmt::print("[WARNING] Failed to load NCA Header Key.\n");
		return false;
	}

	pie::hac::detail::aes128_xtskey_t key = opt.keybag.nca_header_key.get();

	//fmt::print("NCA header key: {} {}\n", tc::cli::FormatUtil::formatBytesAsString(opt.keybag.nca_header_key.get()[0].data(), opt.keybag.nca_header_key.get()[0].size(), true, ""), tc::cli::FormatUtil::formatBytesAsString(opt.keybag.nca_header_key.get()[1].data(), opt.keybag.nca_header_key.get()[1].size(), true, ""));

	// init aes-xts
	tc::crypto::Aes128XtsEncryptor enc;
	enc.initialize(key[0].data(), key[0].size(), key[1].data(), key[1].size(), pie::hac::nca::kSectorSize, false);

	// decrypt main header
	byte_t raw_hdr[pie::hac::nca::kSectorSize];
	enc.decrypt(raw_hdr, sample.data() + pie::hac::ContentArchiveUtil::sectorToOffset(1), pie::hac::nca::kSectorSize, 1);
	pie::hac::sContentArchiveHeader* hdr = (pie::hac::sContentArchiveHeader*)(raw_hdr);

	/*
	fmt::print("NCA Header Raw:\n");
	fmt::print("{:s}\n", tc::cli::FormatUtil::formatBytesAsHxdHexString(sample.data() + pie::hac::ContentArchiveUtil::sectorToOffset(1), pie::hac::nca::kSectorSize));
	fmt::print("{:s}\n", tc::cli::FormatUtil::formatBytesAsHxdHexString(raw_hdr, pie::hac::nca::kSectorSize));
	*/

	if (hdr->st_magic.unwrap() != pie::hac::nca::kNca2StructMagic && hdr->st_magic.unwrap() != pie::hac::nca::kNca3StructMagic)
	{
		return false;
	}

	return true;
}

bool nstool::SettingsInitializer::determineValidCnmtFromSample(const tc::ByteData& sample) const
{
	if (sample.size() < sizeof(pie::hac::sContentMetaHeader))
		return false;

	const pie::hac::sContentMetaHeader* data = (const pie::hac::sContentMetaHeader*)sample.data();

	size_t minimum_size = sizeof(pie::hac::sContentMetaHeader) + data->exhdr_size.unwrap() + data->content_count.unwrap() * sizeof(pie::hac::sContentInfo) + data->content_meta_count.unwrap() * sizeof(pie::hac::sContentMetaInfo) + pie::hac::cnmt::kDigestLen;

	if (sample.size() < minimum_size)
		return false;

	// include exthdr/data check if applicable
	if (data->exhdr_size.unwrap() > 0)
	{
		if (data->type == (byte_t)pie::hac::cnmt::ContentMetaType_Application)
		{
			const pie::hac::sApplicationMetaExtendedHeader* meta = (const pie::hac::sApplicationMetaExtendedHeader*)(sample.data() + sizeof(pie::hac::sContentMetaHeader));
			if ((meta->patch_id.unwrap() & data->id.unwrap()) != data->id.unwrap())
				return false;
		}
		else if (data->type == (byte_t)pie::hac::cnmt::ContentMetaType_Patch)
		{
			const pie::hac::sPatchMetaExtendedHeader* meta = (const pie::hac::sPatchMetaExtendedHeader*)(sample.data() + sizeof(pie::hac::sContentMetaHeader));
			if ((meta->application_id.unwrap() & data->id.unwrap()) != meta->application_id.unwrap())
				return false;

			minimum_size += meta->extended_data_size.unwrap();
		}
		else if (data->type == (byte_t)pie::hac::cnmt::ContentMetaType_AddOnContent)
		{
			const pie::hac::sAddOnContentMetaExtendedHeader* meta = (const pie::hac::sAddOnContentMetaExtendedHeader*)(sample.data() + sizeof(pie::hac::sContentMetaHeader));
			if ((meta->application_id.unwrap() & data->id.unwrap()) != meta->application_id.unwrap())
				return false;
		}
		else if (data->type == (byte_t)pie::hac::cnmt::ContentMetaType_Delta)
		{
			const pie::hac::sDeltaMetaExtendedHeader* meta = (const pie::hac::sDeltaMetaExtendedHeader*)(sample.data() + sizeof(pie::hac::sContentMetaHeader));
			if ((meta->application_id.unwrap() & data->id.unwrap()) != meta->application_id.unwrap())
				return false;

			minimum_size += meta->extended_data_size.unwrap();
		}
		else if (data->type == (byte_t)pie::hac::cnmt::ContentMetaType_SystemUpdate)
		{
			const pie::hac::sSystemUpdateMetaExtendedHeader* meta = (const pie::hac::sSystemUpdateMetaExtendedHeader*)(sample.data() + sizeof(pie::hac::sContentMetaHeader));

			minimum_size += meta->extended_data_size.unwrap();
		}
	}

	if (sample.size() != minimum_size)
		return false;

	return true;
}

bool nstool::SettingsInitializer::determineValidNacpFromSample(const tc::ByteData& sample) const
{
	if (sample.size() != sizeof(pie::hac::sApplicationControlProperty))
		return false;

	const pie::hac::sApplicationControlProperty* data = (const pie::hac::sApplicationControlProperty*)sample.data();

	if (data->logo_type > (byte_t)pie::hac::nacp::LogoType_Nintendo)
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
	pie::hac::es::SignatureBlock sign;

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

	if (sign.getSignType() != pie::hac::es::sign::SIGN_ID_RSA4096_SHA256 && sign.getSignType() != pie::hac::es::sign::SIGN_ID_RSA2048_SHA256 && sign.getSignType() != pie::hac::es::sign::SIGN_ID_ECDSA240_SHA256)
		return false;

	return true;
}

bool nstool::SettingsInitializer::determineValidEsTikFromSample(const tc::ByteData& sample) const
{
	pie::hac::es::SignatureBlock sign;

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

	if (sign.getSignType() != pie::hac::es::sign::SIGN_ID_RSA2048_SHA256)
		return false;

	const pie::hac::es::sTicketBody_v2* body = (const pie::hac::es::sTicketBody_v2*)(sample.data() + sign.getBytes().size());

	if ((body->issuer.decode().substr(0, 5) == "Root-"
		&& body->issuer.decode().substr(16, 2) == "XS") == false)
		return false;

	return true;
}