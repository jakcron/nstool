#pragma once
#include <string>
#include <fnd/types.h>
#include <nx/npdm.h>
#include "nstool.h"

class UserSettings
{
public:
	UserSettings();

	void parseCmdArgs(int argc, char** argv);
	void showHelp();

	// generic options
	const std::string getInputPath() const;
	const sKeyset& getKeyset() const;
	FileType getFileType() const;
	bool isVerifyFile() const;
	CliOutputType getCliOutputType() const;
	
	// specialised toggles
	bool isListFs() const;
	const sOptional<nx::npdm::InstructionType>& getArchType() const;

	// specialised paths
	const sOptional<std::string>& getUpdatePath() const;
	const sOptional<std::string>& getNormalPath() const;
	const sOptional<std::string>& getSecurePath() const;
	const sOptional<std::string>& getFsPath() const;
	const sOptional<std::string>& getPart0Path() const;
	const sOptional<std::string>& getPart1Path() const;
	const sOptional<std::string>& getPart2Path() const;
	const sOptional<std::string>& getPart3Path() const;

private:
	const std::string kModuleName = "UserSettings";
	
	struct sCmdArgs
	{
		sOptional<std::string> input_path;
		sOptional<bool> devkit_keys;
		sOptional<std::string> keyset_path;
		sOptional<std::string> file_type;
		sOptional<bool> verify_file;
		sOptional<bool> verbose_output;
		sOptional<bool> minimal_output;
		sOptional<bool> list_fs;
		sOptional<std::string> update_path;
		sOptional<std::string> normal_path;
		sOptional<std::string> secure_path;
		sOptional<std::string> fs_path;
		sOptional<std::string> nca_titlekey;
		sOptional<std::string> nca_bodykey;
		sOptional<std::string> part0_path;
		sOptional<std::string> part1_path;
		sOptional<std::string> part2_path;
		sOptional<std::string> part3_path;
		sOptional<std::string> arch_type;

		void clear()
		{
			input_path.isSet = false;
			devkit_keys.isSet = false;
			keyset_path.isSet = false;
			file_type.isSet = false;
			verify_file.isSet = false;
			verbose_output.isSet = false;
			minimal_output.isSet = false;
			list_fs.isSet = false;
			update_path.isSet = false;
			normal_path.isSet = false;
			secure_path.isSet = false;
			fs_path.isSet = false;
			nca_titlekey.isSet = false;
			nca_bodykey.isSet = false;
			part0_path.isSet = false;
			part1_path.isSet = false;
			part2_path.isSet = false;
			part3_path.isSet = false;
			arch_type.isSet = false;
		}
	};
	
	std::string mInputPath;
	FileType mFileType;
	sKeyset mKeyset;
	bool mVerifyFile;
	CliOutputType mOutputType;

	bool mListFs;
	sOptional<std::string> mUpdatePath;
	sOptional<std::string> mNormalPath;
	sOptional<std::string> mSecurePath;
	sOptional<std::string> mFsPath;

	sOptional<std::string> mPart0Path;
	sOptional<std::string> mPart1Path;
	sOptional<std::string> mPart2Path;
	sOptional<std::string> mPart3Path;

	sOptional<nx::npdm::InstructionType> mArchType;

	void populateCmdArgs(int argc, char** argv, sCmdArgs& cmd_args);
	void populateKeyset(sCmdArgs& args);
	void populateUserSettings(sCmdArgs& args);
	void decodeHexStringToBytes(const std::string& name, const std::string& str, byte_t* out, size_t out_len);
	FileType getFileTypeFromString(const std::string& type_str);
	FileType determineFileTypeFromFile(const std::string& path);
	nx::npdm::InstructionType getInstructionTypeFromString(const std::string& type_str);
};