#include "IniProcess.h"

#include "util.h"
#include "KipProcess.h"

nstool::IniProcess::IniProcess() :
	mModuleName("nstool::IniProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false),
	mKipExtractPath()
{
}

void nstool::IniProcess::process()
{
	importHeader();
	importKipList();
	if (mCliOutputMode.show_basic_info)
	{
		displayHeader();
		displayKipList();
	}
	if (mKipExtractPath.isSet())
	{
		extractKipList();
	}
}

void nstool::IniProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::IniProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void nstool::IniProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void nstool::IniProcess::setKipExtractPath(const tc::io::Path& path)
{
	mKipExtractPath = path;
}

void nstool::IniProcess::importHeader()
{
	if (mFile == nullptr)
	{
		throw tc::Exception(mModuleName, "No file reader set.");
	}
	if (mFile->canRead() == false || mFile->canSeek() == false)
	{
		throw tc::NotSupportedException(mModuleName, "Input stream requires read/seek permissions.");
	}

	// check if file_size is smaller than INI header size
	if (tc::io::IOUtil::castInt64ToSize(mFile->length()) < sizeof(pie::hac::sIniHeader))
	{
		throw tc::Exception(mModuleName, "Corrupt INI: file too small.");
	}

	// read ini
	tc::ByteData scratch = tc::ByteData(sizeof(pie::hac::sIniHeader));
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());

	// parse ini header
	mHdr.fromBytes(scratch.data(), scratch.size());
}

void nstool::IniProcess::importKipList()
{
	// kip pos info
	int64_t kip_pos = tc::io::IOUtil::castSizeToInt64(sizeof(pie::hac::sIniHeader));
	int64_t kip_size = 0;

	// tmp data to determine size
	pie::hac::sKipHeader hdr_raw;
	pie::hac::KernelInitialProcessHeader hdr;

	for (size_t i = 0; i < mHdr.getKipNum(); i++)
	{
		mFile->seek(kip_pos, tc::io::SeekOrigin::Begin);
		mFile->read((byte_t*)&hdr_raw, sizeof(hdr_raw));
		hdr.fromBytes((byte_t*)&hdr_raw, sizeof(hdr_raw));		
		kip_size = getKipSizeFromHeader(hdr);
		mKipList.push_back({hdr, std::make_shared<tc::io::SubStream>(tc::io::SubStream(mFile, kip_pos, kip_size))});
		kip_pos += kip_size;
	}
}

void nstool::IniProcess::displayHeader()
{
	fmt::print("[INI Header]\n");
	fmt::print("  Size:         0x{:x}\n", mHdr.getSize());
	fmt::print("  KIP Num:      {:d}\n", mHdr.getKipNum());
}

void nstool::IniProcess::displayKipList()
{
	for (auto itr = mKipList.begin(); itr != mKipList.end(); itr++)
	{
		KipProcess obj;

		obj.setInputFile(itr->stream);
		obj.setCliOutputMode(mCliOutputMode);
		obj.setVerifyMode(mVerify);

		obj.process();
	}
}

void nstool::IniProcess::extractKipList()
{
	// allocate cache memory
	tc::ByteData cache = tc::ByteData(kCacheSize);

	// make extract dir
	tc::io::LocalFileSystem local_fs;
	local_fs.createDirectory(mKipExtractPath.get());
	
	// out path for extracted KIP
	tc::io::Path out_path;

	// extract KIPs
	for (auto itr = mKipList.begin(); itr != mKipList.end(); itr++)
	{
		out_path = mKipExtractPath.get();
		out_path += fmt::format("{:s}.kip", itr->hdr.getName());

		if (mCliOutputMode.show_basic_info)
			fmt::print("Saving {:s}...\n", out_path.to_string());

		writeStreamToFile(itr->stream, out_path, cache);
	}
}

int64_t nstool::IniProcess::getKipSizeFromHeader(const pie::hac::KernelInitialProcessHeader& hdr) const
{	
	// the order of elements in a KIP are sequential, there are no file offsets
	return int64_t(sizeof(pie::hac::sKipHeader)) + int64_t(hdr.getTextSegmentInfo().file_layout.size + hdr.getRoSegmentInfo().file_layout.size + hdr.getDataSegmentInfo().file_layout.size);
}