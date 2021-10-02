#include "FsProcess.h"
#include <iostream>

nstool::FsProcess::FsProcess() :
	mModuleLabel("nstool::FsProcess"),
	mInputFs(),
	mShowFs(false),
	mExtractPath()
{

}

void nstool::FsProcess::setInputFileSystem(const std::shared_ptr<tc::io::IStorage>& input_fs)
{
	mInputFs = input_fs;
}

void nstool::FsProcess::setFsLabel(const std::string& fs_label)
{
	mFsLabel = fs_label;
}

void nstool::FsProcess::setCliOutputMode(bool show_fs)
{
	mShowFs = show_fs;
}

void nstool::FsProcess::setExtractPath(const tc::io::Path& extract_path)
{
	mExtractPath = extract_path;
}

void nstool::FsProcess::process()
{
	if (mInputFs == nullptr)
	{
		throw tc::InvalidOperationException(mModuleLabel, "No input filesystem");
	}

	if (mShowFs)
		printFs();

	if (mExtractPath.isSet())
		extractFs();
}

void nstool::FsProcess::printFs()
{
	fmt::print("[{:s}FsTree]\n", (mFsLabel.isSet() ? (mFsLabel.get() + "/") : ""));
	visitDir(tc::io::Path("/"), tc::io::Path("/"), false, true);
}

void nstool::FsProcess::extractFs()
{
	fmt::print("[{:s}FsExtract]\n", (mFsLabel.isSet() ? (mFsLabel.get() + "/") : ""));
	visitDir(tc::io::Path("/"), mExtractPath.get(), true, false);
}

void nstool::FsProcess::visitDir(const tc::io::Path& v_path, const tc::io::Path& l_path, bool extract_fs, bool print_fs)
{
	tc::io::LocalStorage local_fs;

	// get listing for directory
	tc::io::sDirectoryListing info;
	mInputFs->getDirectoryListing(v_path, info);

	if (print_fs)
	{
		for (size_t i = 0; i < v_path.size(); i++)
			fmt::print(" ");;

		fmt::print("{:s}/\n", ((v_path.size() == 1) ? "Root:" : v_path.back()));
	}
	if (extract_fs)
	{
		// create local dir
		local_fs.createDirectory(l_path);
	}

	// iterate thru child files
	tc::ByteData cache = tc::ByteData(0x10000);
	size_t cache_read_len;
	tc::io::Path out_path;
	std::string out_path_str;
	std::shared_ptr<tc::io::IStream> in_stream;
	std::shared_ptr<tc::io::IStream> out_stream;
	for (auto itr = info.file_list.begin(); itr != info.file_list.end(); itr++)
	{
		if (print_fs)
		{
			for (size_t i = 0; i < v_path.size(); i++)
				fmt::print(" ");
			fmt::print(" {:s}\n", *itr);
		}
		if (extract_fs)
		{
			// build out path
			out_path = l_path + *itr;
			tc::io::PathUtil::pathToUnixUTF8(out_path, out_path_str);

			fmt::print("Saving {:s}...\n", out_path_str);

			// begin export
			mInputFs->openFile(v_path + *itr, tc::io::FileMode::Open, tc::io::FileAccess::Read, in_stream);
			local_fs.openFile(out_path, tc::io::FileMode::OpenOrCreate, tc::io::FileAccess::Write, out_stream);

			in_stream->seek(0, tc::io::SeekOrigin::Begin);
			out_stream->seek(0, tc::io::SeekOrigin::Begin);
			for (int64_t remaining_data = in_stream->length(); remaining_data > 0;)
			{
				cache_read_len = in_stream->read(cache.data(), cache.size());
				if (cache_read_len == 0)
				{
					throw tc::io::IOException(mModuleLabel, fmt::format("Failed to read from {:s}file.", (mFsLabel.isSet() ? (mFsLabel.get() + " ") : "")));
				}

				out_stream->write(cache.data(), cache_read_len);

				remaining_data -= int64_t(cache_read_len);
			}
		}
	}

	// iterate thru child dirs
	for (auto itr = info.dir_list.begin(); itr != info.dir_list.end(); itr++)
	{
		visitDir(v_path + *itr, l_path + *itr, extract_fs, print_fs);
	}
}