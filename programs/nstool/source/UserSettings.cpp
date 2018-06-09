#include "UserSettings.h"
#include "version.h"
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <fnd/io.h>
#include <fnd/SimpleFile.h>
#include <fnd/SimpleTextOutput.h>
#include <fnd/MemoryBlob.h>
#include <fnd/ResourceFileReader.h>
#include <nx/NcaUtils.h>
#include <nx/AesKeygen.h>
#include <nx/xci.h>
#include <nx/pfs.h>
#include <nx/nca.h>
#include <nx/npdm.h>
#include <nx/romfs.h>
#include <nx/nso.h>
#include <nx/nro.h>

UserSettings::UserSettings()
{}

void UserSettings::parseCmdArgs(int argc, char** argv)
{
	sCmdArgs args;
	populateCmdArgs(argc, argv, args);
	populateKeyset(args);
	populateUserSettings(args);
}

void UserSettings::showHelp()
{
	printf("NSTool v%d.%d (C) %s\n", VER_MAJOR, VER_MINOR, AUTHORS);
	printf("Built: %s %s\n\n", __TIME__, __DATE__);
	
	printf("Usage: nstool [options... ] <file>\n");
	printf("\n  General Options:\n");
	printf("      -d, --dev       Use devkit keyset\n");
	printf("      -k, --keyset    Specify keyset file\n");
	printf("      -t, --type      Specify input file type [xci, pfs, romfs, nca, npdm, cnmt, nso, nro]\n");
	printf("      -y, --verify    Verify file\n");
	printf("      -v, --verbose   Verbose output\n");
	printf("      -q, --quiet     Minimal output\n");
	printf("\n  XCI (GameCard Image)\n");
	printf("    nstool [--listfs] [--update <dir> --normal <dir> --secure <dir>] <.xci file>\n");
	printf("      --listfs        Print file system in embedded partitions\n");
	printf("      --update        Extract \"update\" partition to directory\n");
	printf("      --normal        Extract \"normal\" partition to directory\n");
	printf("      --logo          Extract \"logo\" partition to directory\n");
	printf("      --secure        Extract \"secure\" partition to directory\n");
	printf("\n  PFS0/HFS0 (PartitionFs), RomFs, NSP (Ninendo Submission Package)\n");
	printf("    nstool [--listfs] [--fsdir <dir>] <file>\n");
	printf("      --listfs        Print file system\n");
	printf("      --fsdir         Extract file system to directory\n");
	printf("\n  NCA (Nintendo Content Archive)\n");
	printf("    nstool [--listfs] [--bodykey <key> --titlekey <key>] [--part0 <dir> ...] <.nca file>\n");
	printf("      --listfs        Print file system in embedded partitions\n");
	printf("      --titlekey      Specify title key extracted from ticket\n");
	printf("      --bodykey       Specify body encryption key\n");
	printf("      --part0         Extract \"partition 0\" to directory \n");
	printf("      --part1         Extract \"partition 1\" to directory \n");
	printf("      --part2         Extract \"partition 2\" to directory \n");
	printf("      --part3         Extract \"partition 3\" to directory \n");
	printf("\n  NSO (Nintendo Software Object), NRO (Nintendo Relocatable Object)\n");
	printf("    nstool [--listapi --listsym] [--insttype <inst. type>] <file>\n");
	printf("      --listapi       Print SDK API List.\n");
	printf("      --listsym       Print Dynamic Symbols.\n");
	printf("      --insttype      Specify instruction type [64bit|32bit] (64bit is assumed).\n");

}

const std::string UserSettings::getInputPath() const
{
	return mInputPath;
}

const sKeyset& UserSettings::getKeyset() const
{
	return mKeyset;
}

FileType UserSettings::getFileType() const
{
	return mFileType;
}

bool UserSettings::isVerifyFile() const
{
	return mVerifyFile;
}

