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

	void setInputFileSystem(const std::shared_ptr<tc::io::IStorage>& input_fs);
	void setFsLabel(const std::string& fs_label);
	void setCliOutputMode(bool show_fs);
	void setExtractPath(const tc::io::Path& extract_path);

	void process();
private:
	std::string mModuleLabel;

	std::shared_ptr<tc::io::IStorage> mInputFs;
	tc::Optional<std::string> mFsLabel;
	bool mShowFs;
	tc::Optional<tc::io::Path> mExtractPath;
	
	void printFs();
	void extractFs();

	void visitDir(const tc::io::Path& v_path, const tc::io::Path& l_path, bool extract_fs, bool print_fs);
};

}