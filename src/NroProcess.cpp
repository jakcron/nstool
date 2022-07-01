#include "NroProcess.h"

nstool::NroProcess::NroProcess() :
	mModuleName("nstool::NroProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false)
{
}

void nstool::NroProcess::process()
{
	importHeader();
	importCodeSegments();

	if (mCliOutputMode.show_basic_info)
		displayHeader();

	processRoMeta();

	if (mIsHomebrewNro)
		mAssetProc.process();
}

void nstool::NroProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::NroProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void nstool::NroProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void nstool::NroProcess::setIs64BitInstruction(bool flag)
{
	mRoMeta.setIs64BitInstruction(flag);
}

void nstool::NroProcess::setListApi(bool listApi)
{
	mRoMeta.setListApi(listApi);
}

void nstool::NroProcess::setListSymbols(bool listSymbols)
{
	mRoMeta.setListSymbols(listSymbols);
}

void nstool::NroProcess::setAssetIconExtractPath(const tc::io::Path& path)
{
	mAssetProc.setIconExtractPath(path);
}

void nstool::NroProcess::setAssetNacpExtractPath(const tc::io::Path& path)
{
	mAssetProc.setNacpExtractPath(path);
}

void nstool::NroProcess::setAssetRomfsShowFsTree(bool show_fs_tree)
{
	mAssetProc.setRomfsShowFsTree(show_fs_tree);
}

void nstool::NroProcess::setAssetRomfsExtractJobs(const std::vector<nstool::ExtractJob>& extract_jobs)
{
	mAssetProc.setRomfsExtractJobs(extract_jobs);
}

const nstool::RoMetadataProcess& nstool::NroProcess::getRoMetadataProcess() const
{
	return mRoMeta;
}

void nstool::NroProcess::importHeader()
{
	if (mFile == nullptr)
	{
		throw tc::Exception(mModuleName, "No file reader set.");
	}
	if (mFile->canRead() == false || mFile->canSeek() == false)
	{
		throw tc::NotSupportedException(mModuleName, "Input stream requires read/seek permissions.");
	}

	// check if file_size is smaller than NRO header size
	if (tc::io::IOUtil::castInt64ToSize(mFile->length()) < sizeof(pie::hac::sNroHeader))
	{
		throw tc::Exception(mModuleName, "Corrupt NRO: file too small.");
	}

	// read nro
	tc::ByteData scratch = tc::ByteData(sizeof(pie::hac::sNroHeader));
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());

	// parse nro header
	mHdr.fromBytes(scratch.data(), scratch.size());

	// setup homebrew extension
	pie::hac::sNroHeader* raw_hdr = (pie::hac::sNroHeader*)scratch.data();

	int64_t file_size = mFile->length();
	if (((tc::bn::le64<uint64_t>*)raw_hdr->reserved_0.data())->unwrap() == pie::hac::nro::kNroHomebrewStructMagic && file_size > int64_t(mHdr.getNroSize()))
	{
		mIsHomebrewNro = true;
		mAssetProc.setInputFile(std::make_shared<tc::io::SubStream>(tc::io::SubStream(mFile, int64_t(mHdr.getNroSize()), file_size - int64_t(mHdr.getNroSize()))));
		mAssetProc.setCliOutputMode(mCliOutputMode);
		mAssetProc.setVerifyMode(mVerify);
	}
	else
		mIsHomebrewNro = false;
}

void nstool::NroProcess::importCodeSegments()
{
	if (mHdr.getTextInfo().size > 0)
	{
		mTextBlob = tc::ByteData(mHdr.getTextInfo().size);
		mFile->seek(mHdr.getTextInfo().memory_offset, tc::io::SeekOrigin::Begin);
		mFile->read(mTextBlob.data(), mTextBlob.size());
	}

	if (mHdr.getRoInfo().size > 0)
	{
		mRoBlob = tc::ByteData(mHdr.getRoInfo().size);
		mFile->seek(mHdr.getRoInfo().memory_offset, tc::io::SeekOrigin::Begin);
		mFile->read(mRoBlob.data(), mRoBlob.size());
	}

	if (mHdr.getDataInfo().size > 0)
	{
		mDataBlob = tc::ByteData(mHdr.getDataInfo().size);
		mFile->seek(mHdr.getDataInfo().memory_offset, tc::io::SeekOrigin::Begin);
		mFile->read(mDataBlob.data(), mDataBlob.size());
	}
}

void nstool::NroProcess::displayHeader()
{
	fmt::print("[NRO Header]\n");
	fmt::print("  RoCrt:       \n");
	fmt::print("    EntryPoint: 0x{:x}\n", mHdr.getRoCrtEntryPoint());
	fmt::print("    ModOffset:  0x{:x}\n", mHdr.getRoCrtModOffset());
	fmt::print("  ModuleId:    {:s}\n", tc::cli::FormatUtil::formatBytesAsString(mHdr.getModuleId().data(), mHdr.getModuleId().size(), false, ""));
	fmt::print("  NroSize:     0x{:x}\n", mHdr.getNroSize());
	fmt::print("  Program Sections:\n");
	fmt::print("     .text:\n");
	fmt::print("      Offset:     0x{:x}\n", mHdr.getTextInfo().memory_offset);
	fmt::print("      Size:       0x{:x}\n", mHdr.getTextInfo().size);
	fmt::print("    .ro:\n");
	fmt::print("      Offset:     0x{:x}\n", mHdr.getRoInfo().memory_offset);
	fmt::print("      Size:       0x{:x}\n", mHdr.getRoInfo().size);
	if (mCliOutputMode.show_extended_info)
	{
		fmt::print("    .api_info:\n");
		fmt::print("      Offset:     0x{:x}\n", mHdr.getRoEmbeddedInfo().memory_offset);
		fmt::print("      Size:       0x{:x}\n", mHdr.getRoEmbeddedInfo().size);
		fmt::print("    .dynstr:\n");
		fmt::print("      Offset:     0x{:x}\n", mHdr.getRoDynStrInfo().memory_offset);
		fmt::print("      Size:       0x{:x}\n", mHdr.getRoDynStrInfo().size);
		fmt::print("    .dynsym:\n");
		fmt::print("      Offset:     0x{:x}\n", mHdr.getRoDynSymInfo().memory_offset);
		fmt::print("      Size:       0x{:x}\n", mHdr.getRoDynSymInfo().size);
	}                                                                
	fmt::print("    .data:\n");
	fmt::print("      Offset:     0x{:x}\n", mHdr.getDataInfo().memory_offset);
	fmt::print("      Size:       0x{:x}\n", mHdr.getDataInfo().size);
	fmt::print("    .bss:\n");
	fmt::print("      Size:       0x{:x}\n", mHdr.getBssSize());
}

void nstool::NroProcess::processRoMeta()
{
	if (mRoBlob.size())
	{
		// setup ro metadata
		mRoMeta.setApiInfo(mHdr.getRoEmbeddedInfo().memory_offset, mHdr.getRoEmbeddedInfo().size);
		mRoMeta.setDynSym(mHdr.getRoDynSymInfo().memory_offset, mHdr.getRoDynSymInfo().size);
		mRoMeta.setDynStr(mHdr.getRoDynStrInfo().memory_offset, mHdr.getRoDynStrInfo().size);
		mRoMeta.setRoBinary(mRoBlob);
		mRoMeta.setCliOutputMode(mCliOutputMode);
		mRoMeta.process();
	}
}