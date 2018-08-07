#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/Vec.h>
#include <fnd/List.h>
#include <nn/hac/romfs.h>

#include "nstool.h"

class RomfsProcess
{
public:
	struct sDirectory;
	struct sFile;

	struct sDirectory
	{
		std::string name;
		fnd::List<sDirectory> dir_list;
		fnd::List<sFile> file_list;

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
		uint64_t offset;
		uint64_t size;

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
	~RomfsProcess();

	void process();

	// generic
	void setInputFile(fnd::IFile* file, bool ownIFile);
	void setCliOutputMode(CliOutputMode type);
	void setVerifyMode(bool verify);

	// romfs specific
	void setMountPointName(const std::string& mount_name);
	void setExtractPath(const std::string& path);
	void setListFs(bool list_fs);

	const sDirectory& getRootDir() const;
private:
	const std::string kModuleName = "RomfsProcess";
	static const size_t kCacheSize = 0x10000;

	fnd::IFile* mFile;
	bool mOwnIFile;
	CliOutputMode mCliOutputMode;
	bool mVerify;

	std::string mExtractPath;
	bool mExtract;
	std::string mMountName;
	bool mListFs;

	fnd::Vec<byte_t> mCache;

	size_t mDirNum;
	size_t mFileNum;
	nn::hac::sRomfsHeader mHdr;
	fnd::Vec<byte_t> mDirNodes;
	fnd::Vec<byte_t> mFileNodes;
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