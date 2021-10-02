#include "PfsProcess.h"
#include "util.h"

#include <nn/hac/PartitionFsUtil.h>
#include <tc/io/LocalStorage.h>


nstool::PfsProcess::PfsProcess() :
	mModuleName("nstool::PfsProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false),
	mExtractPath(),
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
	if (mExtractPath.isSet())
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

void nstool::PfsProcess::setExtractPath(const tc::io::Path& path)
{
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
}

void nstool::PfsProcess::displayHeader()
{
	fmt::print("[PartitionFS]\n");
	fmt::print("  Type:        {:s}\n", nn::hac::PartitionFsUtil::getFsTypeAsString(mPfs.getFsType()));
	fmt::print("  FileNum:     {:d}\n", mPfs.getFileList().size());
	if (mMountName.empty() == false)
	{
		fmt::print("  MountPoint:  {:s}");
		if (mMountName.at(mMountName.length()-1) != '/')
			fmt::print("/");
		fmt::print("\n");
	}
}

void nstool::PfsProcess::displayFs()
{	
	auto file_list = mPfs.getFileList();
	for (auto itr = file_list.begin(); itr != file_list.end(); itr++)
	{
		fmt::print("    {:s}", itr->name);
		if (mCliOutputMode.show_layout)
		{
			switch (mPfs.getFsType())
			{
			case (nn::hac::PartitionFsHeader::TYPE_PFS0):
				fmt::print(" (offset=0x{:x}, size=0x{:x})", itr->offset, itr->size);
				break;
			case (nn::hac::PartitionFsHeader::TYPE_HFS0):
				fmt::print(" (offset=0x{:x}, size=0x{:x}, hash_protected_size=0x{:x})", itr->offset, itr->size, itr->hash_protected_size);
				break;
			}
			
		}
		fmt::print("\n");
	}
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

void nstool::PfsProcess::validateHfs()
{
	nn::hac::detail::sha256_hash_t hash;
	auto file_list = mPfs.getFileList();
	for (auto itr = file_list.begin(); itr != file_list.end(); itr++)
	{
		tc::ByteData cache = tc::ByteData(tc::io::IOUtil::castInt64ToSize(itr->hash_protected_size));
		mFile->seek(itr->offset, tc::io::SeekOrigin::Begin);
		mFile->read(cache.data(), cache.size());
		tc::crypto::GenerateSha256Hash(hash.data(), cache.data(), cache.size());
		if (hash != itr->hash)
		{
			fmt::print("[WARNING] HFS0 {:s}{:s}{:s}: FAIL (bad hash)\n", !mMountName.empty()? mMountName.c_str() : "", (!mMountName.empty() && mMountName.at(mMountName.length()-1) != '/' )? "/" : "", itr->name);
		}
	}
}

void nstool::PfsProcess::extractFs()
{
	// create extract directory
	tc::io::LocalStorage fs;
	fs.createDirectory(mExtractPath.get());

	// extract files
	tc::ByteData cache_for_extract = tc::ByteData(kCacheSize);

	auto file_list = mPfs.getFileList();
	for (auto itr = file_list.begin(); itr != file_list.end(); itr++)
	{
		tc::io::Path extract_path = mExtractPath.get() + itr->name;

		writeSubStreamToFile(mFile, itr->offset, itr->size, extract_path, cache_for_extract);
	}
}