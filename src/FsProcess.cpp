#include "FsProcess.h"
#include "util.h"

#include <memory>
#include <tc/io/FileNotFoundException.h>
#include <tc/io/DirectoryNotFoundException.h>

nstool::FsProcess::FsProcess() :
	mModuleLabel("nstool::FsProcess"),
	mInputFs(),
	mFsFormatName(),
	mShowFsInfo(false),
	mProperties(),
	mShowFsTree(false),
	mFsRootLabel(),
	mExtractJobs(),
	mArchiveJobs(),
	mDataCache(0x10000)
{

}

void nstool::FsProcess::process()
{
	if (mInputFs == nullptr)
	{
		throw tc::InvalidOperationException(mModuleLabel, "No input filesystem");
	}

	processArchiveJobs();

	if (mShowFsInfo)
	{
		fmt::print("[{:s}]\n", mFsFormatName.isSet() ? mFsFormatName.get() : "FileSystem/Info");
		for (auto itr = mProperties.begin(); itr != mProperties.end(); itr++)
		{
			fmt::print("  {:s}\n", *itr);
		}
	}

	if (mShowFsTree)
	{
		printFs();
	}
	
	if (mExtractJobs.empty() == false)
	{
		extractFs();
	}
}

void nstool::FsProcess::setInputFileSystem(const std::shared_ptr<tc::io::IFileSystem>& input_fs)
{
	mInputFs = input_fs;
}

void nstool::FsProcess::setFsFormatName(const std::string& fs_format_name)
{
	mFsFormatName = fs_format_name;
}

void nstool::FsProcess::setShowFsInfo(bool show_fs_info)
{
	mShowFsInfo = show_fs_info;
}

void nstool::FsProcess::setFsProperties(const std::vector<std::string>& properties)
{
	mProperties = properties;
}

void nstool::FsProcess::setShowFsTree(bool show_fs_tree)
{
	mShowFsTree = show_fs_tree;
}

void nstool::FsProcess::setFsRootLabel(const std::string& root_label)
{
	mFsRootLabel = root_label;
}

void nstool::FsProcess::setExtractJobs(const std::vector<nstool::ExtractJob>& extract_jobs)
{
	mExtractJobs = extract_jobs;
}

void nstool::FsProcess::setArchiveJobs(const std::vector<nstool::ArchiveJob>& archive_jobs)
{
	mArchiveJobs = archive_jobs;
}

void nstool::FsProcess::processArchiveJobs()
{
	for (auto itr = mArchiveJobs.begin(); itr != mArchiveJobs.end(); itr++)
	{
		/*
		std::string archive_path_str, extract_base_path_str;
		tc::io::PathUtil::pathToUnixUTF8(itr->archive_path, archive_path_str);
		tc::io::PathUtil::pathToUnixUTF8(itr->extract_base_path, extract_base_path_str);
		*/

		// Qualify Archive Path
		tc::io::Path qualified_archive_path;
		bool is_dir = false, is_file = false;
		if (!qualifyArchivePath(itr->archive_path, qualified_archive_path, is_dir, is_file))
		{
			std::string archive_path_str;
			tc::io::PathUtil::pathToUnixUTF8(itr->archive_path, archive_path_str);
			fmt::print("Path \"{}\" did not exist for this archive.\n", archive_path_str);
			
			continue;
		}

		// print
		{
			std::string qualified_archive_path_str;
			tc::io::PathUtil::pathToUnixUTF8(qualified_archive_path, qualified_archive_path_str);
			fmt::print("Path \"{}\" was valid, is_dir={}, is_file={}.\n", qualified_archive_path_str, is_dir, is_file);
		}
		

		if (itr->job_action == ArchiveJob::JobAction::DoNothing)
		{
			fmt::print("DoNothing\n");
			continue;
		}
		else if (itr->job_action == ArchiveJob::JobAction::ListFileTree)
		{
			fmt::print("ListFileTree\n");


		}
		else if (itr->job_action == ArchiveJob::JobAction::Extract)
		{
			fmt::print("Extract\n");
		}
		else
		{
			fmt::print("Unsupported ArchiveJob::JobAction\n");
		}

	}
}

