#include <tc.h>
#include <tc/os/UnicodeMain.h>
#include "Settings.h"

/*
#include "GameCardProcess.h"
#include "PfsProcess.h"
#include "RomfsProcess.h"
#include "NcaProcess.h"
#include "MetaProcess.h"
#include "CnmtProcess.h"
#include "NsoProcess.h"
#include "NroProcess.h"
#include "NacpProcess.h"
#include "IniProcess.h"
#include "KipProcess.h"
#include "PkiCertProcess.h"
#include "EsTikProcess.h"
#include "AssetProcess.h"
*/

int umain(const std::vector<std::string>& args, const std::vector<std::string>& env)
{
	try 
	{
		nstool::Settings set = nstool::SettingsInitializer(args);
		
		std::shared_ptr<tc::io::IStream> infile_stream = std::make_shared<tc::io::FileStream>(tc::io::FileStream(set.infile.path.get(), tc::io::FileMode::Open, tc::io::FileAccess::Read));

		if (set.infile.filetype == nstool::Settings::FILE_TYPE_GAMECARD)
		{	
			GameCardProcess obj;

			obj.setInputFile(infile_stream);
			
			obj.setKeyCfg(set.opt.keybag);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(set.opt.verify);

			if (set.xci.update_extract_path.isSet())
				obj.setPartitionForExtract(nn::hac::gc::kUpdatePartitionStr, set.xci.update_extract_path.get());
			if (set.xci.logo_extract_path.isSet())
				obj.setPartitionForExtract(nn::hac::gc::kLogoPartitionStr, set.xci.logo_extract_path.get());
			if (user_set.getXciNormalPath().isSet())
				obj.setPartitionForExtract(nn::hac::gc::kNormalPartitionStr, user_set.getXciNormalPath().get());
			if (user_set.getXciSecurePath().isSet())
				obj.setPartitionForExtract(nn::hac::gc::kSecurePartitionStr, user_set.getXciSecurePath().get());
			obj.setListFs(user_set.isListFs());

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_PARTITIONFS || set.infile.filetype == nstool::Settings::FILE_TYPE_NSP)
		{
			PfsProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(set.opt.verify);

			if (user_set.getFsPath().isSet())
				obj.setExtractPath(user_set.getFsPath().get());
			obj.setListFs(user_set.isListFs());
			
			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_ROMFS)
		{
			RomfsProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(set.opt.verify);

			if (user_set.getFsPath().isSet())
				obj.setExtractPath(user_set.getFsPath().get());
			obj.setListFs(user_set.isListFs());

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_NCA)
		{
			NcaProcess obj;

			obj.setInputFile(infile_stream);
			obj.setKeyCfg(set.opt.keybag);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(set.opt.verify);


			if (user_set.getNcaPart0Path().isSet())
				obj.setPartition0ExtractPath(user_set.getNcaPart0Path().get());
			if (user_set.getNcaPart1Path().isSet())
				obj.setPartition1ExtractPath(user_set.getNcaPart1Path().get());
			if (user_set.getNcaPart2Path().isSet())
				obj.setPartition2ExtractPath(user_set.getNcaPart2Path().get());
			if (user_set.getNcaPart3Path().isSet())
				obj.setPartition3ExtractPath(user_set.getNcaPart3Path().get());
			obj.setListFs(user_set.isListFs());

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_META)
		{
			MetaProcess obj;

			obj.setInputFile(infile_stream);
			obj.setKeyCfg(set.opt.keybag);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(set.opt.verify);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_CNMT)
		{
			CnmtProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(set.opt.verify);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_NSO)
		{
			NsoProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(set.opt.verify);
			
			obj.setIs64BitInstruction(user_set.getIs64BitInstruction());
			obj.setListApi(user_set.isListApi());
			obj.setListSymbols(user_set.isListSymbols());

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_NRO)
		{
			NroProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(set.opt.verify);
			
			obj.setIs64BitInstruction(user_set.getIs64BitInstruction());
			obj.setListApi(user_set.isListApi());
			obj.setListSymbols(user_set.isListSymbols());

			if (user_set.getAssetIconPath().isSet())
				obj.setAssetIconExtractPath(user_set.getAssetIconPath().get());
			if (user_set.getAssetNacpPath().isSet())
				obj.setAssetNacpExtractPath(user_set.getAssetNacpPath().get());

			if (user_set.getFsPath().isSet())
				obj.setAssetRomfsExtractPath(user_set.getFsPath().get());
			obj.setAssetListFs(user_set.isListFs());

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_NACP)
		{
			NacpProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(set.opt.verify);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_INI)
		{
			IniProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(set.opt.verify);

			if (user_set.getKipExtractPath().isSet())
				obj.setKipExtractPath(user_set.getKipExtractPath().get());

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_KIP)
		{
			KipProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(set.opt.verify);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_PKI_CERT)
		{
			PkiCertProcess obj;

			obj.setInputFile(infile_stream);
			obj.setKeyCfg(set.opt.keybag);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(set.opt.verify);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_ES_TIK)
		{
			EsTikProcess obj;

			obj.setInputFile(infile_stream);
			obj.setKeyCfg(set.opt.keybag);
			obj.setCertificateChain(user_set.getCertificateChain());
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(set.opt.verify);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_HB_ASSET)
		{
			AssetProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(set.opt.verify);

			if (user_set.getAssetIconPath().isSet())
				obj.setIconExtractPath(user_set.getAssetIconPath().get());
			if (user_set.getAssetNacpPath().isSet())
				obj.setNacpExtractPath(user_set.getAssetNacpPath().get());

			if (user_set.getFsPath().isSet())
				obj.setRomfsExtractPath(user_set.getFsPath().get());
			obj.setListFs(user_set.isListFs());

			obj.process();
		}

		switch (set.infile.filetype)
		{
			case nstool::Settings::FILE_TYPE_GAMECARD :
				fmt::print("## FILE_TYPE_GAMECARD ##\n");
				break;
			case nstool::Settings::FILE_TYPE_NSP :
				fmt::print("## FILE_TYPE_NSP ##\n");
				break;
			case nstool::Settings::FILE_TYPE_PARTITIONFS :
				fmt::print("## FILE_TYPE_PARTITIONFS ##\n");
				break;
			case nstool::Settings::FILE_TYPE_ROMFS :
				fmt::print("## FILE_TYPE_ROMFS ##\n");
				break;
			case nstool::Settings::FILE_TYPE_NCA :
				fmt::print("## FILE_TYPE_NCA ##\n");
				break;
			case nstool::Settings::FILE_TYPE_META :
				fmt::print("## FILE_TYPE_META ##\n");
				break;
			case nstool::Settings::FILE_TYPE_CNMT :
				fmt::print("## FILE_TYPE_CNMT ##\n");
				break;
			case nstool::Settings::FILE_TYPE_NSO :
				fmt::print("## FILE_TYPE_NSO ##\n");
				break;
			case nstool::Settings::FILE_TYPE_NRO :
				fmt::print("## FILE_TYPE_NRO ##\n");
				break;
			case nstool::Settings::FILE_TYPE_NACP :
				fmt::print("## FILE_TYPE_NACP ##\n");
				break;
			case nstool::Settings::FILE_TYPE_INI :
				fmt::print("## FILE_TYPE_INI ##\n");
				break;
			case nstool::Settings::FILE_TYPE_KIP :
				fmt::print("## FILE_TYPE_KIP ##\n");
				break;
			case nstool::Settings::FILE_TYPE_PKI_CERT :
				fmt::print("## FILE_TYPE_PKI_CERT ##\n");
				break;
			case nstool::Settings::FILE_TYPE_ES_TIK :
				fmt::print("## FILE_TYPE_ES_TIK ##\n");
				break;
			case nstool::Settings::FILE_TYPE_HB_ASSET :
				fmt::print("## FILE_TYPE_HB_ASSET ##\n");
				break;
			default:
				fmt::print("## unknown({}) ##\n", (int)set.infile.filetype);
				break;
		}
		
	}
	catch (tc::Exception& e)
	{
		fmt::print("[{0}{1}ERROR] {2}\n", e.module(), (strlen(e.module()) != 0 ? " ": ""), e.error());
		return 1;
	}
	return 0;
}