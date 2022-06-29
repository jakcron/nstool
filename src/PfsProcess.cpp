#include "PfsProcess.h"
#include "util.h"

#include <pietendo/hac/PartitionFsUtil.h>
#include <tc/io/LocalFileSystem.h>

#include <tc/io/VirtualFileSystem.h>
#include <pietendo/hac/PartitionFsSnapshotGenerator.h>


nstool::PfsProcess::PfsProcess() :
	mModuleName("nstool::PfsProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false),
	mPfs(),
	mFileSystem(),
	mFsProcess()
{
	mFsProcess.setFsFormatName("PartitionFs");
}

void nstool::PfsProcess::process()
{
	if (mFile == nullptr)
	{
		throw tc::Exception(mModuleName, "No file reader set.");
	}
	if (mFile->canRead() == false || mFile->canSeek() == false)
	{
		throw tc::NotSupportedException(mModuleName, "Input stream requires read/seek permissions.");
	}

	tc::ByteData scratch;

	// read base header to determine complete header size
	if (mFile->length() < tc::io::IOUtil::castSizeToInt64(sizeof(pie::hac::sPfsHeader)))
	{
		throw tc::Exception(mModuleName, "Corrupt PartitionFs: File too small");
	}

	scratch = tc::ByteData(sizeof(pie::hac::sPfsHeader));
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());
	if (validateHeaderMagic(((pie::hac::sPfsHeader*)scratch.data())) == false)
	{
		throw tc::Exception(mModuleName, "Corrupt PartitionFs: Header had incorrect struct magic.");
	}

	// read complete size header
	size_t pfsHeaderSize = determineHeaderSize(((pie::hac::sPfsHeader*)scratch.data()));
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
	mFileSystem = std::make_shared<tc::io::VirtualFileSystem>(tc::io::VirtualFileSystem(pie::hac::PartitionFsSnapshotGenerator(mFile, mVerify ? pie::hac::PartitionFsSnapshotGenerator::ValidationMode_Warn : pie::hac::PartitionFsSnapshotGenerator::ValidationMode_None)));
	mFsProcess.setInputFileSystem(mFileSystem);

	// set properties for FsProcess
	mFsProcess.setFsProperties({
		fmt::format("Type:        {:s}", pie::hac::PartitionFsUtil::getFsTypeAsString(mPfs.getFsType())), 
		fmt::format("FileNum:     {:d}", mPfs.getFileList().size())
	});
	
	mFsProcess.process();
}

void nstool::PfsProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::PfsProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
	mFsProcess.setShowFsInfo(mCliOutputMode.show_basic_info);
}

void nstool::PfsProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void nstool::PfsProcess::setShowFsTree(bool show_fs_tree)
{
	mFsProcess.setShowFsTree(show_fs_tree);
}

void nstool::PfsProcess::setFsRootLabel(const std::string& root_label)
{
	mFsProcess.setFsRootLabel(root_label);
}

void nstool::PfsProcess::setExtractJobs(const std::vector<nstool::ExtractJob>& extract_jobs)
{
	mFsProcess.setExtractJobs(extract_jobs);
}

const pie::hac::PartitionFsHeader& nstool::PfsProcess::getPfsHeader() const
{
	return mPfs;
}

const std::shared_ptr<tc::io::IFileSystem>& nstool::PfsProcess::getFileSystem() const
{
	return mFileSystem;
}

size_t nstool::PfsProcess::determineHeaderSize(const pie::hac::sPfsHeader* hdr)
{
	size_t fileEntrySize = 0;
	if (hdr->st_magic.unwrap() == pie::hac::pfs::kPfsStructMagic)
		fileEntrySize = sizeof(pie::hac::sPfsFile);
	else
		fileEntrySize = sizeof(pie::hac::sHashedPfsFile);

	return sizeof(pie::hac::sPfsHeader) + hdr->file_num.unwrap() * fileEntrySize + hdr->name_table_size.unwrap();
}

bool nstool::PfsProcess::validateHeaderMagic(const pie::hac::sPfsHeader* hdr)
{
	return hdr->st_magic.unwrap() == pie::hac::pfs::kPfsStructMagic || hdr->st_magic.unwrap() == pie::hac::pfs::kHashedPfsStructMagic;
}