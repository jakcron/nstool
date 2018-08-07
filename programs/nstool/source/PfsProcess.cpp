#include <fnd/SimpleFile.h>
#include <fnd/io.h>
#include "PfsProcess.h"

PfsProcess::PfsProcess() :
	mFile(nullptr),
	mOwnIFile(false),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
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
	if (mOwnIFile)
	{
		delete mFile;
	}
}

void PfsProcess::process()
{
	fnd::Vec<byte_t> scratch;

	if (mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}
	
	// open minimum header to get full header size
	scratch.alloc(sizeof(nn::hac::sPfsHeader));
	mFile->read(scratch.data(), 0, scratch.size());
	if (validateHeaderMagic(((nn::hac::sPfsHeader*)scratch.data())) == false)
	{
		throw fnd::Exception(kModuleName, "Corrupt Header");
	}
	size_t pfsHeaderSize = determineHeaderSize(((nn::hac::sPfsHeader*)scratch.data()));
	
	// open minimum header to get full header size
	scratch.alloc(pfsHeaderSize);
	mFile->read(scratch.data(), 0, scratch.size());
	mPfs.fromBytes(scratch.data(), scratch.size());

	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
	{
		displayHeader();
		if (mListFs || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
			displayFs();
	}
	if (mPfs.getFsType() == mPfs.TYPE_HFS0 && mVerify)
		validateHfs();
	if (mExtract)
		extractFs();
}

void PfsProcess::setInputFile(fnd::IFile* file, bool ownIFile)
{
	mFile = file;
	mOwnIFile = ownIFile;
}

void PfsProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
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

const nn::hac::PfsHeader& PfsProcess::getPfsHeader() const
{
	return mPfs;
}

void PfsProcess::displayHeader()
{
	printf("[PartitionFS]\n");
	printf("  Type:        %s\n", mPfs.getFsType() == mPfs.TYPE_PFS0? "PFS0" : "HFS0");
	printf("  FileNum:     %" PRId64 "\n", (uint64_t)mPfs.getFileList().size());
	if (mMountName.empty() == false)	
		printf("  MountPoint:  %s%s\n", mMountName.c_str(), mMountName.at(mMountName.length()-1) != '/' ? "/" : "");
}

void PfsProcess::displayFs()
{	
	for (size_t i = 0; i < mPfs.getFileList().size(); i++)
	{
		printf("    %s", mPfs.getFileList()[i].name.c_str());
		if (_HAS_BIT(mCliOutputMode, OUTPUT_LAYOUT))
		{
			if (mPfs.getFsType() == mPfs.TYPE_PFS0)
				printf(" (offset=0x%" PRIx64 ", size=0x%" PRIx64 ")\n", (uint64_t)mPfs.getFileList()[i].offset, (uint64_t)mPfs.getFileList()[i].size);
			else
				printf(" (offset=0x%" PRIx64 ", size=0x%" PRIx64 ", hash_protected_size=0x%" PRIx64 ")\n", (uint64_t)mPfs.getFileList()[i].offset, (uint64_t)mPfs.getFileList()[i].size, (uint64_t)mPfs.getFileList()[i].hash_protected_size);
		}
		else
		{
			printf("\n");
		}
		
	}
}

size_t PfsProcess::determineHeaderSize(const nn::hac::sPfsHeader* hdr)
{
	size_t fileEntrySize = 0;
	if (hdr->st_magic.get() == nn::hac::pfs::kPfsStructMagic)
		fileEntrySize = sizeof(nn::hac::sPfsFile);
	else
		fileEntrySize = sizeof(nn::hac::sHashedPfsFile);

	return sizeof(nn::hac::sPfsHeader) + hdr->file_num.get() * fileEntrySize + hdr->name_table_size.get();
}

bool PfsProcess::validateHeaderMagic(const nn::hac::sPfsHeader* hdr)
{
	return hdr->st_magic.get() == nn::hac::pfs::kPfsStructMagic || hdr->st_magic.get() == nn::hac::pfs::kHashedPfsStructMagic;
}

void PfsProcess::validateHfs()
{
	fnd::sha::sSha256Hash hash;
	const fnd::List<nn::hac::PfsHeader::sFile>& file = mPfs.getFileList();
	for (size_t i = 0; i < file.size(); i++)
	{
		mCache.alloc(file[i].hash_protected_size);
		mFile->read(mCache.data(), file[i].offset, file[i].hash_protected_size);
		fnd::sha::Sha256(mCache.data(), file[i].hash_protected_size, hash.bytes);
		if (hash != file[i].hash)
		{
			printf("[WARNING] HFS0 %s%s%s: FAIL (bad hash)\n", !mMountName.empty()? mMountName.c_str() : "", (!mMountName.empty() && mMountName.at(mMountName.length()-1) != '/' )? "/" : "", file[i].name.c_str());
		}
	}
}

void PfsProcess::extractFs()
{
	// allocate only when extractDir is invoked
	mCache.alloc(kCacheSize);

	// make extract dir
	fnd::io::makeDirectory(mExtractPath);

	fnd::SimpleFile outFile;
	const fnd::List<nn::hac::PfsHeader::sFile>& file = mPfs.getFileList();

	std::string file_path;
	for (size_t i = 0; i < file.size(); i++)
	{
		file_path.clear();
		fnd::io::appendToPath(file_path, mExtractPath);
		fnd::io::appendToPath(file_path, file[i].name);

		if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
			printf("extract=[%s]\n", file_path.c_str());

		outFile.open(file_path, outFile.Create);
		mFile->seek(file[i].offset);
		for (size_t j = 0; j < ((file[i].size / kCacheSize) + ((file[i].size % kCacheSize) != 0)); j++)
		{
			mFile->read(mCache.data(), _MIN(file[i].size - (kCacheSize * j),kCacheSize));
			outFile.write(mCache.data(), _MIN(file[i].size - (kCacheSize * j),kCacheSize));
		}		
		outFile.close();
	}
}
