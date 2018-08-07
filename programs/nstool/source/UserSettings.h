#pragma once
#include <vector>
#include <string>
#include <fnd/types.h>
#include <fnd/Vec.h>
#include <fnd/List.h>
#include <nn/pki/SignedData.h>
#include <nn/pki/CertificateBody.h>
#include <nn/hac/npdm.h>
#include "nstool.h"

class UserSettings
{
public:
	UserSettings();

	void parseCmdArgs(const std::vector<std::string>& arg_list);
	void showHelp();

	// generic options
	const std::string getInputPath() const;
	const sKeyset& getKeyset() const;
	FileType getFileType() const;
	bool isVerifyFile() const;
	CliOutputMode getCliOutputMode() const;
	
	// specialised toggles
	bool isListFs() const;
	bool isListApi() const;
	bool isListSymbols() const;
	nn::hac::npdm::InstructionType getInstType() const;

	// specialised paths
	const sOptional<std::string>& getXciUpdatePath() const;
	const sOptional<std::string>& getXciLogoPath() const;
	const sOptional<std::string>& getXciNormalPath() const;
	const sOptional<std::string>& getXciSecurePath() const;
	const sOptional<std::string>& getFsPath() const;
	const sOptional<std::string>& getNcaPart0Path() const;
	const sOptional<std::string>& getNcaPart1Path() const;
	const sOptional<std::string>& getNcaPart2Path() const;
	const sOptional<std::string>& getNcaPart3Path() const;
	const sOptional<std::string>& getAssetIconPath() const;
	const sOptional<std::string>& getAssetNacpPath() const;
	const fnd::List<nn::pki::SignedData<nn::pki::CertificateBody>>& getCertificateChain() const;

private:
	const std::string kModuleName = "UserSettings";
	
	struct sCmdArgs
	{
		sCmdArgs() {}
		sOptional<std::string> input_path;
		sOptional<bool> devkit_keys;
		sOptional<std::string> keyset_path;
		sOptional<std::string> file_type;
		sOptional<bool> verify_file;
		sOptional<bool> show_keys;
		sOptional<bool> show_layout;
		sOptional<bool> verbose_output;
		sOptional<bool> list_fs;
		sOptional<std::string> update_path;
		sOptional<std::string> logo_path;
		sOptional<std::string> normal_path;
		sOptional<std::string> secure_path;
		sOptional<std::string> fs_path;
		sOptional<std::string> nca_titlekey;
		sOptional<std::string> nca_bodykey;
		sOptional<std::string> ticket_path;
		sOptional<std::string> cert_path;
		sOptional<std::string> part0_path;
		sOptional<std::string> part1_path;
		sOptional<std::string> part2_path;
		sOptional<std::string> part3_path;
		sOptional<bool> list_api;
		sOptional<bool> list_sym;
		sOptional<std::string> inst_type;
		sOptional<std::string> asset_icon_path;
		sOptional<std::string> asset_nacp_path;
	};
	
	std::string mInputPath;
	FileType mFileType;
	sKeyset mKeyset;
	bool mVerifyFile;
	CliOutputMode mOutputMode;

	bool mListFs;
	sOptional<std::string> mXciUpdatePath;
	sOptional<std::string> mXciLogoPath;
	sOptional<std::string> mXciNormalPath;
	sOptional<std::string> mXciSecurePath;
	sOptional<std::string> mFsPath;

	sOptional<std::string> mNcaPart0Path;
	sOptional<std::string> mNcaPart1Path;
	sOptional<std::string> mNcaPart2Path;
	sOptional<std::string> mNcaPart3Path;

	sOptional<std::string> mAssetIconPath;
	sOptional<std::string> mAssetNacpPath;

	fnd::List<nn::pki::SignedData<nn::pki::CertificateBody>> mCertChain;

	bool mListApi;
	bool mListSymbols;
	nn::hac::npdm::InstructionType mInstructionType;

	void populateCmdArgs(const std::vector<std::string>& arg_list, sCmdArgs& cmd_args);
	void populateKeyset(sCmdArgs& args);
	void populateUserSettings(sCmdArgs& args);
	void decodeHexStringToBytes(const std::string& name, const std::string& str, byte_t* out, size_t out_len);
	FileType getFileTypeFromString(const std::string& type_str);
	FileType determineFileTypeFromFile(const std::string& path);
	bool determineValidNcaFromSample(const fnd::Vec<byte_t>& sample) const;
	bool determineValidCnmtFromSample(const fnd::Vec<byte_t>& sample) const;
	bool determineValidNacpFromSample(const fnd::Vec<byte_t>& sample) const;
	bool determineValidEsCertFromSample(const fnd::Vec<byte_t>& sample) const;
	bool determineValidEsTikFromSample(const fnd::Vec<byte_t>& sample) const;
	nn::hac::npdm::InstructionType getInstructionTypeFromString(const std::string& type_str);
};