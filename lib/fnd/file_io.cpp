#include "file_io.h"

using namespace fnd;

static const std::string kModuleName = "FILE_IO";
static const size_t kBlockSize = 0x100000;

void FileIO::ReadFile(const std::string& path, MemoryBlob & blob)
{
	FILE* fp;
	size_t filesz, filepos;

	if ((fp = fopen(path.c_str(), "rb")) == NULL)
	{
		throw Exception(kModuleName, "Failed to open \"" + path + "\"");
	}

	fseek(fp, 0, SEEK_END);
	filesz = ftell(fp);
	rewind(fp);

	if (blob.alloc(filesz) != blob.ERR_NONE)
	{
		fclose(fp);
		throw Exception(kModuleName, "Failed to allocate memory for file");
	}

	for (filepos = 0; filesz > kBlockSize; filesz -= kBlockSize, filepos += kBlockSize)
	{
		fread(blob.data() + filepos, 1, kBlockSize, fp);
	}

	if (filesz)
	{
		fread(blob.data() + filepos, 1, filesz, fp);
	}

	fclose(fp);
}

void FileIO::WriteFile(const std::string& path, const MemoryBlob & blob)
{

}
