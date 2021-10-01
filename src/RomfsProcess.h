#pragma once
#include "types.h"

#include <nn/hac/define/romfs.h>

namespace nstool {

class RomfsProcess
{
public:
	struct sDirectory;
	struct sFile;

	struct sDirectory
	{
		std::string name;
		std::vector<sDirectory> dir_list;
		std::vector<sFile> file_list;

		void operator=(const sDirectory& other)
		{
			name = other.name;
			dir_list = other.dir_list;
			file_list = other.file_list;
		}

		bool operator==(const sDirectory& other) const
		{
			return (name == other.name) \
				&& (dir_list == other.dir_list) \
				&& (file_list == other.file_list);
		}

		bool operator!=(const sDirectory& other) const
		{
			return !operator==(other);
		}

		bool operator==(const std::string& other) const
		{
			return (name == other);
		}
	};

	struct sFile
	{
		std::string name;
		int64_t offset;
		int64_t size;

		void operator=(const sFile& other)
		{
			name = other.name;
			offset = other.offset;
			size = other.size;
		}

		bool operator==(const sFile& other) const
		{
			return (name == other.name) \
				&& (offset == other.offset) \
				&& (size == other.size);
		}

		bool operator!=(const sFile& other) const
		{
			return !operator==(other);
		}

		bool operator==(const std::string& other) const
		{
			return (name == other);
		}
	};

	RomfsProcess();

	void process();

	// generic
	void setInputFile(const std::shared_ptr<tc::io::IStream>& file);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	// romfs specific
	void setMountPointName(const std::string& mount_name);
	void setExtractPath(const tc::io::Path& path);
	void setListFs(bool list_fs);

	const sDirectory& getRootDir() const;
private:
	const std::string kModuleName = "RomfsProcess";
	static const size_t kCacheSize = 0x10000;

	std::shared_ptr<tc::io::IStream> mFile;
	CliOutputMode mCliOutputMode;
	bool mShowBasicInfo;
	bool mShowExtendedInfo;
	bool mShowLayoutInfo;
	bool mShowKeydata;
	bool mVerbose;
	bool mVerify;

	tc::Optional<tc::io::Path> mExtractPath;
	std::string mMountName;
	bool mListFs;

	tc::ByteData mCache;

	size_t mDirNum;
	size_t mFileNum;
	nn::hac::sRomfsHeader mHdr;
	tc::ByteData mDirNodes;
	tc::ByteData mFileNodes;
	sDirectory mRootDir;

	inline nn::hac::sRomfsDirEntry* get_dir_node(uint32_t offset) { return (nn::hac::sRomfsDirEntry*)(mDirNodes.data() + offset); }
	inline nn::hac::sRomfsFileEntry* get_file_node(uint32_t offset) { return (nn::hac::sRomfsFileEntry*)(mFileNodes.data() + offset); }

	
	void printTab(size_t tab) const;
	void displayFile(const sFile& file, size_t tab) const;
	void displayDir(const sDirectory& dir, size_t tab) const;

	void displayHeader();
	void displayFs();

	void extractDir(const std::string& path, const sDirectory& dir);
	void extractFs();

	bool validateHeaderLayout(const nn::hac::sRomfsHeader* hdr) const;
	void importDirectory(uint32_t dir_offset, sDirectory& dir);
	void resolveRomfs();
};

}