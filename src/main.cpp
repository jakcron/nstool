#include <tc.h>
#include <tc/os/UnicodeMain.h>
#include "Settings.h"


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
#include "EsCertProcess.h"
#include "EsTikProcess.h"
#include "AssetProcess.h"


int umain(const std::vector<std::string>& args, const std::vector<std::string>& env)
{
	try 
	{
		nstool::Settings set = nstool::SettingsInitializer(args);
		
		std::shared_ptr<tc::io::IStream> infile_stream = std::make_shared<tc::io::FileStream>(tc::io::FileStream(set.infile.path.get(), tc::io::FileMode::Open, tc::io::FileAccess::Read));

		if (set.infile.filetype == nstool::Settings::FILE_TYPE_GAMECARD)
		{	
			nstool::GameCardProcess obj;

			obj.setInputFile(infile_stream);
			
			obj.setKeyCfg(set.opt.keybag);
			obj.setCliOutputMode(set.opt.cli_output_mode);
			obj.setVerifyMode(set.opt.verify);

			obj.setShowFsTree(set.fs.show_fs_tree);
			obj.setExtractJobs(set.fs.extract_jobs);
		
			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_PARTITIONFS || set.infile.filetype == nstool::Settings::FILE_TYPE_NSP)
		{
			nstool::PfsProcess obj;

			obj.setInputFile(infile_stream);

			obj.setCliOutputMode(set.opt.cli_output_mode);
			obj.setVerifyMode(set.opt.verify);

			obj.setShowFsTree(set.fs.show_fs_tree);
			obj.setExtractJobs(set.fs.extract_jobs);
			
			obj.process();
		}
		
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_ROMFS)
		{
			nstool::RomfsProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(set.opt.cli_output_mode);
			obj.setVerifyMode(set.opt.verify);

			obj.setShowFsTree(set.fs.show_fs_tree);
			obj.setExtractJobs(set.fs.extract_jobs);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_NCA)
		{
			nstool::NcaProcess obj;

			obj.setInputFile(infile_stream);
			obj.setBaseNcaPath(set.nca.base_nca_path);
			obj.setKeyCfg(set.opt.keybag);
			obj.setCliOutputMode(set.opt.cli_output_mode);
			obj.setVerifyMode(set.opt.verify);

			obj.setShowFsTree(set.fs.show_fs_tree);
			obj.setExtractJobs(set.fs.extract_jobs);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_META)
		{
			nstool::MetaProcess obj;

			obj.setInputFile(infile_stream);
			obj.setKeyCfg(set.opt.keybag);
			obj.setCliOutputMode(set.opt.cli_output_mode);
			obj.setVerifyMode(set.opt.verify);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_CNMT)
		{
			nstool::CnmtProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(set.opt.cli_output_mode);
			obj.setVerifyMode(set.opt.verify);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_NSO)
		{
			nstool::NsoProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(set.opt.cli_output_mode);
			obj.setVerifyMode(set.opt.verify);
			
			obj.setIs64BitInstruction(set.code.is_64bit_instruction);
			obj.setListApi(set.code.list_api);
			obj.setListSymbols(set.code.list_symbols);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_NRO)
		{
			nstool::NroProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(set.opt.cli_output_mode);
			obj.setVerifyMode(set.opt.verify);
			
			obj.setIs64BitInstruction(set.code.is_64bit_instruction);
			obj.setListApi(set.code.list_api);
			obj.setListSymbols(set.code.list_symbols);

			if (set.aset.icon_extract_path.isSet())
				obj.setAssetIconExtractPath(set.aset.icon_extract_path.get());
			if (set.aset.nacp_extract_path.isSet())
				obj.setAssetNacpExtractPath(set.aset.nacp_extract_path.get());

			obj.setAssetRomfsShowFsTree(set.fs.show_fs_tree);
			obj.setAssetRomfsExtractJobs(set.fs.extract_jobs);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_NACP)
		{
			nstool::NacpProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(set.opt.cli_output_mode);
			obj.setVerifyMode(set.opt.verify);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_INI)
		{
			nstool::IniProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(set.opt.cli_output_mode);
			obj.setVerifyMode(set.opt.verify);

			if (set.kip.extract_path.isSet())
				obj.setKipExtractPath(set.kip.extract_path.get());

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_KIP)
		{
			nstool::KipProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(set.opt.cli_output_mode);
			obj.setVerifyMode(set.opt.verify);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_ES_CERT)
		{
			nstool::EsCertProcess obj;

			obj.setInputFile(infile_stream);
			obj.setKeyCfg(set.opt.keybag);
			obj.setCliOutputMode(set.opt.cli_output_mode);
			obj.setVerifyMode(set.opt.verify);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_ES_TIK)
		{
			nstool::EsTikProcess obj;

			obj.setInputFile(infile_stream);
			obj.setKeyCfg(set.opt.keybag);
			//obj.setCertificateChain(user_set.getCertificateChain());
			obj.setCliOutputMode(set.opt.cli_output_mode);
			obj.setVerifyMode(set.opt.verify);

			obj.process();
		}
		else if (set.infile.filetype == nstool::Settings::FILE_TYPE_HB_ASSET)
		{
			nstool::AssetProcess obj;

			obj.setInputFile(infile_stream);
			obj.setCliOutputMode(set.opt.cli_output_mode);
			obj.setVerifyMode(set.opt.verify);

			if (set.aset.icon_extract_path.isSet())
				obj.setIconExtractPath(set.aset.icon_extract_path.get());
			if (set.aset.nacp_extract_path.isSet())
				obj.setNacpExtractPath(set.aset.nacp_extract_path.get());

			obj.setRomfsShowFsTree(set.fs.show_fs_tree);
			obj.setRomfsExtractJobs(set.fs.extract_jobs);

			obj.process();
		}
	}
	catch (tc::Exception& e)
	{
		fmt::print("[{0}{1}ERROR] {2}\n", e.module(), (strlen(e.module()) != 0 ? " ": ""), e.error());
		return 1;
	}
	return 0;
}