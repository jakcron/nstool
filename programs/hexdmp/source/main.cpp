#include <cstdio>
#include <fnd/SimpleFile.h>
#include <fnd/SharedPtr.h>
#include <fnd/StringConv.h>
#include "UserSettings.h"
#include "HexDumpProcess.h"




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

		if (user_set.getFileType() == FILE_RAW)
		{	
			HexDumpProcess hexdmp;

			hexdmp.setInputFile(inputFile);
			hexdmp.setCliOutputMode(user_set.getCliOutputMode());
			hexdmp.setByteGroupingSize(user_set.getByteGroupingSize());
			hexdmp.setReadOffset(user_set.getReadOffset());
			hexdmp.setReadSize(user_set.getReadSize());
			hexdmp.process();
		}
	}
	catch (const fnd::Exception& e) {
		printf("\n\n%s\n", e.what());
	}
	return 0;
}