CliOutputType UserSettings::getCliOutputType() const
{
	return mOutputType;
}

bool UserSettings::isListFs() const
{
	return mListFs;
}

bool UserSettings::isListApi() const
{
	return mListApi;
}
bool UserSettings::isListSymbols() const
{
	return mListSymbols;
}

nx::npdm::InstructionType UserSettings::getInstType() const
{
	return mInstructionType;
}

const sOptional<std::string>& UserSettings::getXciUpdatePath() const
{
	return mXciUpdatePath;
}

const sOptional<std::string>& UserSettings::getXciNormalPath() const
{
	return mXciNormalPath;
}

const sOptional<std::string>& UserSettings::getXciSecurePath() const
{
	return mXciSecurePath;
}

const sOptional<std::string>& UserSettings::getXciLogoPath() const
{
	return mXciLogoPath;
}

const sOptional<std::string>& UserSettings::getFsPath() const
{
	return mFsPath;
}

const sOptional<std::string>& UserSettings::getNcaPart0Path() const
{
	return mNcaPart0Path;
}

const sOptional<std::string>& UserSettings::getNcaPart1Path() const
{
	return mNcaPart1Path;
}

const sOptional<std::string>& UserSettings::getNcaPart2Path() const
{
	return mNcaPart2Path;
}

const sOptional<std::string>& UserSettings::getNcaPart3Path() const
{
	return mNcaPart3Path;
}


void UserSettings::populateCmdArgs(int argc, char** argv, sCmdArgs& cmd_args)
{
	// create vector of args
	std::vector<std::string> args;
	for (size_t i = 0; i < (size_t)argc; i++)
	{
		args.push_back(argv[i]);
	}

	// show help text
	if (args.size() < 2)
	{
		showHelp();
		throw fnd::Exception(kModuleName, "Not enough arguments.");
	}

	cmd_args.input_path = args.back();

	for (size_t i = 1; i < args.size(); i++)
	{
		if (args[i] == "-h" || args[i] == "--help")
		{
			showHelp();
			throw fnd::Exception(kModuleName, "Nothing to do.");
		}
	}

	for (size_t i = 1; i+1 < args.size(); i++)
	{
		bool hasParamter = args[i+1][0] != '-' && i+2 < args.size();

		if (args[i] == "-d" || args[i] == "--dev")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, args[i] + " does not take a parameter.");
			cmd_args.devkit_keys = true;
		}

		else if (args[i] == "-y" || args[i] == "--verify")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, args[i] + " does not take a parameter.");
			cmd_args.verify_file = true;
		}

		else if (args[i] == "-v" || args[i] == "--verbose")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, args[i] + " does not take a parameter.");
			cmd_args.verbose_output = true;
		}

		else if (args[i] == "-q" || args[i] == "--quiet")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, args[i] + " does not take a parameter.");
			cmd_args.minimal_output = true;
		}

		else if (args[i] == "-k" || args[i] == "--keyset")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, args[i] + " requries a parameter.");
			cmd_args.keyset_path = args[i+1];
		}

		else if (args[i] == "-t" || args[i] == "--type")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, args[i] + " requries a parameter.");
			cmd_args.file_type = args[i+1];
		}

		else if (args[i] == "--listfs")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, args[i] + " does not take a parameter.");
			cmd_args.list_fs = true;
		}

		else if (args[i] == "--update")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, args[i] + " requries a parameter.");
			cmd_args.update_path = args[i+1];
		}

		else if (args[i] == "--normal")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, args[i] + " requries a parameter.");
			cmd_args.normal_path = args[i+1];
		}

		else if (args[i] == "--secure")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, args[i] + " requries a parameter.");
			cmd_args.secure_path = args[i+1];
		}

		else if (args[i] == "--logo")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, args[i] + " requries a parameter.");
			cmd_args.logo_path = args[i+1];
		}

		else if (args[i] == "--fsdir")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, args[i] + " requries a parameter.");
			cmd_args.fs_path = args[i+1];
		}

		else if (args[i] == "--titlekey")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, args[i] + " requries a parameter.");
			cmd_args.nca_titlekey = args[i+1];
		}

		else if (args[i] == "--bodykey")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, args[i] + " requries a parameter.");
			cmd_args.nca_bodykey = args[i+1];
		}

		else if (args[i] == "--part0")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, args[i] + " requries a parameter.");
			cmd_args.part0_path = args[i+1];
		}

		else if (args[i] == "--part1")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, args[i] + " requries a parameter.");
			cmd_args.part1_path = args[i+1];
		}

		else if (args[i] == "--part2")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, args[i] + " requries a parameter.");
			cmd_args.part2_path = args[i+1];
		}

		else if (args[i] == "--part3")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, args[i] + " requries a parameter.");
			cmd_args.part3_path = args[i+1];
		}

		else if (args[i] == "--listapi")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, args[i] + " does not take a parameter.");
			cmd_args.list_api = true;
		}

		else if (args[i] == "--listsym")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, args[i] + " does not take a parameter.");
			cmd_args.list_sym = true;
		}

		else if (args[i] == "--insttype")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, args[i] + " requries a parameter.");
			cmd_args.inst_type = args[i + 1];
		}

		else
		{
			throw fnd::Exception(kModuleName, args[i] + " is not recognised.");
		}

		i += hasParamter;
	}
}