bool nstool::FsProcess::qualifyArchivePath(const tc::io::Path& path, tc::io::Path& qualified_path, bool& is_dir, bool& is_file)
{
	// test if the path is for a directory
	try {
		tc::io::sDirectoryListing dir_listing;

		// getDirectoryListing will throw if this is not a directory
		mInputFs->getDirectoryListing(path, dir_listing);

		// set state and return
		qualified_path = dir_listing.abs_path;
		is_dir = true;
		is_file = false;
		return true;
	} catch (tc::io::DirectoryNotFoundException&) {
		// acceptable exception, just means directory didn't exist
	}

	// test if the path is for a file
	try {
		// opening the file will throw if the file doesn't exist
		std::shared_ptr<tc::io::IStream> file_stream;
		mInputFs->openFile(path, tc::io::FileMode::Open, tc::io::FileAccess::Read, file_stream);

		// breakup the path into the parent directory path and file name
		tc::io::Path parent_dir_path = path;
		parent_dir_path.pop_back(); // remove the last element, which should be the file name (which should exist because opening the file worked)

		std::string file_name = path.back();

		// get the qualified file path
		tc::io::sDirectoryListing dir_listing;

		// getDirectoryListing will throw if this is not a directory
		mInputFs->getDirectoryListing(parent_dir_path, dir_listing);

		// set state
		qualified_path = dir_listing.abs_path + file_name;
		is_dir = false;
		is_file = true;
		return true;
	} catch (tc::io::FileNotFoundException&) {
		// acceptable exception, just means file didn't exist
	}

	qualified_path = tc::io::Path();
	is_dir = false;
	is_file = false;
	return false;
}

void nstool::FsProcess::printFs()
{
	fmt::print("[{:s}/Tree]\n", (mFsFormatName.isSet() ? mFsFormatName.get() : "FileSystem"));
	visitDir(tc::io::Path("/"), tc::io::Path("/"), false, true);
}

