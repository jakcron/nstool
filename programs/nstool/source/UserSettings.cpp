#include "UserSettings.h"
#include "version.h"
#include "PkiValidator.h"
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <fnd/io.h>
#include <fnd/SimpleFile.h>
#include <fnd/SimpleTextOutput.h>
#include <fnd/Vec.h>
#include <fnd/ResourceFileReader.h>
#include <nn/hac/NcaUtils.h>
#include <nn/hac/AesKeygen.h>
#include <nn/hac/xci.h>
#include <nn/hac/pfs.h>
#include <nn/hac/nca.h>
#include <nn/hac/npdm.h>
#include <nn/hac/romfs.h>
#include <nn/hac/cnmt.h>
#include <nn/hac/nacp.h>
#include <nn/hac/nso.h>
#include <nn/hac/nro.h>
#include <nn/hac/aset.h>
#include <nn/pki/SignedData.h>
#include <nn/pki/CertificateBody.h>
#include <nn/pki/SignUtils.h>
#include <nn/es/TicketBody_V2.h>

UserSettings::UserSettings()
{}

void UserSettings::parseCmdArgs(const std::vector<std::string>& arg_list)
{
	sCmdArgs args;
	populateCmdArgs(arg_list, args);
	populateKeyset(args);
	populateUserSettings(args);
}

