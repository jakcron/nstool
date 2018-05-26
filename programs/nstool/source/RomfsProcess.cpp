#include <fnd/SimpleTextOutput.h>
#include <fnd/SimpleFile.h>
#include <fnd/io.h>
#include "OffsetAdjustedIFile.h"
#include "RomfsProcess.h"

RomfsProcess::RomfsProcess() :
	mReader(nullptr),
	mCliOutputType(OUTPUT_NORMAL),
	mVerify(false),
	mExtractPath(),
	mExtract(false),
	mMountName(),
	mListFs(false),
	mDirNum(0),
	mFileNum(0)
{
	mRootDir.name.clear();
	mRootDir.dir_list.clear();
	mRootDir.file_list.clear();
}

RomfsProcess::~RomfsProcess()
{
	if (mReader != nullptr)
	{
		delete mReader;
	}
}

void RomfsProcess::process()
{
	if (mReader == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	resolveRomfs();	

	if (mCliOutputType >= OUTPUT_NORMAL)
		displayHeader();
	if (mListFs || mCliOutputType >= OUTPUT_VERBOSE)
		displayFs();
	if (mExtract)
		extractFs();
}

void RomfsProcess::setInputFile(fnd::IFile* file, size_t offset, size_t size)
{
	mReader = new OffsetAdjustedIFile(file, offset, size);
}

void RomfsProcess::setCliOutputMode(CliOutputType type)
{
	mCliOutputType = type;
}

void RomfsProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void RomfsProcess::setMountPointName(const std::string& mount_name)
{
	mMountName = mount_name;
}

void RomfsProcess::setExtractPath(const std::string& path)
{
	mExtract = true;
	mExtractPath = path;
}

void RomfsProcess::setListFs(bool list_fs)
{
	mListFs = list_fs;
}

const RomfsProcess::sDirectory& RomfsProcess::getRootDir() const
{
	return mRootDir;
}

void RomfsProcess::printTab(size_t tab) const
{
	for (size_t i = 0; i < tab; i++)
	{
		printf("  ");
	}
}

void RomfsProcess::displayFile(const sFile& file, size_t tab) const
{
	printTab(tab);
	printf("%s", file.name.c_str());
	if (mCliOutputType >= OUTPUT_VERBOSE)
	{
		printf(" (offset=0x%" PRIx64 ", size=0x%" PRIx64 ")", file.offset, file.size);
	}
	putchar('\n');
}

void RomfsProcess::displayDir(const sDirectory& dir, size_t tab) const
{
	if (dir.name.empty() == false)
	{
		printTab(tab);
		printf("%s\n", dir.name.c_str());
	}

	for (size_t i = 0; i < dir.dir_list.getSize(); i++)
	{
		displayDir(dir.dir_list[i], tab+1);
	}
	for (size_t i = 0; i < dir.file_list.getSize(); i++)
	{
		displayFile(dir.file_list[i], tab+1);
	}
}

void RomfsProcess::displayHeader()
{
	printf("[RomFS]\n");
	printf("  DirNum:      %" PRId64 "\n", mDirNum);
	printf("  FileNum:     %" PRId64 "\n", mFileNum);
	if (mMountName.empty() == false)	
		printf("  MountPoint:  %s%s\n", mMountName.c_str(), mMountName.at(mMountName.length()-1) != '/' ? "/" : "");
}

void RomfsProcess::displayFs()
{	
	displayDir(mRootDir, 1);
}

void RomfsProcess::extractDir(const std::string& path, const sDirectory& dir)
{
	std::string dir_path;
	std::string file_path;

	// make dir path
	fnd::io::appendToPath(dir_path, path);
	if (dir.name.empty() == false)
		fnd::io::appendToPath(dir_path, dir.name);

	//printf("dirpath=[%s]\n", dir_path.c_str());

	// make directory
	fnd::io::makeDirectory(dir_path);


	// allocate memory for file extraction
#ifdef NSTOOL_ALLOC_UNIQUE_SCRATCH
	fnd::MemoryBlob scratch;
	scratch.alloc(kFileExportBlockSize);
#endif

	// extract files
	fnd::SimpleFile outFile;
	for (size_t i = 0; i < dir.file_list.getSize(); i++)
	{
		file_path.clear();
		fnd::io::appendToPath(file_path, dir_path);
		fnd::io::appendToPath(file_path, dir.file_list[i].name);

		if (mCliOutputType >= OUTPUT_VERBOSE)
			printf("extract=[%s]\n", file_path.c_str());
		
		
		outFile.open(file_path, outFile.Create);
		mReader->seek(dir.file_list[i].offset);
		for (size_t j = 0; j < (dir.file_list[i].size / kFileExportBlockSize); j++)
		{
			mReader->read(mFileExtractBlock.getBytes(), kFileExportBlockSize);
			outFile.write(mFileExtractBlock.getBytes(), kFileExportBlockSize);
		}
		if (dir.file_list[i].size % kFileExportBlockSize)
		{
			mReader->read(mFileExtractBlock.getBytes(), dir.file_list[i].size % kFileExportBlockSize);
			outFile.write(mFileExtractBlock.getBytes(), dir.file_list[i].size % kFileExportBlockSize);
		}		
		outFile.close();
	}

	for (size_t i = 0; i < dir.dir_list.getSize(); i++)
	{
		extractDir(dir_path, dir.dir_list[i]);
	}
}


void RomfsProcess::extractFs()
{
	// allocate only when extractDir is invoked
	mFileExtractBlock.alloc(kFileExportBlockSize);
	extractDir(mExtractPath, mRootDir);
}

bool RomfsProcess::validateHeaderLayout(const nx::sRomfsHeader* hdr) const
{
	bool validLayout = true;

	if (hdr->header_size.get() != sizeof(nx::sRomfsHeader))
	{
		validLayout = false;
	}

	uint64_t pos = hdr->sections[0].offset.get();
	for (size_t i = 0; i < nx::romfs::SECTION_NUM; i++)
	{
		if (hdr->sections[i].offset.get() != pos)
		{
			validLayout = false;
		}
		pos += hdr->sections[i].size.get();
	}

	return validLayout;
}

void RomfsProcess::importDirectory(uint32_t dir_offset, sDirectory& dir)
{
	nx::sRomfsDirEntry* d_node = get_dir_node(dir_offset);

	/*
	printf("[DIR-NODE]\n");
	printf("  parent=%08x\n", d_node->parent.get());
	printf("  sibling=%08x\n", d_node->sibling.get());
	printf("  child=%08x\n", d_node->child.get());
	printf("  file=%08x\n", d_node->file.get());
	printf("  hash=%08x\n", d_node->hash.get());
	printf("  name_size=%08x\n", d_node->name_size.get());
	printf("  name=%s\n", d_node->name);
	*/

	for (uint32_t file_addr = d_node->file.get(); file_addr != nx::romfs::kInvalidAddr; )
	{
		nx::sRomfsFileEntry* f_node = get_file_node(file_addr);

		/*
		printf("[FILE-NODE]\n");
		printf("  parent=%08x\n", f_node->parent.get());
		printf("  sibling=%08x\n", f_node->sibling.get());
		printf("  offset=%08" PRIx64 "\n", f_node->offset.get());
		printf("  size=%08" PRIx64 "\n", f_node->size.get());
		printf("  hash=%08x\n", f_node->hash.get());
		printf("  name_size=%08x\n", f_node->name_size.get());
		printf("  name=%s\n", f_node->name);
		*/

		dir.file_list.addElement({std::string(f_node->name(), f_node->name_size.get()), mHdr.data_offset.get() + f_node->offset.get(), f_node->size.get()});

		file_addr = f_node->sibling.get();
		mFileNum++;
	}

	for (uint32_t child_addr = d_node->child.get(); child_addr != nx::romfs::kInvalidAddr; )
	{
		nx::sRomfsDirEntry* c_node = get_dir_node(child_addr);

		dir.dir_list.addElement({std::string(c_node->name(), c_node->name_size.get())});
		importDirectory(child_addr, dir.dir_list.atBack());

		child_addr = c_node->sibling.get();
		mDirNum++;
	}
}

void RomfsProcess::resolveRomfs()
{
	// read header
	mReader->read((byte_t*)&mHdr, 0, sizeof(nx::sRomfsHeader));

	// logic check on the header layout
	if (validateHeaderLayout(&mHdr) == false)
	{
		throw fnd::Exception(kModuleName, "Invalid ROMFS Header");
	}

	// read directory nodes
	mDirNodes.alloc(mHdr.sections[nx::romfs::DIR_NODE_TABLE].size.get());
	mReader->read(mDirNodes.getBytes(), mHdr.sections[nx::romfs::DIR_NODE_TABLE].offset.get(), mDirNodes.getSize());
	//printf("[RAW DIR NODES]\n");
	//fnd::SimpleTextOutput::hxdStyleDump(mDirNodes.getBytes(), mDirNodes.getSize());

	// read file nodes
	mFileNodes.alloc(mHdr.sections[nx::romfs::FILE_NODE_TABLE].size.get());
	mReader->read(mFileNodes.getBytes(), mHdr.sections[nx::romfs::FILE_NODE_TABLE].offset.get(), mFileNodes.getSize());
	//printf("[RAW FILE NODES]\n");
	//fnd::SimpleTextOutput::hxdStyleDump(mFileNodes.getBytes(), mFileNodes.getSize());
	
	// A logic check on the root directory node
	if (	get_dir_node(0)->parent.get() != 0 \
		|| 	get_dir_node(0)->sibling.get() != nx::romfs::kInvalidAddr \
		|| 	get_dir_node(0)->hash.get() != nx::romfs::kInvalidAddr \
		|| 	get_dir_node(0)->name_size.get() != 0)
	{
		throw fnd::Exception(kModuleName, "Invalid root directory node");
	}

	// import directory into internal structure
	mDirNum = 0;
	mFileNum = 0;
	importDirectory(0, mRootDir);
}