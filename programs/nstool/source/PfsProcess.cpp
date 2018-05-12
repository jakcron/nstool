#include <fnd/SimpleFile.h>
#include <fnd/io.h>
#include "OffsetAdjustedIFile.h"
#include "PfsProcess.h"

void PfsProcess::displayHeader()
{
	printf("[PartitionFS]\n");
	printf("  Type:        %s\n", mPfs.getFsType() == mPfs.TYPE_PFS0? "PFS0" : "HFS0");
	printf("  FileNum:     %u\n", mPfs.getFileList().getSize());
	if (mMountName.empty() == false)	
		printf("  MountPoint:  %s%s\n", mMountName.c_str(), mMountName.at(mMountName.length()-1) != '/' ? "/" : "");
}

void PfsProcess::displayFs()
{	
	for (size_t i = 0; i < mPfs.getFileList().getSize(); i++)
	{
		printf("    %s", mPfs.getFileList()[i].name.c_str());
		if (mCliOutputType >= OUTPUT_VERBOSE)
		{
			if (mPfs.getFsType() == mPfs.TYPE_PFS0)
				printf(" (offset=0x%" PRIx64 ", size=0x%" PRIx64 ")\n", mPfs.getFileList()[i].offset, mPfs.getFileList()[i].size);
			else
				printf(" (offset=0x%" PRIx64 ", size=0x%" PRIx64 ", hash_protected_size=0x%" PRIx64 ")\n", mPfs.getFileList()[i].offset, mPfs.getFileList()[i].size, mPfs.getFileList()[i].hash_protected_size);
		}
		else
		{
			printf("\n");
		}
		
	}
}

size_t PfsProcess::determineHeaderSize(const nx::sPfsHeader* hdr)
{
	size_t fileEntrySize = 0;
	if (std::string(hdr->signature, 4) == nx::pfs::kPfsSig)
		fileEntrySize = sizeof(nx::sPfsFile);
	else
		fileEntrySize = sizeof(nx::sHashedPfsFile);

	return sizeof(nx::sPfsHeader) + hdr->file_num.get() * fileEntrySize + hdr->name_table_size.get();
}

bool PfsProcess::validateHeaderMagic(const nx::sPfsHeader* hdr)
{
	return std::string(hdr->signature, 4) == nx::pfs::kPfsSig || std::string(hdr->signature, 4) == nx::pfs::kHashedPfsSig;
}

void PfsProcess::validateHfs()
{
	fnd::MemoryBlob scratch;
	crypto::sha::sSha256Hash hash;
	const fnd::List<nx::PfsHeader::sFile>& file = mPfs.getFileList();
	for (size_t i = 0; i < file.getSize(); i++)
	{
		scratch.alloc(file[i].hash_protected_size);
		mReader->read(scratch.getBytes(), file[i].offset, file[i].hash_protected_size);
		crypto::sha::Sha256(scratch.getBytes(), scratch.getSize(), hash.bytes);
		if (hash != file[i].hash)
		{
			if (mCliOutputType >= OUTPUT_MINIMAL)
				printf("[WARNING] HFS0 %s%s%s: FAIL (bad hash)\n", !mMountName.empty()? mMountName.c_str() : "", !mMountName.empty()? "/" : "", file[i].name.c_str());
	
		}
	}
}

void PfsProcess::extractFs()
{
	// allocate scratch memory
	fnd::MemoryBlob scratch;
	scratch.alloc(kFileExportBlockSize);

	// make extract dir
	fnd::io::makeDirectory(mExtractPath);

	fnd::SimpleFile outFile;
	const fnd::List<nx::PfsHeader::sFile>& file = mPfs.getFileList();

	std::string file_path;
	for (size_t i = 0; i < file.getSize(); i++)
	{
		file_path.clear();
		fnd::io::appendToPath(file_path, mExtractPath);
		fnd::io::appendToPath(file_path, file[i].name);
		outFile.open(file_path, outFile.Create);
		mReader->seek(file[i].offset);
		for (size_t j = 0; j < (file[i].size / kFileExportBlockSize); j++)
		{
			mReader->read(scratch.getBytes(), kFileExportBlockSize);
			outFile.write(scratch.getBytes(), kFileExportBlockSize);
		}
		if (file[i].size % kFileExportBlockSize)
		{
			mReader->read(scratch.getBytes(), file[i].size % kFileExportBlockSize);
			outFile.write(scratch.getBytes(), file[i].size % kFileExportBlockSize);
		}		
		outFile.close();
	}
}

PfsProcess::PfsProcess() :
	mReader(nullptr),
	mCliOutputType(OUTPUT_NORMAL),
	mVerify(false),
	mExtractPath(),
	mExtract(false),
	mMountName(),
	mListFs(false),
	mPfs()
{
}

PfsProcess::~PfsProcess()
{
	if (mReader != nullptr)
	{
		delete mReader;
	}
}

void PfsProcess::process()
{
	fnd::MemoryBlob scratch;

	if (mReader == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}
	
	// open minimum header to get full header size
	scratch.alloc(sizeof(nx::sPfsHeader));
	mReader->read(scratch.getBytes(), 0, scratch.getSize());
	if (validateHeaderMagic(((nx::sPfsHeader*)scratch.getBytes())) == false)
	{
		throw fnd::Exception(kModuleName, "Corrupt Header");
	}
	size_t pfsHeaderSize = determineHeaderSize(((nx::sPfsHeader*)scratch.getBytes()));
	
	// open minimum header to get full header size
	scratch.alloc(pfsHeaderSize);
	mReader->read(scratch.getBytes(), 0, scratch.getSize());
	mPfs.importBinary(scratch.getBytes(), scratch.getSize());

	if (mCliOutputType >= OUTPUT_NORMAL)
		displayHeader();
	if (mListFs || mCliOutputType >= OUTPUT_VERBOSE)
		displayFs();
	if (mPfs.getFsType() == mPfs.TYPE_HFS0 && mVerify)
		validateHfs();
	if (mExtract)
		extractFs();
}

void PfsProcess::setInputFile(fnd::IFile* file, size_t offset, size_t size)
{
	mReader = new OffsetAdjustedIFile(file, offset, size);
}

void PfsProcess::setCliOutputMode(CliOutputType type)
{
	mCliOutputType = type;
}

void PfsProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void PfsProcess::setMountPointName(const std::string& mount_name)
{
	mMountName = mount_name;
}

void PfsProcess::setExtractPath(const std::string& path)
{
	mExtract = true;
	mExtractPath = path;
}

void PfsProcess::setListFs(bool list_fs)
{
	mListFs = list_fs;
}

const nx::PfsHeader& PfsProcess::getPfsHeader() const
{
	return mPfs;
}