void UserSettings::showHelp()
{
	printf("NSTool v%d.%d.%d (C) %s\n", VER_MAJOR, VER_MINOR, VER_PATCH, AUTHORS);
	printf("Built: %s %s\n\n", __TIME__, __DATE__);
	
	printf("Usage: nstool [options... ] <file>\n");
	printf("\n  General Options:\n");
	printf("      -d, --dev       Use devkit keyset.\n");
	printf("      -k, --keyset    Specify keyset file.\n");
	printf("      -t, --type      Specify input file type. [xci, pfs, romfs, nca, npdm, cnmt, nso, nro, nacp, aset, cert, tik]\n");
	printf("      -y, --verify    Verify file.\n");
	printf("\n  Output Options:\n");
	printf("      --showkeys      Show keys generated.\n");
	printf("      --showlayout    Show layout metadata.\n");
	printf("      -v, --verbose   Verbose output.\n");
	printf("\n  XCI (GameCard Image)\n");
	printf("    nstool [--listfs] [--update <dir> --logo <dir> --normal <dir> --secure <dir>] <.xci file>\n");
	printf("      --listfs        Print file system in embedded partitions.\n");
	printf("      --update        Extract \"update\" partition to directory.\n");
	printf("      --logo          Extract \"logo\" partition to directory.\n");
	printf("      --normal        Extract \"normal\" partition to directory.\n");
	printf("      --secure        Extract \"secure\" partition to directory.\n");
	printf("\n  PFS0/HFS0 (PartitionFs), RomFs, NSP (Ninendo Submission Package)\n");
	printf("    nstool [--listfs] [--fsdir <dir>] <file>\n");
	printf("      --listfs        Print file system.\n");
	printf("      --fsdir         Extract file system to directory.\n");
	printf("\n  NCA (Nintendo Content Archive)\n");
	printf("    nstool [--listfs] [--bodykey <key> --titlekey <key>] [--part0 <dir> ...] <.nca file>\n");
	printf("      --listfs        Print file system in embedded partitions.\n");
	printf("      --titlekey      Specify title key extracted from ticket.\n");
	printf("      --bodykey       Specify body encryption key.\n");
	printf("      --tik           Specify ticket to source title key.\n");
	printf("      --cert          Specify certificate chain to verify ticket.\n");
	printf("      --part0         Extract \"partition 0\" to directory.\n");
	printf("      --part1         Extract \"partition 1\" to directory.\n");
	printf("      --part2         Extract \"partition 2\" to directory.\n");
	printf("      --part3         Extract \"partition 3\" to directory.\n");
	printf("\n  NSO (Nintendo Software Object), NRO (Nintendo Relocatable Object)\n");
	printf("    nstool [--listapi --listsym] [--insttype <inst. type>] <file>\n");
	printf("      --listapi       Print SDK API List.\n");
	printf("      --listsym       Print Code Symbols.\n");
	printf("      --insttype      Specify instruction type [64bit|32bit] (64bit is assumed).\n");
	printf("\n  ASET (Homebrew Asset Blob)\n");
	printf("    nstool [--listfs] [--icon <file> --nacp <file> --fsdir <dir>] <file>\n");
	printf("      --listfs        Print filesystem in embedded RomFS partition.\n");
	printf("      --icon          Extract icon partition to file.\n");
	printf("      --nacp          Extract NACP partition to file.\n");
	printf("      --fsdir         Extract RomFS partition to directory.\n");

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

CliOutputMode UserSettings::getCliOutputMode() const
{
	return mOutputMode;
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

nn::hac::npdm::InstructionType UserSettings::getInstType() const
{
	return mInstructionType;
}

const sOptional<std::string>& UserSettings::getXciUpdatePath() const
{
	return mXciUpdatePath;
}

const sOptional<std::string>& UserSettings::getXciLogoPath() const
{
	return mXciLogoPath;
}

const sOptional<std::string>& UserSettings::getXciNormalPath() const
{
	return mXciNormalPath;
}

const sOptional<std::string>& UserSettings::getXciSecurePath() const
{
	return mXciSecurePath;
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

const sOptional<std::string>& UserSettings::getAssetIconPath() const
{
	return mAssetIconPath;
}

const sOptional<std::string>& UserSettings::getAssetNacpPath() const
{
	return mAssetNacpPath;
}

const fnd::List<nn::pki::SignedData<nn::pki::CertificateBody>>& UserSettings::getCertificateChain() const
{
	return mCertChain;
}

void UserSettings::populateCmdArgs(const std::vector<std::string>& arg_list, sCmdArgs& cmd_args)
{
	// show help text
	if (arg_list.size() < 2)
	{
		showHelp();
		throw fnd::Exception(kModuleName, "Not enough arguments.");
	}

	cmd_args.input_path = arg_list.back();

	for (size_t i = 1; i < arg_list.size(); i++)
	{
		if (arg_list[i] == "-h" || arg_list[i] == "--help")
		{
			showHelp();
			throw fnd::Exception(kModuleName, "Nothing to do.");
		}
	}

	for (size_t i = 1; i+1 < arg_list.size(); i++)
	{
		bool hasParamter = arg_list[i+1][0] != '-' && i+2 < arg_list.size();

		if (arg_list[i] == "-d" || arg_list[i] == "--dev")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " does not take a parameter.");
			cmd_args.devkit_keys = true;
		}

		else if (arg_list[i] == "-y" || arg_list[i] == "--verify")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " does not take a parameter.");
			cmd_args.verify_file = true;
		}

		else if (arg_list[i] == "--showkeys")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " does not take a parameter.");
			cmd_args.show_keys = true;
		}

		else if (arg_list[i] == "--showlayout")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " does not take a parameter.");
			cmd_args.show_layout = true;
		}

		else if (arg_list[i] == "-v" || arg_list[i] == "--verbose")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " does not take a parameter.");
			cmd_args.verbose_output = true;
		}

		else if (arg_list[i] == "-k" || arg_list[i] == "--keyset")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.keyset_path = arg_list[i+1];
		}

		else if (arg_list[i] == "-t" || arg_list[i] == "--type")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.file_type = arg_list[i+1];
		}

		else if (arg_list[i] == "--listfs")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " does not take a parameter.");
			cmd_args.list_fs = true;
		}

		else if (arg_list[i] == "--update")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.update_path = arg_list[i+1];
		}

		else if (arg_list[i] == "--normal")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.normal_path = arg_list[i+1];
		}

		else if (arg_list[i] == "--secure")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.secure_path = arg_list[i+1];
		}

		else if (arg_list[i] == "--logo")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.logo_path = arg_list[i+1];
		}

		else if (arg_list[i] == "--fsdir")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.fs_path = arg_list[i+1];
		}

		else if (arg_list[i] == "--titlekey")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.nca_titlekey = arg_list[i+1];
		}

		else if (arg_list[i] == "--bodykey")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.nca_bodykey = arg_list[i+1];
		}

		else if (arg_list[i] == "--tik")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.ticket_path = arg_list[i+1];
		}

		else if (arg_list[i] == "--cert")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.cert_path = arg_list[i+1];
		}

		else if (arg_list[i] == "--part0")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.part0_path = arg_list[i+1];
		}

		else if (arg_list[i] == "--part1")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.part1_path = arg_list[i+1];
		}

		else if (arg_list[i] == "--part2")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.part2_path = arg_list[i+1];
		}

		else if (arg_list[i] == "--part3")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.part3_path = arg_list[i+1];
		}

		else if (arg_list[i] == "--listapi")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " does not take a parameter.");
			cmd_args.list_api = true;
		}

		else if (arg_list[i] == "--listsym")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " does not take a parameter.");
			cmd_args.list_sym = true;
		}

		else if (arg_list[i] == "--insttype")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.inst_type = arg_list[i + 1];
		}

		else if (arg_list[i] == "--icon")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.asset_icon_path = arg_list[i + 1];
		}

		else if (arg_list[i] == "--nacp")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.asset_nacp_path = arg_list[i + 1];
		}

		else
		{
			throw fnd::Exception(kModuleName, arg_list[i] + " is not recognised.");
		}

		i += hasParamter;
	}
}

