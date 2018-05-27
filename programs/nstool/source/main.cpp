#include <cstdio>
#include <fnd/SimpleFile.h>
#include "UserSettings.h"
#include "XciProcess.h"
#include "PfsProcess.h"
#include "RomfsProcess.h"
#include "NcaProcess.h"
#include "NpdmProcess.h"
#include "CnmtProcess.h"
#include "NsoProcess.h"


int main(int argc, char** argv)
{
	UserSettings user_set;
	try {
		user_set.parseCmdArgs(argc, argv);

		fnd::SimpleFile inputFile;
		inputFile.open(user_set.getInputPath(), inputFile.Read);

		if (user_set.getFileType() == FILE_XCI)
		{	
			XciProcess xci;

			xci.setInputFile(&inputFile, 0, inputFile.size());
			
			xci.setKeyset(&user_set.getKeyset());
			xci.setCliOutputMode(user_set.getCliOutputType());
			xci.setVerifyMode(user_set.isVerifyFile());

			if (user_set.getUpdatePath().isSet)
				xci.setUpdateExtractPath(user_set.getUpdatePath().var);
			if (user_set.getNormalPath().isSet)
				xci.setNormalExtractPath(user_set.getNormalPath().var);
			if (user_set.getSecurePath().isSet)
				xci.setSecureExtractPath(user_set.getSecurePath().var);
			xci.setListFs(user_set.isListFs());

			xci.process();
		}
		else if (user_set.getFileType() == FILE_PARTITIONFS || user_set.getFileType() == FILE_NSP)
		{
			PfsProcess pfs;

			pfs.setInputFile(&inputFile, 0, inputFile.size());
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

			romfs.setInputFile(&inputFile, 0, inputFile.size());
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

			nca.setInputFile(&inputFile, 0, inputFile.size());
			nca.setKeyset(&user_set.getKeyset());
			nca.setCliOutputMode(user_set.getCliOutputType());
			nca.setVerifyMode(user_set.isVerifyFile());


			if (user_set.getPart0Path().isSet)
				nca.setPartition0ExtractPath(user_set.getPart0Path().var);
			if (user_set.getPart1Path().isSet)
				nca.setPartition1ExtractPath(user_set.getPart1Path().var);
			if (user_set.getPart2Path().isSet)
				nca.setPartition2ExtractPath(user_set.getPart2Path().var);
			if (user_set.getPart3Path().isSet)
				nca.setPartition3ExtractPath(user_set.getPart3Path().var);
			nca.setListFs(user_set.isListFs());

			nca.process();
		}
		else if (user_set.getFileType() == FILE_NPDM)
		{
			NpdmProcess npdm;

			npdm.setInputFile(&inputFile, 0, inputFile.size());
			npdm.setKeyset(&user_set.getKeyset());
			npdm.setCliOutputMode(user_set.getCliOutputType());
			npdm.setVerifyMode(user_set.isVerifyFile());

			npdm.process();
		}
		else if (user_set.getFileType() == FILE_CNMT)
		{
			CnmtProcess cnmt;

			cnmt.setInputFile(&inputFile, 0, inputFile.size());
			cnmt.setCliOutputMode(user_set.getCliOutputType());
			cnmt.setVerifyMode(user_set.isVerifyFile());

			cnmt.process();
		}
		else if (user_set.getFileType() == FILE_NSO)
		{
			NsoProcess nso;

			nso.setInputFile(&inputFile, 0, inputFile.size());
			nso.setCliOutputMode(user_set.getCliOutputType());
			nso.setVerifyMode(user_set.isVerifyFile());

			nso.process();
		}
	}
	catch (const fnd::Exception& e) {
		printf("\n\n%s\n", e.what());
	}
	return 0;
}