void UserSettings::populateKeyset(sCmdArgs& args)
{
	crypto::aes::sAes128Key zeros_aes_key;
	crypto::aes::sAesXts128Key zeros_aes_xts_key;
	memset((void*)&zeros_aes_key, 0, sizeof(crypto::aes::sAes128Key));
	memset((void*)&zeros_aes_xts_key, 0, sizeof(crypto::aes::sAesXts128Key));
	memset((void*)&mKeyset, 0, sizeof(sKeyset));

	fnd::ResourceFileReader res;
	if (args.keyset_path.isSet)
	{
		res.processFile(*args.keyset_path);
 	}
	else
	{
		// open other resource files in $HOME/.switch/prod.keys (or $HOME/.switch/dev.keys if -d/--dev is set).
		std::string home;
		if (home.empty()) fnd::io::getEnvironVar(home, "HOME");
		if (home.empty()) fnd::io::getEnvironVar(home, "USERPROFILE");
		if (home.empty()) return;

		const std::string kKeysetNameStr[2] = {"prod.keys", "dev.keys"};
		const std::string kHomeSwitchDirStr = ".switch";
		
		std::string keyset_path;
		fnd::io::appendToPath(keyset_path, home);
		fnd::io::appendToPath(keyset_path, kHomeSwitchDirStr);
		fnd::io::appendToPath(keyset_path, kKeysetNameStr[args.devkit_keys.isSet ? *args.devkit_keys : 0]);

		try
		{
			res.processFile(keyset_path);
		}
		catch (const fnd::Exception&)
		{
			return;
		}
		
	}
	
	// suffix
	const std::string kKeyStr = "key";
	const std::string kKekStr = "kek";
	const std::string kSourceStr = "source";
	const std::string kRsaKeySuffix[2] = {"sign_key_private", "sign_key_modulus"};
	const std::string kKeyIndex[kMasterKeyNum] = {"00","01","02","03","04","05","06","07","08","09","0a","0b","0c","0d","0e","0f","10","11","12","13","14","15","16","17","18","19","1a","1b","1c","1d","1e","1f"};

	// keyname bases
	const std::string kMasterBase = "master";
	const std::string kPackage1Base = "package1";
	const std::string kPackage2Base = "package2";
	const std::string kXciHeaderBase = "xci_header";
	const std::string kNcaHeaderBase[2] = {"header", "nca_header"};
	const std::string kKekGenSource = "aes_kek_generation";
	const std::string kKeyGenSource = "aes_key_generation";
	const std::string kAcidBase = "acid";
	const std::string kTicketCommonKeyBase[2] = { "titlekek", "ticket_commonkey" };
	const std::string kNcaBodyBase[2] = {"key_area_key", "nca_body_keak"};
	const std::string kNcaBodyKeakIndexName[3] = {"application", "ocean", "system"};


	// sources
	crypto::aes::sAes128Key master_key[kMasterKeyNum] = { zeros_aes_key };
	crypto::aes::sAes128Key package2_key_source = zeros_aes_key;
	crypto::aes::sAes128Key ticket_titlekek_source = zeros_aes_key;
	crypto::aes::sAes128Key key_area_key_source[3] = { zeros_aes_key, zeros_aes_key, zeros_aes_key };
	crypto::aes::sAes128Key aes_kek_generation_source = zeros_aes_key;
	crypto::aes::sAes128Key aes_key_generation_source = zeros_aes_key;
	crypto::aes::sAes128Key nca_header_kek_source = zeros_aes_key;
	crypto::aes::sAesXts128Key nca_header_key_source = zeros_aes_xts_key;


#define _CONCAT_2_STRINGS(str1, str2) ((str1) + "_" + (str2))
#define _CONCAT_3_STRINGS(str1, str2, str3) ((str1) + "_"+ (str2) + "_" + (str3))

	std::string key,val;

#define _SAVE_KEYDATA(key_name, array, len) \
	key = (key_name); \
	val = res[key]; \
	if (val.empty() == false) { \
		decodeHexStringToBytes(key, val, (byte_t*)array, len); \
	}
	
	_SAVE_KEYDATA(_CONCAT_3_STRINGS(kPackage2Base, kKeyStr, kSourceStr), package2_key_source.key, 0x10);
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kTicketCommonKeyBase[0], kSourceStr), ticket_titlekek_source.key, 0x10);
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kTicketCommonKeyBase[1], kSourceStr), ticket_titlekek_source.key, 0x10);
	_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaBodyBase[0], kNcaBodyKeakIndexName[0], kSourceStr), key_area_key_source[0].key, 0x10);
	_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaBodyBase[0], kNcaBodyKeakIndexName[1], kSourceStr), key_area_key_source[1].key, 0x10);
	_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaBodyBase[0], kNcaBodyKeakIndexName[2], kSourceStr), key_area_key_source[2].key, 0x10);
	_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaBodyBase[1], kNcaBodyKeakIndexName[0], kSourceStr), key_area_key_source[0].key, 0x10);
	_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaBodyBase[1], kNcaBodyKeakIndexName[1], kSourceStr), key_area_key_source[1].key, 0x10);
	_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaBodyBase[1], kNcaBodyKeakIndexName[2], kSourceStr), key_area_key_source[2].key, 0x10);
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kKekGenSource, kSourceStr), aes_kek_generation_source.key, 0x10);
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kKeyGenSource, kSourceStr), aes_key_generation_source.key, 0x10);
	_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaHeaderBase[0], kKekStr, kSourceStr), nca_header_kek_source.key, 0x10);
	_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaHeaderBase[0], kKeyStr, kSourceStr), nca_header_key_source.key, 0x20);
	_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaHeaderBase[1], kKekStr, kSourceStr), nca_header_kek_source.key, 0x10);
	_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaHeaderBase[1], kKeyStr, kSourceStr), nca_header_key_source.key, 0x20);

	// Store Key Variants/Derivatives
	for (size_t i = 0; i < kMasterKeyNum; i++)
	{
		
		_SAVE_KEYDATA(_CONCAT_3_STRINGS(kMasterBase, kKeyStr, kKeyIndex[i]), master_key[i].key, 0x10);
		_SAVE_KEYDATA(_CONCAT_3_STRINGS(kPackage1Base, kKeyStr, kKeyIndex[i]), mKeyset.package1_key[i].key, 0x10);
		_SAVE_KEYDATA(_CONCAT_3_STRINGS(kPackage2Base, kKeyStr, kKeyIndex[i]), mKeyset.package2_key[i].key, 0x10);
		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kTicketCommonKeyBase[0], kKeyIndex[i]), mKeyset.ticket.titlekey_kek[i].key, 0x10);
		_SAVE_KEYDATA(_CONCAT_2_STRINGS(kTicketCommonKeyBase[1], kKeyIndex[i]), mKeyset.ticket.titlekey_kek[i].key, 0x10);
		_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaBodyBase[0], kNcaBodyKeakIndexName[0], kKeyIndex[i]), mKeyset.nca.key_area_key[0][i].key, 0x10);
		_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaBodyBase[0], kNcaBodyKeakIndexName[1], kKeyIndex[i]), mKeyset.nca.key_area_key[1][i].key, 0x10);
		_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaBodyBase[0], kNcaBodyKeakIndexName[2], kKeyIndex[i]), mKeyset.nca.key_area_key[2][i].key, 0x10);
		_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaBodyBase[1], kNcaBodyKeakIndexName[0], kKeyIndex[i]), mKeyset.nca.key_area_key[0][i].key, 0x10);
		_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaBodyBase[1], kNcaBodyKeakIndexName[1], kKeyIndex[i]), mKeyset.nca.key_area_key[1][i].key, 0x10);
		_SAVE_KEYDATA(_CONCAT_3_STRINGS(kNcaBodyBase[1], kNcaBodyKeakIndexName[2], kKeyIndex[i]), mKeyset.nca.key_area_key[2][i].key, 0x10);
	}
	
	// store nca header key
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kNcaHeaderBase[0], kKeyStr), mKeyset.nca.header_key.key[0], 0x20);
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kNcaHeaderBase[1], kKeyStr), mKeyset.nca.header_key.key[0], 0x20);

	// store xci header key
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kXciHeaderBase, kKeyStr), mKeyset.xci.header_key.key, 0x10);

	// store rsa keys
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kNcaHeaderBase[1], kRsaKeySuffix[0]), mKeyset.nca.header_sign_key.priv_exponent, 0x100);
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kNcaHeaderBase[1], kRsaKeySuffix[1]), mKeyset.nca.header_sign_key.modulus, 0x100);
	
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kXciHeaderBase, kRsaKeySuffix[0]), mKeyset.xci.header_sign_key.priv_exponent, 0x100);
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kXciHeaderBase, kRsaKeySuffix[1]), mKeyset.xci.header_sign_key.modulus, 0x100);

	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kAcidBase, kRsaKeySuffix[0]), mKeyset.acid_sign_key.priv_exponent, 0x100);
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kAcidBase, kRsaKeySuffix[1]), mKeyset.acid_sign_key.modulus, 0x100);

	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kPackage2Base, kRsaKeySuffix[0]), mKeyset.package2_sign_key.priv_exponent, 0x100);
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kPackage2Base, kRsaKeySuffix[1]), mKeyset.package2_sign_key.modulus, 0x100);

	// save keydata from input args
	if (args.nca_bodykey.isSet)
	{
		if (args.nca_bodykey.var.length() == (sizeof(crypto::aes::sAes128Key)*2))
		{
			decodeHexStringToBytes("--bodykey", args.nca_bodykey.var, mKeyset.nca.manual_body_key_aesctr.key, sizeof(crypto::aes::sAes128Key));
		}
		else
		{
			decodeHexStringToBytes("--bodykey", args.nca_bodykey.var, mKeyset.nca.manual_body_key_aesxts.key[0], sizeof(crypto::aes::sAesXts128Key));
		}
	}

	if (args.nca_titlekey.isSet)
	{
		if (args.nca_titlekey.var.length() == (sizeof(crypto::aes::sAes128Key)*2))
		{
			decodeHexStringToBytes("--titlekey", args.nca_titlekey.var, mKeyset.nca.manual_title_key_aesctr.key, sizeof(crypto::aes::sAes128Key));
		}
		else
		{
			decodeHexStringToBytes("--titlekey", args.nca_titlekey.var, mKeyset.nca.manual_title_key_aesxts.key[0], sizeof(crypto::aes::sAesXts128Key));
		}
	}