void UserSettings::populateKeyset(sCmdArgs& args)
{
	fnd::aes::sAes128Key zeros_aes_key;
	fnd::aes::sAesXts128Key zeros_aes_xts_key;
	memset((void*)&zeros_aes_key, 0, sizeof(fnd::aes::sAes128Key));
	memset((void*)&zeros_aes_xts_key, 0, sizeof(fnd::aes::sAesXts128Key));
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
	const std::string kPkiRootBase = "pki_root";
	const std::string kTicketCommonKeyBase[2] = { "titlekek", "ticket_commonkey" };
	const std::string kNcaBodyBase[2] = {"key_area_key", "nca_body_keak"};
	const std::string kNcaBodyKeakIndexName[3] = {"application", "ocean", "system"};


	// sources
	fnd::aes::sAes128Key master_key[kMasterKeyNum] = { zeros_aes_key };
	fnd::aes::sAes128Key package2_key_source = zeros_aes_key;
	fnd::aes::sAes128Key ticket_titlekek_source = zeros_aes_key;
	fnd::aes::sAes128Key key_area_key_source[3] = { zeros_aes_key, zeros_aes_key, zeros_aes_key };
	fnd::aes::sAes128Key aes_kek_generation_source = zeros_aes_key;
	fnd::aes::sAes128Key aes_key_generation_source = zeros_aes_key;
	fnd::aes::sAes128Key nca_header_kek_source = zeros_aes_key;
	fnd::aes::sAesXts128Key nca_header_key_source = zeros_aes_xts_key;


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
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kNcaHeaderBase[1], kRsaKeySuffix[0]), mKeyset.nca.header_sign_key.priv_exponent, fnd::rsa::kRsa2048Size);
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kNcaHeaderBase[1], kRsaKeySuffix[1]), mKeyset.nca.header_sign_key.modulus, fnd::rsa::kRsa2048Size);
	
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kXciHeaderBase, kRsaKeySuffix[0]), mKeyset.xci.header_sign_key.priv_exponent, fnd::rsa::kRsa2048Size);
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kXciHeaderBase, kRsaKeySuffix[1]), mKeyset.xci.header_sign_key.modulus, fnd::rsa::kRsa2048Size);

	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kAcidBase, kRsaKeySuffix[0]), mKeyset.acid_sign_key.priv_exponent, fnd::rsa::kRsa2048Size);
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kAcidBase, kRsaKeySuffix[1]), mKeyset.acid_sign_key.modulus, fnd::rsa::kRsa2048Size);

	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kPackage2Base, kRsaKeySuffix[0]), mKeyset.package2_sign_key.priv_exponent, fnd::rsa::kRsa2048Size);
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kPackage2Base, kRsaKeySuffix[1]), mKeyset.package2_sign_key.modulus, fnd::rsa::kRsa2048Size);

	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kPkiRootBase, kRsaKeySuffix[0]), mKeyset.pki.root_sign_key.priv_exponent, fnd::rsa::kRsa4096Size);
	_SAVE_KEYDATA(_CONCAT_2_STRINGS(kPkiRootBase, kRsaKeySuffix[1]), mKeyset.pki.root_sign_key.modulus, fnd::rsa::kRsa4096Size);


	// save keydata from input args
	if (args.nca_bodykey.isSet)
	{
		if (args.nca_bodykey.var.length() == (sizeof(fnd::aes::sAes128Key)*2))
		{
			decodeHexStringToBytes("--bodykey", args.nca_bodykey.var, mKeyset.nca.manual_body_key_aesctr.key, sizeof(fnd::aes::sAes128Key));
		}
		else
		{
			decodeHexStringToBytes("--bodykey", args.nca_bodykey.var, mKeyset.nca.manual_body_key_aesxts.key[0], sizeof(fnd::aes::sAesXts128Key));
		}
	}

	if (args.nca_titlekey.isSet)
	{
		if (args.nca_titlekey.var.length() == (sizeof(fnd::aes::sAes128Key)*2))
		{
			decodeHexStringToBytes("--titlekey", args.nca_titlekey.var, mKeyset.nca.manual_title_key_aesctr.key, sizeof(fnd::aes::sAes128Key));
		}
		else
		{
			decodeHexStringToBytes("--titlekey", args.nca_titlekey.var, mKeyset.nca.manual_title_key_aesxts.key[0], sizeof(fnd::aes::sAesXts128Key));
		}
	}

	// import certificate chain
	if (args.cert_path.isSet)
	{
		fnd::SimpleFile cert_file;
		fnd::Vec<byte_t> cert_raw;
		nn::pki::SignedData<nn::pki::CertificateBody> cert;

		cert_file.open(args.cert_path.var, fnd::SimpleFile::Read);
		cert_raw.alloc(cert_file.size());
		cert_file.read(cert_raw.data(), cert_raw.size());

		for (size_t i = 0; i < cert_raw.size(); i+= cert.getBytes().size())
		{
			cert.fromBytes(cert_raw.data() + i, cert_raw.size() - i);
			mCertChain.addElement(cert);
		}
	}

	// get titlekey from ticket
	if (args.ticket_path.isSet)
	{
		fnd::SimpleFile tik_file;
		fnd::Vec<byte_t> tik_raw;
		nn::pki::SignedData<nn::es::TicketBody_V2> tik;

		// open and import ticket
		tik_file.open(args.ticket_path.var, fnd::SimpleFile::Read);
		tik_raw.alloc(tik_file.size());
		tik_file.read(tik_raw.data(), tik_raw.size());
		tik.fromBytes(tik_raw.data(), tik_raw.size());

		// validate ticket signature
		if (mCertChain.size() > 0)
		{
			PkiValidator pki_validator;
			fnd::Vec<byte_t> tik_hash;

			switch (nn::pki::sign::getHashAlgo(tik.getSignature().getSignType()))
			{
			case (nn::pki::sign::HASH_ALGO_SHA1):
				tik_hash.alloc(fnd::sha::kSha1HashLen);
				fnd::sha::Sha1(tik.getBody().getBytes().data(), tik.getBody().getBytes().size(), tik_hash.data());
				break;
			case (nn::pki::sign::HASH_ALGO_SHA256):
				tik_hash.alloc(fnd::sha::kSha256HashLen);
				fnd::sha::Sha256(tik.getBody().getBytes().data(), tik.getBody().getBytes().size(), tik_hash.data());
				break;
			}

			try 
			{
				pki_validator.setRootKey(mKeyset.pki.root_sign_key);
				pki_validator.addCertificates(mCertChain);
				pki_validator.validateSignature(tik.getBody().getIssuer(), tik.getSignature().getSignType(), tik.getSignature().getSignature(), tik_hash);
			}
			catch (const fnd::Exception& e)
			{
				std::cout << "[WARNING] Ticket signature could not be validated (" << e.error() << ")" << std::endl;
			}
			
		}

		// extract title key
		if (tik.getBody().getTitleKeyEncType() == nn::es::ticket::AES128_CBC)
		{
			memcpy(mKeyset.nca.manual_title_key_aesctr.key, tik.getBody().getEncTitleKey(), fnd::aes::kAes128KeySize);
		}
		else
		{
			std::cout << "[WARNING] Titlekey not imported from ticket because it is personalised" << std::endl;
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
						fnd::aes::sAes128Key nca_header_kek;
						nn::hac::AesKeygen::generateKey(nca_header_kek.key, aes_kek_generation_source.key, nca_header_kek_source.key, aes_key_generation_source.key, master_key[i].key);
						nn::hac::AesKeygen::generateKey(mKeyset.nca.header_key.key[0], nca_header_key_source.key[0], nca_header_kek.key);
						nn::hac::AesKeygen::generateKey(mKeyset.nca.header_key.key[1], nca_header_key_source.key[1], nca_header_kek.key);
						//printf("nca header key[0] ");
						//fnd::SimpleTextOutput::hexDump(mKeyset.nca.header_key.key[0], 0x10);
						//printf("nca header key[1] ");
						//fnd::SimpleTextOutput::hexDump(mKeyset.nca.header_key.key[1], 0x10);
					}
				}

				for (size_t j = 0; j < nn::hac::nca::kKeyAreaEncryptionKeyNum; j++)
				{
					if (key_area_key_source[j] != zeros_aes_key && mKeyset.nca.key_area_key[j][i] == zeros_aes_key)
					{
						nn::hac::AesKeygen::generateKey(mKeyset.nca.key_area_key[j][i].key, aes_kek_generation_source.key, key_area_key_source[j].key, aes_key_generation_source.key, master_key[i].key);
						//printf("nca keak %d/%02d ", j, i);
						//fnd::SimpleTextOutput::hexDump(mKeyset.nca.key_area_key[j][i].key, 0x10);
					}
				}
			}

			if (ticket_titlekek_source != zeros_aes_key && mKeyset.ticket.titlekey_kek[i] == zeros_aes_key)
			{
				nn::hac::AesKeygen::generateKey(mKeyset.ticket.titlekey_kek[i].key, ticket_titlekek_source.key, master_key[i].key);
				//printf("ticket titlekek %02d ", i);
				//fnd::SimpleTextOutput::hexDump(mKeyset.ticket.titlekey_kek[i].key, 0x10);
			}
			if (package2_key_source != zeros_aes_key && mKeyset.package2_key[i] == zeros_aes_key)
			{
				nn::hac::AesKeygen::generateKey(mKeyset.package2_key[i].key, package2_key_source.key, master_key[i].key);
				//printf("package2 key %02d ", i);
				//fnd::SimpleTextOutput::hexDump(mKeyset.package2_key[i].key, 0x10);
			}
		}
		/*
		for (size_t j = 0; j < nn::hac::nca::kKeyAreaEncryptionKeyNum; j++)
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
		mInstructionType = nn::hac::npdm::INSTR_64BIT; // default 64bit
	
	mListApi = args.list_api.isSet;
	mListSymbols = args.list_sym.isSet;

	mAssetIconPath = args.asset_icon_path;
	mAssetNacpPath = args.asset_nacp_path;

	// determine output mode
	mOutputMode = _BIT(OUTPUT_BASIC);
	if (args.verbose_output.isSet)
	{
		mOutputMode |= _BIT(OUTPUT_KEY_DATA);
		mOutputMode |= _BIT(OUTPUT_LAYOUT);
		mOutputMode |= _BIT(OUTPUT_EXTENDED);
	}
	if (args.show_keys.isSet)
	{
		mOutputMode |= _BIT(OUTPUT_KEY_DATA);
	}
	if (args.show_layout.isSet)
	{
		mOutputMode |= _BIT(OUTPUT_LAYOUT);
	}

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
	else if (str == "nacp")
		type = FILE_NACP;
	else if (str == "cert")
		type = FILE_PKI_CERT;
	else if (str == "tik")
		type = FILE_ES_TIK;
	else if (str == "aset" || str == "asset")
		type = FILE_HB_ASSET;
	else
		type = FILE_INVALID;

	return type;
}

FileType UserSettings::determineFileTypeFromFile(const std::string& path)
{
	static const size_t kMaxReadSize = 0x4000;
	FileType file_type = FILE_INVALID;
	fnd::SimpleFile file;
	fnd::Vec<byte_t> scratch;

	// open file
	file.open(path, file.Read);

	// read file
	scratch.alloc(_MIN(kMaxReadSize, file.size()));
	file.read(scratch.data(), 0, scratch.size());
	// close file
	file.close();

	// _TYPE_PTR resolves to a pointer of type 'st' located at scratch.data()
#define _TYPE_PTR(st) ((st*)(scratch.data()))
#define _ASSERT_SIZE(sz) (scratch.size() >= (sz))

	// test npdm
	if (_ASSERT_SIZE(sizeof(nn::hac::sXciHeaderPage)) && _TYPE_PTR(nn::hac::sXciHeaderPage)->header.st_magic.get() == nn::hac::xci::kXciStructMagic)
		file_type = FILE_XCI;
	// test pfs0
	else if (_ASSERT_SIZE(sizeof(nn::hac::sPfsHeader)) && _TYPE_PTR(nn::hac::sPfsHeader)->st_magic.get() == nn::hac::pfs::kPfsStructMagic)
		file_type = FILE_PARTITIONFS;
	// test hfs0
	else if (_ASSERT_SIZE(sizeof(nn::hac::sPfsHeader)) && _TYPE_PTR(nn::hac::sPfsHeader)->st_magic.get() == nn::hac::pfs::kHashedPfsStructMagic)
		file_type = FILE_PARTITIONFS;
	// test romfs
	else if (_ASSERT_SIZE(sizeof(nn::hac::sRomfsHeader)) && _TYPE_PTR(nn::hac::sRomfsHeader)->header_size.get() == sizeof(nn::hac::sRomfsHeader) && _TYPE_PTR(nn::hac::sRomfsHeader)->sections[1].offset.get() == (_TYPE_PTR(nn::hac::sRomfsHeader)->sections[0].offset.get() + _TYPE_PTR(nn::hac::sRomfsHeader)->sections[0].size.get()))
		file_type = FILE_ROMFS;
	// test npdm
	else if (_ASSERT_SIZE(sizeof(nn::hac::sNpdmHeader)) && _TYPE_PTR(nn::hac::sNpdmHeader)->st_magic.get() == nn::hac::npdm::kNpdmStructMagic)
		file_type = FILE_NPDM;
	// test nca
	else if (determineValidNcaFromSample(scratch))
		file_type = FILE_NCA;
	// test cnmt
	else if (determineValidCnmtFromSample(scratch))
		file_type = FILE_CNMT;
	// test nacp
	else if (determineValidNacpFromSample(scratch))
		file_type = FILE_NACP;
	// test nso
	else if (_ASSERT_SIZE(sizeof(nn::hac::sNsoHeader)) && _TYPE_PTR(nn::hac::sNsoHeader)->st_magic.get() == nn::hac::nso::kNsoStructMagic)
		file_type = FILE_NSO;
	// test nso
	else if (_ASSERT_SIZE(sizeof(nn::hac::sNroHeader)) && _TYPE_PTR(nn::hac::sNroHeader)->st_magic.get() == nn::hac::nro::kNroStructMagic)
		file_type = FILE_NRO;
	// test pki certificate
	else if (determineValidEsCertFromSample(scratch))
		file_type = FILE_PKI_CERT;
	// test ticket
	else if (determineValidEsTikFromSample(scratch))
		file_type = FILE_ES_TIK;
	// test hb asset
	else if (_ASSERT_SIZE(sizeof(nn::hac::sAssetHeader)) && _TYPE_PTR(nn::hac::sAssetHeader)->st_magic.get() == nn::hac::aset::kAssetStructMagic)
		file_type = FILE_HB_ASSET;
	// else unrecognised
	else
		file_type = FILE_INVALID;

#undef _ASSERT_SIZE
#undef _TYPE_PTR

	return file_type;
}

bool UserSettings::determineValidNcaFromSample(const fnd::Vec<byte_t>& sample) const
{
	// prepare decrypted NCA data
	byte_t nca_raw[nn::hac::nca::kHeaderSize];
	nn::hac::sNcaHeader* nca_header = (nn::hac::sNcaHeader*)(nca_raw + nn::hac::NcaUtils::sectorToOffset(1));
	
	if (sample.size() < nn::hac::nca::kHeaderSize)
		return false;

	nn::hac::NcaUtils::decryptNcaHeader(sample.data(), nca_raw, mKeyset.nca.header_key);

	if (nca_header->st_magic.get() != nn::hac::nca::kNca2StructMagic && nca_header->st_magic.get() != nn::hac::nca::kNca3StructMagic)
		return false;

	return true;
}

bool UserSettings::determineValidCnmtFromSample(const fnd::Vec<byte_t>& sample) const
{
	if (sample.size() < sizeof(nn::hac::sContentMetaHeader))
		return false;

	const nn::hac::sContentMetaHeader* data = (const nn::hac::sContentMetaHeader*)sample.data();

	size_t minimum_size = sizeof(nn::hac::sContentMetaHeader) + data->exhdr_size.get() + data->content_count.get() * sizeof(nn::hac::sContentInfo) + data->content_meta_count.get() * sizeof(nn::hac::sContentMetaInfo) + nn::hac::cnmt::kDigestLen;

	if (sample.size() < minimum_size)
		return false;

	if (data->type == nn::hac::cnmt::METATYPE_APPLICATION)
	{
		const nn::hac::sApplicationMetaExtendedHeader* meta = (const nn::hac::sApplicationMetaExtendedHeader*)(sample.data() + sizeof(nn::hac::sContentMetaHeader));
		if ((meta->patch_id.get() & data->id.get()) != data->id.get())
			return false;
	}
	else if (data->type == nn::hac::cnmt::METATYPE_PATCH)
	{
		const nn::hac::sPatchMetaExtendedHeader* meta = (const nn::hac::sPatchMetaExtendedHeader*)(sample.data() + sizeof(nn::hac::sContentMetaHeader));
		if ((meta->application_id.get() & data->id.get()) != meta->application_id.get())
			return false;

		minimum_size += meta->extended_data_size.get();
	}
	else if (data->type == nn::hac::cnmt::METATYPE_ADD_ON_CONTENT)
	{
		const nn::hac::sAddOnContentMetaExtendedHeader* meta = (const nn::hac::sAddOnContentMetaExtendedHeader*)(sample.data() + sizeof(nn::hac::sContentMetaHeader));
		if ((meta->application_id.get() & data->id.get()) != meta->application_id.get())
			return false;
	}
	else if (data->type == nn::hac::cnmt::METATYPE_DELTA)
	{
		const nn::hac::sDeltaMetaExtendedHeader* meta = (const nn::hac::sDeltaMetaExtendedHeader*)(sample.data() + sizeof(nn::hac::sContentMetaHeader));
		if ((meta->application_id.get() & data->id.get()) != meta->application_id.get())
			return false;

		minimum_size += meta->extended_data_size.get();
	}

	if (sample.size() != minimum_size)
		return false;

	return true;
}

bool UserSettings::determineValidNacpFromSample(const fnd::Vec<byte_t>& sample) const
{
	if (sample.size() != sizeof(nn::hac::sApplicationControlProperty))
		return false;

	const nn::hac::sApplicationControlProperty* data = (const nn::hac::sApplicationControlProperty*)sample.data();

	if (data->logo_type > nn::hac::nacp::LOGO_Nintendo)
		return false;

	if (data->display_version[0] == 0)
		return false;

	if (data->user_account_save_data_size.get() == 0 && data->user_account_save_data_journal_size.get() != 0)
		return false;
	
	if (data->user_account_save_data_journal_size.get() == 0 && data->user_account_save_data_size.get() != 0)
		return false;

	if (data->supported_language_flag.get() == 0)
		return false;

	return true;
}

bool UserSettings::determineValidEsCertFromSample(const fnd::Vec<byte_t>& sample) const
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

bool UserSettings::determineValidEsTikFromSample(const fnd::Vec<byte_t>& sample) const
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

	return true;
}

nn::hac::npdm::InstructionType UserSettings::getInstructionTypeFromString(const std::string & type_str)
{
	std::string str = type_str;
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);

	nn::hac::npdm::InstructionType type;
	if (str == "32bit")
		type = nn::hac::npdm::INSTR_32BIT;
	else if (str == "64bit")
		type = nn::hac::npdm::INSTR_64BIT;
	else
		throw fnd::Exception(kModuleName, "Unsupported instruction type: " + str);

	return type;
}
