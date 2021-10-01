#include "PfsProcess.h"

#include <iostream>
#include <iomanip>

#include <fnd/SimpleFile.h>
#include <fnd/io.h>

#include <nn/hac/PartitionFsUtil.h>


nstool::PfsProcess::PfsProcess() :
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false),
	mExtractPath(),
	mExtract(false),
	mMountName(),
	mListFs(false),
	mPfs()
{
}

void nstool::PfsProcess::process()
{
	importHeader();

	if (mCliOutputMode.show_basic_info)
	{
		displayHeader();
		if (mListFs || mCliOutputMode.show_extended_info)
			displayFs();
	}
	if (mPfs.getFsType() == mPfs.TYPE_HFS0 && mVerify)
		validateHfs();
	if (mExtract)
		extractFs();
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
	mMountName = mount_name;
}

void nstool::PfsProcess::setExtractPath(const std::string& path)
{
	mExtract = true;
	mExtractPath = path;
}

void nstool::PfsProcess::setListFs(bool list_fs)
{
	mListFs = list_fs;
}

const nn::hac::PartitionFsHeader& nstool::PfsProcess::getPfsHeader() const
{
	return mPfs;
}

void nstool::PfsProcess::importHeader()
{
	tc::ByteData scratch;

	if (*mFile == nullptr)
	{
		throw tc::Exception(kModuleName, "No file reader set.");
	}
	
	// open minimum header to get full header size
	scratch.alloc(sizeof(nn::hac::sPfsHeader));
	(*mFile)->read(scratch.data(), 0, scratch.size());
	if (validateHeaderMagic(((nn::hac::sPfsHeader*)scratch.data())) == false)
	{
		throw tc::Exception(kModuleName, "Corrupt Header");
	}
	size_t pfsHeaderSize = determineHeaderSize(((nn::hac::sPfsHeader*)scratch.data()));
	
	// open minimum header to get full header size
	scratch.alloc(pfsHeaderSize);
	(*mFile)->read(scratch.data(), 0, scratch.size());
	mPfs.fromBytes(scratch.data(), scratch.size());
}

void nstool::PfsProcess::displayHeader()
{
	std::cout << "[PartitionFS]" << std::endl;
	std::cout << "  Type:        " << nn::hac::PartitionFsUtil::getFsTypeAsString(mPfs.getFsType()) << std::endl;
	std::cout << "  FileNum:     " << std::dec << mPfs.getFileList().size() << std::endl;
	if (mMountName.empty() == false)
	{
		std::cout << "  MountPoint:  " << mMountName;
		if (mMountName.at(mMountName.length()-1) != '/')
			std::cout << "/";
		std::cout << std::endl;
	}
}

void nstool::PfsProcess::displayFs()
{	
	for (size_t i = 0; i < mPfs.getFileList().size(); i++)
	{
		const nn::hac::PartitionFsHeader::sFile& file = mPfs.getFileList()[i];
		std::cout << "    " << file.name;
		if (mCliOutputMode.show_layout)
		{
			switch (mPfs.getFsType())
			{
			case (nn::hac::PartitionFsHeader::TYPE_PFS0):
				std::cout << std::hex << " (offset=0x" << file.offset << ", size=0x" << file.size << ")";
				break;
			case (nn::hac::PartitionFsHeader::TYPE_HFS0):
				std::cout << std::hex << " (offset=0x" << file.offset << ", size=0x" << file.size << ", hash_protected_size=0x" << file.hash_protected_size << ")";
				break;
			}
			
		}
		std::cout << std::endl;
	}
}

size_t nstool::PfsProcess::determineHeaderSize(const nn::hac::sPfsHeader* hdr)
{
	size_t fileEntrySize = 0;
	if (hdr->st_magic.get() == nn::hac::pfs::kPfsStructMagic)
		fileEntrySize = sizeof(nn::hac::sPfsFile);
	else
		fileEntrySize = sizeof(nn::hac::sHashedPfsFile);

	return sizeof(nn::hac::sPfsHeader) + hdr->file_num.get() * fileEntrySize + hdr->name_table_size.get();
}

bool nstool::PfsProcess::validateHeaderMagic(const nn::hac::sPfsHeader* hdr)
{
	return hdr->st_magic.get() == nn::hac::pfs::kPfsStructMagic || hdr->st_magic.get() == nn::hac::pfs::kHashedPfsStructMagic;
}

void nstool::PfsProcess::validateHfs()
{
	fnd::sha::sSha256Hash hash;
	const std::vector<nn::hac::PartitionFsHeader::sFile>& file = mPfs.getFileList();
	for (size_t i = 0; i < file.size(); i++)
	{
		mCache.alloc(file[i].hash_protected_size);
		(*mFile)->read(mCache.data(), file[i].offset, file[i].hash_protected_size);
		fnd::sha::Sha256(mCache.data(), file[i].hash_protected_size, hash.bytes);
		if (hash != file[i].hash)
		{
			printf("[WARNING] HFS0 %s%s%s: FAIL (bad hash)\n", !mMountName.empty()? mMountName.c_str() : "", (!mMountName.empty() && mMountName.at(mMountName.length()-1) != '/' )? "/" : "", file[i].name.c_str());
		}
	}
}

void nstool::PfsProcess::extractFs()
{
	// allocate only when extractDir is invoked
	mCache.alloc(kCacheSize);

	// make extract dir
	fnd::io::makeDirectory(mExtractPath);

	fnd::SimpleFile outFile;
	const std::vector<nn::hac::PartitionFsHeader::sFile>& file = mPfs.getFileList();

	std::string file_path;
	for (size_t i = 0; i < file.size(); i++)
	{
		file_path.clear();
		fnd::io::appendToPath(file_path, mExtractPath);
		fnd::io::appendToPath(file_path, file[i].name);

		if (mCliOutputMode.show_basic_info)
			printf("extract=[%s]\n", file_path.c_str());

		outFile.open(file_path, outFile.Create);
		(*mFile)->seek(file[i].offset);
		for (size_t j = 0; j < ((file[i].size / kCacheSize) + ((file[i].size % kCacheSize) != 0)); j++)
		{
			(*mFile)->read(mCache.data(), _MIN(file[i].size - (kCacheSize * j),kCacheSize));
			outFile.write(mCache.data(), _MIN(file[i].size - (kCacheSize * j),kCacheSize));
		}		
		outFile.close();
	}
}