#include <cstdio>
#include <fnd/SimpleFile.h>
#include "UserSettings.h"
#include "XciProcess.h"
#include "PfsProcess.h"
#include "RomfsProcess.h"
//#include "NcaProcess.h"
#include "NpdmProcess.h"


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

			xci.setInputFile(inputFile);
			
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
		else if (user_set.getFileType() == FILE_PARTITIONFS)
		{
			PfsProcess pfs;

			pfs.setInputFile(inputFile);
			pfs.setKeyset(&user_set.getKeyset());
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

			romfs.setInputFile(inputFile);
			romfs.setKeyset(&user_set.getKeyset());
			romfs.setCliOutputMode(user_set.getCliOutputType());
			romfs.setVerifyMode(user_set.isVerifyFile());

			if (user_set.getFsPath().isSet)
				romfs.setExtractPath(user_set.getFsPath().var);
			romfs.setListFs(user_set.isListFs());

			romfs.process();
			
		}
		else if (user_set.getFileType() == FILE_NCA)
		{
			/*
			NcaProcess nca;

			nca.setNcaPath(user_set.getInputPath());
			nca.setKeyset(user_set.getKeyset());
			nca.setCliOutputMode(user_set.getCliOutputType());
			nca.setVerifyMode(user_set.isVerifyFile());

			nca.process();
			*/
		}
		else if (user_set.getFileType() == FILE_NPDM)
		{
			NpdmProcess npdm;

			npdm.setInputFile(inputFile);
			npdm.setKeyset(&user_set.getKeyset());
			npdm.setCliOutputMode(user_set.getCliOutputType());
			npdm.setVerifyMode(user_set.isVerifyFile());

			npdm.process();
		}
	}
	catch (const fnd::Exception& e) {
		printf("\n\n%s\n", e.what());
	}
	return 0;
}