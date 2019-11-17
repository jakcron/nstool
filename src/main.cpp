#include <cstdio>
#include <fnd/SimpleFile.h>
#include <fnd/SharedPtr.h>
#include <fnd/StringConv.h>
#include "UserSettings.h"
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

#ifdef _WIN32
int wmain(int argc, wchar_t** argv)
#else
int main(int argc, char** argv)
#endif
{
	std::vector<std::string> args;
	for (size_t i = 0; i < (size_t)argc; i++)
	{
#ifdef _WIN32
		args.push_back(fnd::StringConv::ConvertChar16ToChar8(std::u16string((char16_t*)argv[i])));
#else
		args.push_back(argv[i]);
#endif
	}

	UserSettings user_set;
	try {
		user_set.parseCmdArgs(args);

		fnd::SharedPtr<fnd::IFile> inputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read));

		if (user_set.getFileType() == FILE_GAMECARD)
		{	
			GameCardProcess obj;

			obj.setInputFile(inputFile);
			
			obj.setKeyCfg(user_set.getKeyCfg());
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(user_set.isVerifyFile());

			if (user_set.getXciUpdatePath().isSet)
				obj.setPartitionForExtract(nn::hac::gc::kUpdatePartitionStr, user_set.getXciUpdatePath().var);
			if (user_set.getXciLogoPath().isSet)
				obj.setPartitionForExtract(nn::hac::gc::kLogoPartitionStr, user_set.getXciLogoPath().var);
			if (user_set.getXciNormalPath().isSet)
				obj.setPartitionForExtract(nn::hac::gc::kNormalPartitionStr, user_set.getXciNormalPath().var);
			if (user_set.getXciSecurePath().isSet)
				obj.setPartitionForExtract(nn::hac::gc::kSecurePartitionStr, user_set.getXciSecurePath().var);
			obj.setListFs(user_set.isListFs());

			obj.process();
		}
		else if (user_set.getFileType() == FILE_PARTITIONFS || user_set.getFileType() == FILE_NSP)
		{
			PfsProcess obj;

			obj.setInputFile(inputFile);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(user_set.isVerifyFile());

			if (user_set.getFsPath().isSet)
				obj.setExtractPath(user_set.getFsPath().var);
			obj.setListFs(user_set.isListFs());
			
			obj.process();
		}
		else if (user_set.getFileType() == FILE_ROMFS)
		{
			RomfsProcess obj;

			obj.setInputFile(inputFile);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(user_set.isVerifyFile());

			if (user_set.getFsPath().isSet)
				obj.setExtractPath(user_set.getFsPath().var);
			obj.setListFs(user_set.isListFs());

			obj.process();
		}
		else if (user_set.getFileType() == FILE_NCA)
		{
			NcaProcess obj;

			obj.setInputFile(inputFile);
			obj.setKeyCfg(user_set.getKeyCfg());
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(user_set.isVerifyFile());


			if (user_set.getNcaPart0Path().isSet)
				obj.setPartition0ExtractPath(user_set.getNcaPart0Path().var);
			if (user_set.getNcaPart1Path().isSet)
				obj.setPartition1ExtractPath(user_set.getNcaPart1Path().var);
			if (user_set.getNcaPart2Path().isSet)
				obj.setPartition2ExtractPath(user_set.getNcaPart2Path().var);
			if (user_set.getNcaPart3Path().isSet)
				obj.setPartition3ExtractPath(user_set.getNcaPart3Path().var);
			obj.setListFs(user_set.isListFs());

			obj.process();
		}
		else if (user_set.getFileType() == FILE_META)
		{
			MetaProcess obj;

			obj.setInputFile(inputFile);
			obj.setKeyCfg(user_set.getKeyCfg());
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(user_set.isVerifyFile());

			obj.process();
		}
		else if (user_set.getFileType() == FILE_CNMT)
		{
			CnmtProcess obj;

			obj.setInputFile(inputFile);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(user_set.isVerifyFile());

			obj.process();
		}
		else if (user_set.getFileType() == FILE_NSO)
		{
			NsoProcess obj;

			obj.setInputFile(inputFile);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(user_set.isVerifyFile());
			
			obj.setInstructionType(user_set.getInstType());
			obj.setListApi(user_set.isListApi());
			obj.setListSymbols(user_set.isListSymbols());

			obj.process();
		}
		else if (user_set.getFileType() == FILE_NRO)
		{
			NroProcess obj;

			obj.setInputFile(inputFile);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(user_set.isVerifyFile());
			
			obj.setInstructionType(user_set.getInstType());
			obj.setListApi(user_set.isListApi());
			obj.setListSymbols(user_set.isListSymbols());

			if (user_set.getAssetIconPath().isSet)
				obj.setAssetIconExtractPath(user_set.getAssetIconPath().var);
			if (user_set.getAssetNacpPath().isSet)
				obj.setAssetNacpExtractPath(user_set.getAssetNacpPath().var);

			if (user_set.getFsPath().isSet)
				obj.setAssetRomfsExtractPath(user_set.getFsPath().var);
			obj.setAssetListFs(user_set.isListFs());

			obj.process();
		}
		else if (user_set.getFileType() == FILE_NACP)
		{
			NacpProcess obj;

			obj.setInputFile(inputFile);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(user_set.isVerifyFile());

			obj.process();
		}
		else if (user_set.getFileType() == FILE_INI)
		{
			IniProcess obj;

			obj.setInputFile(inputFile);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(user_set.isVerifyFile());

			if (user_set.getKipExtractPath().isSet)
				obj.setKipExtractPath(user_set.getKipExtractPath().var);

			obj.process();
		}
		else if (user_set.getFileType() == FILE_KIP)
		{
			KipProcess obj;

			obj.setInputFile(inputFile);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(user_set.isVerifyFile());

			obj.process();
		}
		else if (user_set.getFileType() == FILE_PKI_CERT)
		{
			PkiCertProcess obj;

			obj.setInputFile(inputFile);
			obj.setKeyCfg(user_set.getKeyCfg());
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(user_set.isVerifyFile());

			obj.process();
		}
		else if (user_set.getFileType() == FILE_ES_TIK)
		{
			EsTikProcess obj;

			obj.setInputFile(inputFile);
			obj.setKeyCfg(user_set.getKeyCfg());
			obj.setCertificateChain(user_set.getCertificateChain());
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(user_set.isVerifyFile());

			obj.process();
		}
		else if (user_set.getFileType() == FILE_HB_ASSET)
		{
			AssetProcess obj;

			obj.setInputFile(inputFile);
			obj.setCliOutputMode(user_set.getCliOutputMode());
			obj.setVerifyMode(user_set.isVerifyFile());

			if (user_set.getAssetIconPath().isSet)
				obj.setIconExtractPath(user_set.getAssetIconPath().var);
			if (user_set.getAssetNacpPath().isSet)
				obj.setNacpExtractPath(user_set.getAssetNacpPath().var);

			if (user_set.getFsPath().isSet)
				obj.setRomfsExtractPath(user_set.getFsPath().var);
			obj.setListFs(user_set.isListFs());

			obj.process();
		}
		else
		{
			throw fnd::Exception("main", "Unhandled file type");
		}
	}
	catch (const fnd::Exception& e) {
		printf("\n\n%s\n", e.what());
	}
	return 0;
}