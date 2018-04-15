#include <fnd/io.h>
#include <fstream>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

using namespace fnd;

static const std::string kModuleName = "IO";
static const size_t kBlockSize = 0x100000;

size_t io::getFileSize(const std::string& path)
{
	std::ifstream f;
	f.open(path, std::ios_base::binary | std::ios_base::in);
	if (!f.good() || f.eof() || !f.is_open()) { return 0; }
	f.seekg(0, std::ios_base::beg);
	std::ifstream::pos_type begin_pos = f.tellg();
	f.seekg(0, std::ios_base::end);
	return static_cast<size_t>(f.tellg() - begin_pos);
}

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

void io::makeDirectory(const std::string& path)
{
#ifdef _WIN32
	_mkdir(path.c_str());
#else
	mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}