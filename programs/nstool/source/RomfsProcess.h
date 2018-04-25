#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <nx/romfs.h>

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

		sDirectory& operator=(const sDirectory& other)
		{
			name = other.name;
			dir_list = other.dir_list;
			file_list = other.file_list;
			return *this;
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

		bool operator!=(const std::string& other) const
		{
			return !operator==(other);
		}
	};

	struct sFile
	{
		std::string name;
		uint64_t offset;
		uint64_t size;

		sFile& operator=(const sFile& other)
		{
			name = other.name;
			offset = other.offset;
			size = other.size;
			return *this;
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

		bool operator!=(const std::string& other) const
		{
			return !operator==(other);
		}
	};

	RomfsProcess();

	void process();

	// generic
	void setInputFile(fnd::IFile& reader);
	void setInputFileOffset(size_t offset);
	void setKeyset(const sKeyset* keyset);
	void setCliOutputMode(CliOutputType type);
	void setVerifyMode(bool verify);

	// romfs specific
	void setMountPointName(const std::string& mount_name);
	void setExtractPath(const std::string& path);
	void setListFs(bool list_fs);

	const sDirectory& getRootDir() const;
private:
	const std::string kModuleName = "RomfsProcess";
	static const size_t kFileExportBlockSize = 0x1000000;

	fnd::IFile* mReader;
	size_t mOffset;
	const sKeyset* mKeyset;
	CliOutputType mCliOutputType;
	bool mVerify;

	std::string mExtractPath;
	bool mExtract;
	std::string mMountName;
	bool mListFs;

	size_t mDirNum;
	size_t mFileNum;
	nx::sRomfsHeader mHdr;
	fnd::MemoryBlob mDirNodes;
	fnd::MemoryBlob mFileNodes;
	sDirectory mRootDir;

	inline nx::sRomfsDirEntry* get_dir_node(uint32_t offset) { return (nx::sRomfsDirEntry*)(mDirNodes.getBytes() + offset); }
	inline nx::sRomfsFileEntry* get_file_node(uint32_t offset) { return (nx::sRomfsFileEntry*)(mFileNodes.getBytes() + offset); }

	
	void printTab(size_t tab) const;
	void displayFile(const sFile& file, size_t tab) const;
	void displayDir(const sDirectory& dir, size_t tab) const;

	void displayHeader();
	void displayFs();

	void extractFile(const std::string& path, const sFile& file);
	void extractDir(const std::string& path, const sDirectory& dir);
	void extractFs();

	bool validateHeaderLayout(const nx::sRomfsHeader* hdr) const;
	void importDirectory(uint32_t dir_offset, sDirectory& dir);
	void resolveRomfs();
};