#undef _SAVE_KEYDATA
#undef _CONCAT_3_STRINGS
#undef _CONCAT_2_STRINGS
	
	// Derive keys 
	for (size_t i = 0; i < kMasterKeyNum; i++)
	{
		if (master_key[i] != zeros_aes_key)
		{
			if (aes_kek_generation_source != zeros_aes_key && aes_key_generation_source != zeros_aes_key)
			{
				if (i == 0 && nca_header_kek_source != zeros_aes_key && nca_header_key_source != zeros_aes_xts_key)
				{
					if (mKeyset.nca.header_key == zeros_aes_xts_key)
					{
						crypto::aes::sAes128Key nca_header_kek;
						nx::AesKeygen::generateKey(nca_header_kek.key, aes_kek_generation_source.key, nca_header_kek_source.key, aes_key_generation_source.key, master_key[i].key);
						nx::AesKeygen::generateKey(mKeyset.nca.header_key.key[0], nca_header_key_source.key[0], nca_header_kek.key);
						nx::AesKeygen::generateKey(mKeyset.nca.header_key.key[1], nca_header_key_source.key[1], nca_header_kek.key);
						//printf("nca header key[0] ");
						//fnd::SimpleTextOutput::hexDump(mKeyset.nca.header_key.key[0], 0x10);
						//printf("nca header key[1] ");
						//fnd::SimpleTextOutput::hexDump(mKeyset.nca.header_key.key[1], 0x10);
					}
				}

				for (size_t j = 0; j < nx::nca::kKeyAreaEncryptionKeyNum; j++)
				{
					if (key_area_key_source[j] != zeros_aes_key && mKeyset.nca.key_area_key[j][i] == zeros_aes_key)
					{
						nx::AesKeygen::generateKey(mKeyset.nca.key_area_key[j][i].key, aes_kek_generation_source.key, key_area_key_source[j].key, aes_key_generation_source.key, master_key[i].key);
						//printf("nca keak %d/%02d ", j, i);
						//fnd::SimpleTextOutput::hexDump(mKeyset.nca.key_area_key[j][i].key, 0x10);
					}
				}
			}

			if (ticket_titlekek_source != zeros_aes_key && mKeyset.ticket.titlekey_kek[i] == zeros_aes_key)
			{
				nx::AesKeygen::generateKey(mKeyset.ticket.titlekey_kek[i].key, ticket_titlekek_source.key, master_key[i].key);
				//printf("ticket titlekek %02d ", i);
				//fnd::SimpleTextOutput::hexDump(mKeyset.ticket.titlekey_kek[i].key, 0x10);
			}
			if (package2_key_source != zeros_aes_key && mKeyset.package2_key[i] == zeros_aes_key)
			{
				nx::AesKeygen::generateKey(mKeyset.package2_key[i].key, package2_key_source.key, master_key[i].key);
				//printf("package2 key %02d ", i);
				//fnd::SimpleTextOutput::hexDump(mKeyset.package2_key[i].key, 0x10);
			}
		}
		/*
		for (size_t j = 0; j < nx::nca::kKeyAreaEncryptionKeyNum; j++)
		{
			if (mKeyset.nca.key_area_key[j][i] != zeros_aes_key)
			{
				printf("nca body keak %d/%02d ", j, i);
				fnd::SimpleTextOutput::hexDump(mKeyset.nca.key_area_key[j][i].key, 0x10);
			}
		}
		*/
	}
}

