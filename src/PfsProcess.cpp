#include "PfsProcess.h"
#include "util.h"

#include <nn/hac/PartitionFsUtil.h>
#include <tc/io/LocalStorage.h>

#include "FsProcess.h"
#include <tc/io/VirtualFileSystem.h>
#include <nn/hac/PartitionFsMetaGenerator.h>


nstool::PfsProcess::PfsProcess() :
	mModuleName("nstool::PfsProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false),
	mPfs(),
	mFileSystem(),
	mFsProcess()
{
	mFsProcess.setFsLabel("PartitionFS");
}

void nstool::PfsProcess::process()
{
	importHeader();

	if (mCliOutputMode.show_basic_info)
	{
		displayHeader();
	}

	mFsProcess.process();
}

void nstool::PfsProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::PfsProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void nstool::PfsProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void nstool::PfsProcess::setMountPointName(const std::string& mount_name)
{
	mFsProcess.setFsLabel(mount_name);
}

void nstool::PfsProcess::setExtractPath(const tc::io::Path& path)
{
	mFsProcess.setExtractPath(path);
}

void nstool::PfsProcess::setListFs(bool list_fs)
{
	mFsProcess.setCliOutputMode(list_fs);
}

const nn::hac::PartitionFsHeader& nstool::PfsProcess::getPfsHeader() const
{
	return mPfs;
}

const std::shared_ptr<tc::io::IStorage>& nstool::PfsProcess::getFileSystem() const
{
	return mFileSystem;
}

void nstool::PfsProcess::importHeader()
{
	if (mFile == nullptr)
	{
		throw tc::Exception(mModuleName, "No file reader set.");
	}

	tc::ByteData scratch;

	// read base header to determine complete header size
	if (mFile->length() < tc::io::IOUtil::castSizeToInt64(sizeof(nn::hac::sPfsHeader)))
	{
		throw tc::Exception(mModuleName, "Corrupt PartitionFs: File too small");
	}

	scratch = tc::ByteData(sizeof(nn::hac::sPfsHeader));
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());
	if (validateHeaderMagic(((nn::hac::sPfsHeader*)scratch.data())) == false)
	{
		throw tc::Exception(mModuleName, "Corrupt PartitionFs: Header had incorrect struct magic.");
	}

	// read complete size header
	size_t pfsHeaderSize = determineHeaderSize(((nn::hac::sPfsHeader*)scratch.data()));
	if (mFile->length() < tc::io::IOUtil::castSizeToInt64(pfsHeaderSize))
	{
		throw tc::Exception(mModuleName, "Corrupt PartitionFs: File too small");
	}

	scratch = tc::ByteData(pfsHeaderSize);
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());

	// process PFS
	mPfs.fromBytes(scratch.data(), scratch.size());

	// create virtual filesystem
	mFileSystem = std::make_shared<tc::io::VirtualFileSystem>(tc::io::VirtualFileSystem(nn::hac::PartitionFsMetaGenerator(mFile, mVerify ? nn::hac::PartitionFsMetaGenerator::ValidationMode_Warn : nn::hac::PartitionFsMetaGenerator::ValidationMode_None)));
	mFsProcess.setInputFileSystem(mFileSystem);
}

void nstool::PfsProcess::displayHeader()
{
	fmt::print("[PartitionFS]\n");
	fmt::print("  Type:        {:s}\n", nn::hac::PartitionFsUtil::getFsTypeAsString(mPfs.getFsType()));
	fmt::print("  FileNum:     {:d}\n", mPfs.getFileList().size());
}

size_t nstool::PfsProcess::determineHeaderSize(const nn::hac::sPfsHeader* hdr)
{
	size_t fileEntrySize = 0;
	if (hdr->st_magic.unwrap() == nn::hac::pfs::kPfsStructMagic)
		fileEntrySize = sizeof(nn::hac::sPfsFile);
	else
		fileEntrySize = sizeof(nn::hac::sHashedPfsFile);

	return sizeof(nn::hac::sPfsHeader) + hdr->file_num.unwrap() * fileEntrySize + hdr->name_table_size.unwrap();
}

bool nstool::PfsProcess::validateHeaderMagic(const nn::hac::sPfsHeader* hdr)
{
	return hdr->st_magic.unwrap() == nn::hac::pfs::kPfsStructMagic || hdr->st_magic.unwrap() == nn::hac::pfs::kHashedPfsStructMagic;
}