#include <fnd/SimpleFile.h>

using namespace fnd;

SimpleFile::SimpleFile() :
	mOpen(false),
	mMode(Read),
	mFp(nullptr)
{
}

SimpleFile::~SimpleFile()
{
	close();
}

void SimpleFile::open(const std::string& path, OpenMode mode)
{
	//close();
	mMode = mode;
	//printf("fopen(%s,%s);\n", path.c_str(), getOpenModeStr(mMode));
	mFp = fopen(path.c_str(), getOpenModeStr(mMode));
	if (mFp == nullptr)
		throw fnd::Exception(kModuleName, "Failed to open file.");
	mOpen = true;
	seek(0);
}

bool SimpleFile::isOpen() const
{
	return mOpen == true && mFp != nullptr;
}

void SimpleFile::close()
{
	if (isOpen())
	{
		fclose(mFp);
	}
	mFp = nullptr;
	mOpen = false;
}

size_t SimpleFile::size()
{
	size_t cur_pos = pos();
	fseek(mFp, 0, SEEK_END);
	size_t fsize = pos();
	seek(cur_pos);
	return fsize;
}

void SimpleFile::seek(size_t offset)
{
	fseek(mFp, offset, SEEK_SET);
}

size_t SimpleFile::pos()
{
	return ftell(mFp);
}

void SimpleFile::read(byte_t* out, size_t offset, size_t len)
{
	seek(offset);
	fread(out, len, 1, mFp);
}

void SimpleFile::write(const byte_t* out, size_t offset, size_t len)
{
	seek(offset);
	fwrite(out, len, 1, mFp);
}

const char* SimpleFile::getOpenModeStr(OpenMode mode)
{
	const char* str = "";
	switch(mode)
	{
		case (Read):
			str = "rb";
			break;
		case (Edit):
			str = "rb+";
			break;
		case (Create):
			str = "wb";
			break;
		default:
			throw fnd::Exception(kModuleName, "Unknown open mode");
	}
	return str;
}