void UserSettings::populateUserSettings(sCmdArgs& args)
{
	// check invalid input
	if (args.input_path.isSet == false)
		throw fnd::Exception(kModuleName, "No input file specified");
	if (args.verbose_output.isSet && args.minimal_output.isSet)
		throw fnd::Exception(kModuleName, "Options --verbose and --quiet cannot be used together.");
	
	// save arguments
	mInputPath = *args.input_path;
	mVerifyFile = args.verify_file.isSet;
	mListFs = args.list_fs.isSet;
	mXciUpdatePath = args.update_path;
	mXciNormalPath = args.normal_path;
	mXciSecurePath = args.secure_path;
	mXciLogoPath = args.logo_path;

	mFsPath = args.fs_path;
	mNcaPart0Path = args.part0_path;
	mNcaPart1Path = args.part1_path;
	mNcaPart2Path = args.part2_path;
	mNcaPart3Path = args.part3_path;

	// determine the architecture type for NSO/NRO
	if (args.inst_type.isSet)
		mInstructionType = getInstructionTypeFromString(*args.inst_type);
	else
		mInstructionType = nx::npdm::INSTR_64BIT; // default 64bit
	
	mListApi = args.list_api.isSet;
	mListSymbols = args.list_sym.isSet;

	// determine output path
	if (args.verbose_output.isSet)
		mOutputType = OUTPUT_VERBOSE;
	else if (args.minimal_output.isSet)
		mOutputType = OUTPUT_MINIMAL;
	else
		mOutputType = OUTPUT_NORMAL;

	// determine input file type
	if (args.file_type.isSet)
		mFileType = getFileTypeFromString(*args.file_type);
	else
		mFileType = determineFileTypeFromFile(mInputPath);
	
	// check is the input file could be identified
	if (mFileType == FILE_INVALID)
		throw fnd::Exception(kModuleName, "Unknown file type.");
}


