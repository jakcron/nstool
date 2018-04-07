#include <cstdio>
#include <crypto/aes.h>
#include <fnd/io.h>
#include <fnd/MemoryBlob.h>
#include <nx/NXCrypto.h>
#include <nx/PfsHeader.h>
#include <inttypes.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif


std::string kFsTypeStr[]
{
	"PFS0",
	"HFS0"
};

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("usage: pfstool <file> [<output dir>]\n");
		return 1;
	}

	try
	{
		fnd::MemoryBlob file;
		fnd::io::readFile(argv[1], file);

		// import
		nx::PfsHeader pfs;
		pfs.importBinary(file.getBytes(), file.getSize());

		if (argc == 3)
		{
#ifdef _WIN32
			_mkdir(argv[2]);
#else
			mkdir(argv[2], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
		}

		printf("[PartitionFS]\n");
		printf("  Type:        %s\n", kFsTypeStr[pfs.getFsType()].c_str());
		printf("  FileSystem:  (%d files)\n", pfs.getFileList().getSize());
		for (size_t i = 0; i < pfs.getFileList().getSize(); i++)
		{

			printf("    %s", pfs.getFileList()[i].name.c_str());
			if (pfs.getFsType() == pfs.TYPE_PFS0)
				printf(" (offset=0x%" PRIx64 ", size=0x%" PRIx64 ")\n", pfs.getFileList()[i].offset, pfs.getFileList()[i].size);
			else
				printf(" (offset=0x%" PRIx64 ", size=0x%" PRIx64 ", hash_protected_size=0x%" PRIx64 ")\n", pfs.getFileList()[i].offset, pfs.getFileList()[i].size, pfs.getFileList()[i].hash_protected_size);
			
			if (argc == 3)
			{
#ifdef _WIN32
				fnd::io::writeFile(std::string(argv[2]) + "\\" + pfs.getFileList()[i].name, file.getBytes() + pfs.getFileList()[i].offset, pfs.getFileList()[i].size);
#else
				fnd::io::writeFile(std::string(argv[2]) + "/" + pfs.getFileList()[i].name, file.getBytes() + pfs.getFileList()[i].offset, pfs.getFileList()[i].size);
#endif
			}
			
		}


	} catch (const fnd::Exception& e)
	{
		printf("%s\n", e.what());
	}

	return 0;
}