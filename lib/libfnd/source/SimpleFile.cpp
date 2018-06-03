#include <fnd/SimpleFile.h>
#include <fnd/StringConv.h>

using namespace fnd;

SimpleFile::SimpleFile() :
	mOpen(false),
	mMode(Read),
#ifdef _WIN32
	mFileHandle()
#else
	mFp(nullptr)
#endif
{
}

SimpleFile::SimpleFile(const std::string& path, OpenMode mode) :
	SimpleFile()
{
	open(path, mode);
}

SimpleFile::~SimpleFile()
{
	close();
}

void SimpleFile::open(const std::string& path, OpenMode mode)
{
#ifdef _WIN32
	// convert string to unicode
	std::u16string unicodePath = fnd::StringConv::ConvertChar8ToChar16(path);

	// save mode
	mMode = mode;

	// open file
	mFileHandle = CreateFileW((LPCWSTR)unicodePath.c_str(),
							  getOpenModeFlag(mMode),
							  getShareModeFlag(mMode),
							  0,
							  getCreationModeFlag(mMode),
							  FILE_ATTRIBUTE_NORMAL,
							  NULL);
	// check file handle
	if (mFileHandle == INVALID_HANDLE_VALUE)
	{
		throw fnd::Exception(kModuleName, "Failed to open file.");
	}


#else
	//close();
	mMode = mode;
	//printf("fopen(%s,%s);\n", path.c_str(), getOpenModeStr(mMode));
	mFp = fopen(path.c_str(), getOpenModeStr(mMode));
	if (mFp == nullptr)
		throw fnd::Exception(kModuleName, "Failed to open file.");
	mOpen = true;
#endif

	seek(0);
}

bool SimpleFile::isOpen() const
{
	return mOpen == true;
}

void SimpleFile::close()
{
	if (isOpen())
	{
#ifdef _WIN32
		CloseHandle(mFileHandle);
#else
		fclose(mFp);
		mFp = nullptr;
#endif
	}
	mOpen = false;
}

size_t SimpleFile::size()
{
	size_t fsize = 0;
#ifdef _WIN32
	if (mMode != Create)
	{
		LARGE_INTEGER win_fsize;
		if (GetFileSizeEx(mFileHandle, &win_fsize) == false)
		{
			throw fnd::Exception(kModuleName, "Failed to check filesize");
		}

		fsize = win_fsize.QuadPart;
	}
	else
	{
		fsize = 0;
	}
#else
	size_t cur_pos = pos();
	fseek(mFp, 0, SEEK_END);
	fsize = pos();
	seek(cur_pos);
#endif
	return fsize;
}

void SimpleFile::seek(size_t offset)
{
#ifdef _WIN32
	LARGE_INTEGER win_pos, out;
	win_pos.QuadPart = offset;
	if (SetFilePointerEx(
		mFileHandle,
		win_pos,
		&out,
		FILE_BEGIN
	) == false || out.QuadPart != win_pos.QuadPart)
	{
		throw fnd::Exception(kModuleName, "Failed to change file offset");
	}
#else
	fseek(mFp, offset, SEEK_SET);
#endif
}

size_t SimpleFile::pos()
{
#ifdef _WIN32
	LARGE_INTEGER win_pos, out;
	win_pos.QuadPart = 0;
	if (SetFilePointerEx(
		mFileHandle,
		win_pos,
		&out,
		FILE_CURRENT
	) == false)
	{
		throw fnd::Exception(kModuleName, "Failed to check file offset");
	}

	return out.QuadPart;
#else
	return ftell(mFp);
#endif
}

void SimpleFile::read(byte_t* out, size_t len)
{
#ifdef _WIN32
	LARGE_INTEGER win_len;
	win_len.QuadPart = len;

	static const DWORD kDwordHalf = (MAXDWORD / (DWORD)2) + 1; // 0x80000000
	static const size_t kDwordFull = (size_t)kDwordHalf * (size_t)2; // 0x100000000

	// if the size is greater than a DWORD, read it in parts, 
	for (LONG i = 0; i < win_len.HighPart; i++)
	{
		// since kDwordFull isn't a valid DWORD value, read in two parts
		ReadFile(
			mFileHandle,
			out + i * kDwordFull,
			kDwordHalf,
			NULL,
			NULL
		);
		ReadFile(
			mFileHandle,
			out + i * kDwordFull + kDwordHalf,
			kDwordHalf,
			NULL,
			NULL
		);
	}

	// read remainding low part
	if (win_len.LowPart > 0)
	{
		ReadFile(
			mFileHandle,
			out + win_len.HighPart * kDwordFull,
			win_len.LowPart,
			NULL,
			NULL
		);
	}
#else
	fread(out, len, 1, mFp);
#endif
}

void SimpleFile::read(byte_t* out, size_t offset, size_t len)
{
	seek(offset);
	read(out, len);
}

void SimpleFile::write(const byte_t* out, size_t len)
{
#ifdef _WIN32
	LARGE_INTEGER win_len;
	win_len.QuadPart = len;

	static const DWORD kDwordHalf = ((DWORD)MAXDWORD / (DWORD)2) + 1; // 0x80000000
	static const size_t kDwordFull = (size_t)kDwordHalf * (size_t)2; // 0x100000000

															   // if the size is greater than a DWORD, read it in parts, 
	for (LONG i = 0; i < win_len.HighPart; i++)
	{
		// since kDwordFull isn't a valid DWORD value, read in two parts
		WriteFile(
			mFileHandle,
			out + i * kDwordFull,
			kDwordHalf,
			NULL,
			NULL
		);
		WriteFile(
			mFileHandle,
			out + i * kDwordFull + kDwordHalf,
			kDwordHalf,
			NULL,
			NULL
		);
	}

	// read remainding low part
	if (win_len.LowPart > 0)
	{
		WriteFile(
			mFileHandle,
			out + win_len.HighPart * kDwordFull,
			win_len.LowPart,
			NULL,
			NULL
		);
	}
#else
	fwrite(out, len, 1, mFp);
#endif
}

void SimpleFile::write(const byte_t* out, size_t offset, size_t len)
{
	seek(offset);
	write(out, len);
}

#ifdef _WIN32
DWORD SimpleFile::getOpenModeFlag(OpenMode mode) const
{
	DWORD flag = 0;
	switch (mode)
	{
		case (Read):
			flag = GENERIC_READ;
			break;
		case (Edit):
			flag = GENERIC_READ | GENERIC_WRITE;
			break;
		case (Create):
			flag = GENERIC_WRITE;
			break;
		default:
			throw fnd::Exception(kModuleName, "Unknown open mode");
	}
	return flag;
}
DWORD fnd::SimpleFile::getShareModeFlag(OpenMode mode) const
{
	DWORD flag = 0;
	switch (mode)
	{
		case (Read):
			flag = FILE_SHARE_READ;
			break;
		case (Edit):
			flag = FILE_SHARE_READ;
			break;
		case (Create):
			flag = 0;
			break;
		default:
			throw fnd::Exception(kModuleName, "Unknown open mode");
	}
	return flag;
}
DWORD fnd::SimpleFile::getCreationModeFlag(OpenMode mode) const
{
	DWORD flag = 0;
	switch (mode)
	{
		case (Read):
			flag = OPEN_EXISTING;
			break;
		case (Edit):
			flag = OPEN_EXISTING;
			break;
		case (Create):
			flag = CREATE_ALWAYS;
			break;
		default:
			throw fnd::Exception(kModuleName, "Unknown open mode");
	}
	return flag;
}
#else
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
#endif