#include "RomfsProcess.h"
#include "util.h"

#include <tc/io/VirtualFileSystem.h>
#include <pietendo/hac/RomFsSnapshotGenerator.h>


nstool::RomfsProcess::RomfsProcess() :
	mModuleName("nstool::RomfsProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false),
	mDirNum(0),
	mFileNum(0),
	mFileSystem(),
	mFsProcess()
{
	mFsProcess.setFsFormatName("RomFs");
}

void nstool::RomfsProcess::process()
{
	// state checks
	if (mFile == nullptr)
	{
		throw tc::Exception(mModuleName, "No file reader set.");
	}
	if (mFile->canRead() == false || mFile->canSeek() == false)
	{
		throw tc::NotSupportedException(mModuleName, "Input stream requires read/seek permissions.");
	}

	// read base header to determine complete header size
	if (mFile->length() < tc::io::IOUtil::castSizeToInt64(sizeof(pie::hac::sRomfsHeader)))
	{
		throw tc::Exception(mModuleName, "Corrupt RomFs: File too small");
	}

	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read((byte_t*)&mRomfsHeader, sizeof(mRomfsHeader));
	if (mRomfsHeader.header_size.unwrap() != sizeof(pie::hac::sRomfsHeader) ||
	    mRomfsHeader.dir_entry.offset.unwrap() != (mRomfsHeader.dir_hash_bucket.offset.unwrap() + mRomfsHeader.dir_hash_bucket.size.unwrap()) ||
	    mRomfsHeader.data_offset.unwrap() != align<int64_t>(mRomfsHeader.header_size.unwrap(), pie::hac::romfs::kRomfsHeaderAlign))
	{
		throw tc::ArgumentOutOfRangeException(mModuleName, "Corrupt RomFs: RomFsHeader is corrupted.");
	}

	/*
	fmt::print("RomFsHeader:\n");
	fmt::print(" > header_size = 0x{:04x}\n", mRomfsHeader.header_size.unwrap());
	fmt::print(" > dir_hash_bucket\n");
	fmt::print("   > offset =    0x{:04x}\n", mRomfsHeader.dir_hash_bucket.offset.unwrap());
	fmt::print("   > size =      0x{:04x}\n", mRomfsHeader.dir_hash_bucket.size.unwrap());
	fmt::print(" > dir_entry\n");
	fmt::print("   > offset =    0x{:04x}\n", mRomfsHeader.dir_entry.offset.unwrap());
	fmt::print("   > size =      0x{:04x}\n", mRomfsHeader.dir_entry.size.unwrap());
	fmt::print(" > file_hash_bucket\n");
	fmt::print("   > offset =    0x{:04x}\n", mRomfsHeader.file_hash_bucket.offset.unwrap())
	fmt::print("   > size =      0x{:04x}\n", mRomfsHeader.file_hash_bucket.size.unwrap());
	fmt::print(" > file_entry\n");
	fmt::print("   > offset =    0x{:04x}\n", mRomfsHeader.file_entry.offset.unwrap());
	fmt::print("   > size =      0x{:04x}\n", mRomfsHeader.file_entry.size.unwrap());
	fmt::print(" > data_offset = 0x{:04x}\n", mRomfsHeader.data_offset.unwrap());
	*/

	// get dir entry ptr
	tc::ByteData dir_entry_table = tc::ByteData();
	if (mRomfsHeader.dir_entry.size.unwrap() > 0)
	{
		dir_entry_table = tc::ByteData(tc::io::IOUtil::castInt64ToSize(mRomfsHeader.dir_entry.size.unwrap()));
		mFile->seek(mRomfsHeader.dir_entry.offset.unwrap(), tc::io::SeekOrigin::Begin);
		mFile->read(dir_entry_table.data(), dir_entry_table.size());
	}

	// get file entry ptr
	tc::ByteData file_entry_table = tc::ByteData();
	if (mRomfsHeader.file_entry.size.unwrap() > 0)
	{
		file_entry_table = tc::ByteData(tc::io::IOUtil::castInt64ToSize(mRomfsHeader.file_entry.size.unwrap()));
		mFile->seek(mRomfsHeader.file_entry.offset.unwrap(), tc::io::SeekOrigin::Begin);
		mFile->read(file_entry_table.data(), file_entry_table.size());
	}

	// count dir num
	mDirNum = 0;
	for (uint32_t v_addr = 0; size_t(v_addr) < dir_entry_table.size();)
	{
		uint32_t total_size = sizeof(pie::hac::sRomfsDirEntry) + align<uint32_t>(((pie::hac::sRomfsDirEntry*)(dir_entry_table.data() + v_addr))->name_size.unwrap(), 4);

		// don't count root directory
		if (v_addr != 0)
		{
			mDirNum += 1;
		}

		v_addr += total_size;
	}

	// count file num
	mFileNum = 0;
	for (uint32_t v_addr = 0; size_t(v_addr) < file_entry_table.size();)
	{
		uint32_t total_size = sizeof(pie::hac::sRomfsFileEntry) + align<uint32_t>(((pie::hac::sRomfsFileEntry*)(file_entry_table.data() + v_addr))->name_size.unwrap(), 4);

		mFileNum += 1;

		v_addr += total_size;
	}

	// create virtual filesystem
	mFileSystem = std::make_shared<tc::io::VirtualFileSystem>(tc::io::VirtualFileSystem(pie::hac::RomFsSnapshotGenerator(mFile)));
	mFsProcess.setInputFileSystem(mFileSystem);

	// set properties for FsProcess
	mFsProcess.setFsProperties({
		fmt::format("DirNum:      {:d}", mDirNum), 
		fmt::format("FileNum:     {:d}", mFileNum)
	});

	// process filesystem
	mFsProcess.process();
}

void nstool::RomfsProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::RomfsProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
	mFsProcess.setShowFsInfo(mCliOutputMode.show_basic_info);
}

void nstool::RomfsProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void nstool::RomfsProcess::setFsRootLabel(const std::string& root_label)
{
	mFsProcess.setFsRootLabel(root_label);
}

void nstool::RomfsProcess::setExtractJobs(const std::vector<nstool::ExtractJob>& extract_jobs)
{
	mFsProcess.setExtractJobs(extract_jobs);
}

void nstool::RomfsProcess::setShowFsTree(bool list_fs)
{
	mFsProcess.setShowFsTree(list_fs);
}