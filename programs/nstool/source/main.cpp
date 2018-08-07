#include <cstdio>
#include <fnd/SimpleFile.h>
#include <fnd/StringConv.h>
#include "UserSettings.h"
#include "XciProcess.h"
#include "PfsProcess.h"
#include "RomfsProcess.h"
#include "NcaProcess.h"
#include "NpdmProcess.h"
#include "CnmtProcess.h"
#include "NsoProcess.h"
#include "NroProcess.h"
#include "NacpProcess.h"
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

		if (user_set.getFileType() == FILE_XCI)
		{	
			XciProcess xci;

			xci.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
			
			xci.setKeyset(&user_set.getKeyset());
			xci.setCliOutputMode(user_set.getCliOutputMode());
			xci.setVerifyMode(user_set.isVerifyFile());

			if (user_set.getXciUpdatePath().isSet)
				xci.setPartitionForExtract(nn::hac::xci::kUpdatePartitionStr, user_set.getXciUpdatePath().var);
			if (user_set.getXciLogoPath().isSet)
				xci.setPartitionForExtract(nn::hac::xci::kLogoPartitionStr, user_set.getXciLogoPath().var);
			if (user_set.getXciNormalPath().isSet)
				xci.setPartitionForExtract(nn::hac::xci::kNormalPartitionStr, user_set.getXciNormalPath().var);
			if (user_set.getXciSecurePath().isSet)
				xci.setPartitionForExtract(nn::hac::xci::kSecurePartitionStr, user_set.getXciSecurePath().var);
			xci.setListFs(user_set.isListFs());

			xci.process();
		}
		else if (user_set.getFileType() == FILE_PARTITIONFS || user_set.getFileType() == FILE_NSP)
		{
			PfsProcess pfs;

			pfs.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
			pfs.setCliOutputMode(user_set.getCliOutputMode());
			pfs.setVerifyMode(user_set.isVerifyFile());

			if (user_set.getFsPath().isSet)
				pfs.setExtractPath(user_set.getFsPath().var);
			pfs.setListFs(user_set.isListFs());
			
			pfs.process();
		}
		else if (user_set.getFileType() == FILE_ROMFS)
		{
			RomfsProcess romfs;

			romfs.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
			romfs.setCliOutputMode(user_set.getCliOutputMode());
			romfs.setVerifyMode(user_set.isVerifyFile());

			if (user_set.getFsPath().isSet)
				romfs.setExtractPath(user_set.getFsPath().var);
			romfs.setListFs(user_set.isListFs());

			romfs.process();
		}
		else if (user_set.getFileType() == FILE_NCA)
		{
			NcaProcess nca;

			nca.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
			nca.setKeyset(&user_set.getKeyset());
			nca.setCliOutputMode(user_set.getCliOutputMode());
			nca.setVerifyMode(user_set.isVerifyFile());


			if (user_set.getNcaPart0Path().isSet)
				nca.setPartition0ExtractPath(user_set.getNcaPart0Path().var);
			if (user_set.getNcaPart1Path().isSet)
				nca.setPartition1ExtractPath(user_set.getNcaPart1Path().var);
			if (user_set.getNcaPart2Path().isSet)
				nca.setPartition2ExtractPath(user_set.getNcaPart2Path().var);
			if (user_set.getNcaPart3Path().isSet)
				nca.setPartition3ExtractPath(user_set.getNcaPart3Path().var);
			nca.setListFs(user_set.isListFs());

			nca.process();
		}
		else if (user_set.getFileType() == FILE_NPDM)
		{
			NpdmProcess npdm;

			npdm.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
			npdm.setKeyset(&user_set.getKeyset());
			npdm.setCliOutputMode(user_set.getCliOutputMode());
			npdm.setVerifyMode(user_set.isVerifyFile());

			npdm.process();
		}
		else if (user_set.getFileType() == FILE_CNMT)
		{
			CnmtProcess cnmt;

			cnmt.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
			cnmt.setCliOutputMode(user_set.getCliOutputMode());
			cnmt.setVerifyMode(user_set.isVerifyFile());

			cnmt.process();
		}
		else if (user_set.getFileType() == FILE_NSO)
		{
			NsoProcess obj;

			obj.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
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

			obj.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
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
			NacpProcess nacp;

			nacp.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
			nacp.setCliOutputMode(user_set.getCliOutputMode());
			nacp.setVerifyMode(user_set.isVerifyFile());

			nacp.process();
		}
		else if (user_set.getFileType() == FILE_PKI_CERT)
		{
			PkiCertProcess cert;

			cert.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
			cert.setKeyset(&user_set.getKeyset());
			cert.setCliOutputMode(user_set.getCliOutputMode());
			cert.setVerifyMode(user_set.isVerifyFile());

			cert.process();
		}
		else if (user_set.getFileType() == FILE_ES_TIK)
		{
			EsTikProcess tik;

			tik.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
			tik.setKeyset(&user_set.getKeyset());
			tik.setCertificateChain(user_set.getCertificateChain());
			tik.setCliOutputMode(user_set.getCliOutputMode());
			tik.setVerifyMode(user_set.isVerifyFile());

			tik.process();
		}
		else if (user_set.getFileType() == FILE_HB_ASSET)
		{
			AssetProcess obj;

			obj.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
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
	}
	catch (const fnd::Exception& e) {
		printf("\n\n%s\n", e.what());
	}
	return 0;
}