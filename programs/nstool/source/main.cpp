#include <cstdio>
#include <fnd/SimpleFile.h>
#include "UserSettings.h"
#include "XciProcess.h"
#include "PfsProcess.h"
#include "RomfsProcess.h"
#include "NcaProcess.h"
#include "NpdmProcess.h"
#include "CnmtProcess.h"
#include "CodeObjectProcess.h"

int main(int argc, char** argv)
{
	UserSettings user_set;
	try {
		user_set.parseCmdArgs(argc, argv);

		if (user_set.getFileType() == FILE_XCI)
		{	
			XciProcess xci;

			xci.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
			
			xci.setKeyset(&user_set.getKeyset());
			xci.setCliOutputMode(user_set.getCliOutputType());
			xci.setVerifyMode(user_set.isVerifyFile());

			if (user_set.getXciUpdatePath().isSet)
				xci.setPartitionForExtract(nx::xci::kUpdatePartitionStr, user_set.getXciUpdatePath().var);
			if (user_set.getXciNormalPath().isSet)
				xci.setPartitionForExtract(nx::xci::kNormalPartitionStr, user_set.getXciNormalPath().var);
			if (user_set.getXciSecurePath().isSet)
				xci.setPartitionForExtract(nx::xci::kSecurePartitionStr, user_set.getXciSecurePath().var);
			if (user_set.getXciLogoPath().isSet)
				xci.setPartitionForExtract(nx::xci::kLogoPartitionStr, user_set.getXciLogoPath().var);
			xci.setListFs(user_set.isListFs());

			xci.process();
		}
		else if (user_set.getFileType() == FILE_PARTITIONFS || user_set.getFileType() == FILE_NSP)
		{
			PfsProcess pfs;

			pfs.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
			pfs.setCliOutputMode(user_set.getCliOutputType());
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
			romfs.setCliOutputMode(user_set.getCliOutputType());
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
			nca.setCliOutputMode(user_set.getCliOutputType());
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
			npdm.setCliOutputMode(user_set.getCliOutputType());
			npdm.setVerifyMode(user_set.isVerifyFile());

			npdm.process();
		}
		else if (user_set.getFileType() == FILE_CNMT)
		{
			CnmtProcess cnmt;

			cnmt.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
			cnmt.setCliOutputMode(user_set.getCliOutputType());
			cnmt.setVerifyMode(user_set.isVerifyFile());

			cnmt.process();
		}
		else if (user_set.getFileType() == FILE_NSO || user_set.getFileType() == FILE_NRO)
		{
			CodeObjectProcess obj;

			obj.setInputFile(new fnd::SimpleFile(user_set.getInputPath(), fnd::SimpleFile::Read), OWN_IFILE);
			obj.setCliOutputMode(user_set.getCliOutputType());
			obj.setVerifyMode(user_set.isVerifyFile());
			
			obj.setCodeObjectType(user_set.getFileType() == FILE_NSO ? obj.OBJ_NSO : obj.OBJ_NRO);
			obj.setInstructionType(user_set.getInstType());
			obj.setListApi(user_set.isListApi());
			obj.setListSymbols(user_set.isListSymbols());

			obj.process();
		}
	}
	catch (const fnd::Exception& e) {
		printf("\n\n%s\n", e.what());
	}
	return 0;
}