void nstool::FsProcess::extractFs()
{
	fmt::print("[{:s}/Extract]\n", (mFsFormatName.isSet() ? mFsFormatName.get() : "FileSystem"));

	for (auto itr = mExtractJobs.begin(); itr != mExtractJobs.end(); itr++)
	{
		std::string path_str;
		tc::io::PathUtil::pathToUnixUTF8(itr->virtual_path, path_str);

		// check if root path (legacy case)
		if (itr->virtual_path == tc::io::Path("/"))
		{
			visitDir(tc::io::Path("/"), itr->extract_path, true, false);

			//fmt::print("Root Dir Virtual Path: \"{:s}\"\n", path_str);

			// root directory extract successful, continue to next job
			continue;
		}

		// otherwise determine if this is a file or subdirectory
		try {
			std::shared_ptr<tc::io::IStream> file_stream;
			mInputFs->openFile(itr->virtual_path, tc::io::FileMode::Open, tc::io::FileAccess::Read, file_stream);

			//fmt::print("Valid File Path: \"{:s}\"\n", path_str);

			// the output path for this file will depend on the user specified extract path
			std::shared_ptr<tc::io::IFileSystem> local_fs = std::make_shared<tc::io::LocalFileSystem>(tc::io::LocalFileSystem());

			// case: the extract_path is a valid path to an existing directory
			// behaviour: extract the file, preserving the original filename, to the specified directory
			// method: try getDirectoryListing(itr->extract_path), if this is does not throw, then we can be sure this is a valid path to a directory, file_extract_path = itr->extract_path + itr->virtual_path.back()

			try {
				tc::io::sDirectoryListing dir_listing;
				local_fs->getDirectoryListing(itr->extract_path, dir_listing);

				tc::io::Path file_extract_path = itr->extract_path + itr->virtual_path.back();

				std::string file_extract_path_str;
				tc::io::PathUtil::pathToUnixUTF8(file_extract_path, file_extract_path_str);

				fmt::print("Saving {:s}...\n", file_extract_path_str);

				writeStreamToFile(file_stream, itr->extract_path + itr->virtual_path.back(), mDataCache);

				continue;

			} catch (tc::io::DirectoryNotFoundException&) {
				// acceptable exception, just means directory didn't exist
			}

			// case: the extract_path up until the last element is a valid path to an existing directory, but the full path specifies neither a directory or a file
			// behaviour: treat extract_path as the intended location to write the extracted file (the original filename is not preserved, instead specified by the user in the final element of the extract path)
			// method: since this checks n-1 elements, it implies a path with more than one element, so that must be accounted for, as relative paths are valid and single element paths aren't always root

			try {
				std::string test_path_str;

				// get path to parent directory
				tc::io::Path parent_dir_path = itr->extract_path;

				// replace final path element with the current directory alias 
				parent_dir_path.pop_back(); // remove filename
				parent_dir_path.push_back("."); // replace with the current dir name alias
				tc::io::PathUtil::pathToUnixUTF8(parent_dir_path, test_path_str);

				// test parent directory exists
				tc::io::sDirectoryListing dir_listing;
				local_fs->getDirectoryListing(parent_dir_path, dir_listing);

				std::string file_extract_path_str;
				tc::io::PathUtil::pathToUnixUTF8(itr->extract_path, file_extract_path_str);

				fmt::print("Saving {:s} as {:s}...\n", path_str, file_extract_path_str);

				writeStreamToFile(file_stream, itr->extract_path, mDataCache);

				continue;
			} catch (tc::io::DirectoryNotFoundException&) {
				// acceptable exception, just means the parent directory didn't exist
			}


			// extract path could not be determined, inform the user and skip this job
			std::string literal_extract_path_str;
			tc::io::PathUtil::pathToUnixUTF8(itr->extract_path, literal_extract_path_str);
			fmt::print("[WARNING] Extract path was invalid, and was skipped: {:s}\n", literal_extract_path_str);
			continue;
		} catch (tc::io::FileNotFoundException&) {
			// acceptable exception, just means file didn't exist
		}

		// not a file, attempt to process this as a directory
		try {
			tc::io::sDirectoryListing dir_listing;
			mInputFs->getDirectoryListing(itr->virtual_path, dir_listing);


			visitDir(itr->virtual_path, itr->extract_path, true, false);

			//fmt::print("Valid Directory Path: \"{:s}\"\n", path_str);

			// directory extract successful, continue to next job
			continue;

		} catch (tc::io::DirectoryNotFoundException&) {
			// acceptable exception, just means directory didn't exist
		}

		fmt::print("[WARNING] Failed to extract virtual path: \"{:s}\"\n", path_str);
	}
	
}

void nstool::FsProcess::visitDir(const tc::io::Path& v_path, const tc::io::Path& l_path, bool extract_fs, bool print_fs)
{
	tc::io::LocalFileSystem local_fs;

	// get listing for directory
	tc::io::sDirectoryListing info;
	mInputFs->getDirectoryListing(v_path, info);

	if (print_fs)
	{
		for (size_t i = 0; i < v_path.size(); i++)
			fmt::print(" ");

		fmt::print("{:s}/\n", ((v_path.size() == 1) ? (mFsRootLabel.isSet() ? (mFsRootLabel.get() + ":")  : "Root:") : v_path.back()));
	}
	if (extract_fs)
	{
		// create local dir
		local_fs.createDirectory(l_path);
	}

	// iterate thru child files
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
				cache_read_len = in_stream->read(mDataCache.data(), mDataCache.size());
				if (cache_read_len == 0)
				{
					throw tc::io::IOException(mModuleLabel, fmt::format("Failed to read from {:s}file.", (mFsFormatName.isSet() ? (mFsFormatName.get() + " ") : "")));
				}

				out_stream->write(mDataCache.data(), cache_read_len);

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