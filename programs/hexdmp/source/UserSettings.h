#pragma once
#include <vector>
#include <string>
#include <fnd/types.h>
#include <fnd/Vec.h>
#include <fnd/List.h>
#include <nn/hac/npdm.h>
#include "common.h"

class UserSettings
{
public:
	UserSettings();

	void parseCmdArgs(const std::vector<std::string>& arg_list);
	void showHelp();

	// generic options
	const std::string getInputPath() const;
	FileType getFileType() const;
	CliOutputMode getCliOutputMode() const;
	
	// toggles
	bool isShowAsciiRepresentation() const;
	bool isShowOffset() const;
	
	// parameters
	const sOptional<size_t>& getByteGroupingSize() const;
	const sOptional<size_t>& getReadOffset() const;
	const sOptional<size_t>& getReadSize() const;

private:
	const std::string kModuleName = "UserSettings";
	
	
	struct sCmdArgs
	{
		sCmdArgs() {}
		sOptional<std::string> input_path;
		sOptional<std::string> file_type;
		sOptional<bool> show_ascii;
		sOptional<bool> show_offset;
		sOptional<bool> verbose;
		sOptional<std::string> byte_grouping;
		sOptional<std::string> read_offset;
		sOptional<std::string> read_size;
	};
	
	std::string mInputPath;
	FileType mFileType;
	CliOutputMode mOutputMode;

	bool mShowAsciiRepresentation;
	bool mShowOffset;
	sOptional<size_t> mByteGroupingSize;
	sOptional<size_t> mReadOffset;
	sOptional<size_t> mReadSize;

	void populateCmdArgs(const std::vector<std::string>& arg_list, sCmdArgs& cmd_args);
	void populateUserSettings(sCmdArgs& args);
	FileType getFileTypeFromString(const std::string& type_str);
};