void UserSettings::decodeHexStringToBytes(const std::string& name, const std::string& str, byte_t* out, size_t out_len)
{
	size_t size = str.size();
	if ((size % 2) || ((size / 2) != out_len))
	{
		throw fnd::Exception(kModuleName, "Key: \"" + name + "\" has incorrect length");
	}

	for (size_t i = 0; i < out_len; i++)
	{
		out[i] = (charToByte(str[i * 2]) << 4) | charToByte(str[(i * 2) + 1]);
	}
}

FileType UserSettings::getFileTypeFromString(const std::string& type_str)
{
	std::string str = type_str;
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	
	FileType type;
	if (str == "xci")
		type = FILE_XCI;
	else if (str == "nsp")
		type = FILE_NSP;
	else if (str == "partitionfs" || str == "hashedpartitionfs"  \
			 || str == "pfs" || str == "pfs0" \
			 || str == "hfs" || str == "hfs0")
		type = FILE_PARTITIONFS;
	else if (str == "romfs")
		type = FILE_ROMFS;
	else if (str == "nca")
		type = FILE_NCA;
	else if (str == "npdm")
		type = FILE_NPDM;
	else if (str == "cnmt")
		type = FILE_CNMT;
	else if (str == "nso")
		type = FILE_NSO;
	else if (str == "nro")
		type = FILE_NRO;
	else
		type = FILE_INVALID;

	return type;
}

