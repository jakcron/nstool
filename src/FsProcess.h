#pragma once
#include <tc/Optional.h>
#include <tc/io.h>

#include "types.h"

namespace nstool
{

class FsProcess
{
public:
	FsProcess();

	void process();

	void setInputFileSystem(const std::shared_ptr<tc::io::IFileSystem>& input_fs);
	void setFsFormatName(const std::string& fs_format_name);
	void setFsProperties(const std::vector<std::string>& properties);
	void setShowFsInfo(bool show_fs_info);
	void setShowFsTree(bool show_fs_tree);
	void setFsRootLabel(const std::string& root_label);
	void setExtractJobs(const std::vector<nstool::ExtractJob>& extract_jobs);
private:
	std::string mModuleLabel;

	std::shared_ptr<tc::io::IFileSystem> mInputFs;

	// fs info
	tc::Optional<std::string> mFsFormatName;
	bool mShowFsInfo;
	std::vector<std::string> mProperties;

	// fs tree
	bool mShowFsTree;
	tc::Optional<std::string> mFsRootLabel;

	// extract jobs
	std::vector<nstool::ExtractJob> mExtractJobs;

	// cache for file extract
	tc::ByteData mDataCache;
	
	void printFs();
	void extractFs();

	void visitDir(const tc::io::Path& v_path, const tc::io::Path& l_path, bool extract_fs, bool print_fs);
};

}