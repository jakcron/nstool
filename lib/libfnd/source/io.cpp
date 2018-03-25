#include <fnd/io.h>

using namespace fnd;

static const std::string kModuleName = "IO";
static const size_t kBlockSize = 0x100000;

void io::readFile(const std::string& path, MemoryBlob & blob)
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

	try {
		blob.alloc(filesz);
	}
	catch (const fnd::Exception& e)
	{
		fclose(fp);
		throw fnd::Exception(kModuleName, "Failed to allocate memory for file: " + std::string(e.what()));
	}

	for (filepos = 0; filesz > kBlockSize; filesz -= kBlockSize, filepos += kBlockSize)
	{
		fread(blob.getBytes() + filepos, 1, kBlockSize, fp);
	}

	if (filesz)
	{
		fread(blob.getBytes() + filepos, 1, filesz, fp);
	}

	fclose(fp);
}

void fnd::io::readFile(const std::string& path, size_t offset, size_t len, MemoryBlob& blob)
{
	FILE* fp;
	size_t filesz, filepos;

	if ((fp = fopen(path.c_str(), "rb")) == NULL)
	{
		throw Exception(kModuleName, "Failed to open \"" + path + "\": does not exist");
	}

	fseek(fp, 0, SEEK_END);
	filesz = ftell(fp);
	rewind(fp);
	fseek(fp, offset, SEEK_SET);

	if (filesz < len || filesz < offset || filesz < (offset + len))
	{
		throw Exception(kModuleName, "Failed to open \"" + path + "\": file to small");
	}

	try
	{
		blob.alloc(len);
	} catch (const fnd::Exception& e)
	{
		fclose(fp);
		throw fnd::Exception(kModuleName, "Failed to allocate memory for file: " + std::string(e.what()));
	}

	for (filepos = 0; len > kBlockSize; len -= kBlockSize, filepos += kBlockSize)
	{
		fread(blob.getBytes() + filepos, 1, kBlockSize, fp);
	}

	if (len)
	{
		fread(blob.getBytes() + filepos, 1, len, fp);
	}

	fclose(fp);
}

void io::writeFile(const std::string& path, const MemoryBlob & blob)
{
	writeFile(path, blob.getBytes(), blob.getSize());
}

void io::writeFile(const std::string & path, const byte_t * data, size_t len)
{
	FILE* fp;
	size_t filesz, filepos;

	if ((fp = fopen(path.c_str(), "wb")) == NULL)
	{
		throw Exception(kModuleName, "Failed to open \"" + path + "\"");
	}

	filesz = len;


	for (filepos = 0; filesz > kBlockSize; filesz -= kBlockSize, filepos += kBlockSize)
	{
		fwrite(data + filepos, 1, kBlockSize, fp);
	}

	if (filesz)
	{
		fwrite(data + filepos, 1, filesz, fp);
	}

	fclose(fp);
}