FileType UserSettings::determineFileTypeFromFile(const std::string& path)
{
	static const size_t kMaxReadSize = 0x1000;
	FileType file_type = FILE_INVALID;
	fnd::SimpleFile file;
	fnd::MemoryBlob scratch;

	// open file
	file.open(path, file.Read);

	// read file
	scratch.alloc(MIN(kMaxReadSize, file.size()));
	file.read(scratch.getBytes(), 0, scratch.getSize());
	// close file
	file.close();

	fnd::SimpleTextOutput::hxdStyleDump(scratch.getBytes(), scratch.getSize());

	// prepare decrypted NCA data
	byte_t nca_raw[nx::nca::kHeaderSize];
	nx::sNcaHeader* nca_header = (nx::sNcaHeader*)(nca_raw + nx::NcaUtils::sectorToOffset(1));
	
	if (scratch.getSize() >= nx::nca::kHeaderSize)
	{
		nx::NcaUtils::decryptNcaHeader(scratch.getBytes(), nca_raw, mKeyset.nca.header_key);
	}

	// _QUICK_CAST resolves to a pointer of type 'st' located at scratch.getBytes() + 'oft'
#define _QUICK_CAST(st, oft) ((st*)(scratch.getBytes() + (oft)))
#define _ASSERT_SIZE(size) (scratch.getSize() >= (size))

	// test npdm
	if (_ASSERT_SIZE(sizeof(nx::sXciHeaderPage)) && _QUICK_CAST(nx::sXciHeaderPage, 0)->header.signature.get() == nx::xci::kXciSig)
		file_type = FILE_XCI;
	// test pfs0
	else if (_ASSERT_SIZE(sizeof(nx::sPfsHeader)) && _QUICK_CAST(nx::sPfsHeader, 0)->signature.get() == nx::pfs::kPfsSig)
		file_type = FILE_PARTITIONFS;
	// test hfs0
	else if (_ASSERT_SIZE(sizeof(nx::sPfsHeader)) && _QUICK_CAST(nx::sPfsHeader, 0)->signature.get() == nx::pfs::kHashedPfsSig)
		file_type = FILE_PARTITIONFS;
	// test romfs
	else if (_ASSERT_SIZE(sizeof(nx::sRomfsHeader)) && _QUICK_CAST(nx::sRomfsHeader, 0)->header_size.get() == sizeof(nx::sRomfsHeader) && _QUICK_CAST(nx::sRomfsHeader, 0)->sections[1].offset.get() == (_QUICK_CAST(nx::sRomfsHeader, 0)->sections[0].offset.get() + _QUICK_CAST(nx::sRomfsHeader, 0)->sections[0].size.get()))
		file_type = FILE_ROMFS;
	// test nca2
	else if (_ASSERT_SIZE(nx::nca::kHeaderSize) && nca_header->signature.get() == nx::nca::kNca2Sig)
		file_type = FILE_NCA;
	// test nca3
	else if (_ASSERT_SIZE(nx::nca::kHeaderSize) && nca_header->signature.get() == nx::nca::kNca3Sig)
		file_type = FILE_NCA;
	// test npdm
	else if (_ASSERT_SIZE(sizeof(nx::sNpdmHeader)) && _QUICK_CAST(nx::sNpdmHeader, 0)->signature.get() == nx::npdm::kNpdmStructSig)
		file_type = FILE_NPDM;
	// test nso
	else if (_ASSERT_SIZE(sizeof(nx::sNsoHeader)) && _QUICK_CAST(nx::sNsoHeader, 0)->signature.get() == nx::nso::kNsoSig)
		file_type = FILE_NSO;
	// test nso
	else if (_ASSERT_SIZE(sizeof(nx::sNroHeader)) && _QUICK_CAST(nx::sNroHeader, 0)->signature.get() == nx::nro::kNroSig)
		file_type = FILE_NRO;
	// else unrecognised
	else
		file_type = FILE_INVALID;

#undef _ASSERT_SIZE
#undef _QUICK_CAST

	return file_type;
}

nx::npdm::InstructionType UserSettings::getInstructionTypeFromString(const std::string & type_str)
{
	std::string str = type_str;
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);

	nx::npdm::InstructionType type;
	if (str == "32bit")
		type = nx::npdm::INSTR_32BIT;
	else if (str == "64bit")
		type = nx::npdm::INSTR_64BIT;
	else
		throw fnd::Exception(kModuleName, "Unsupported instruction type: " + str);

	return type;
}
