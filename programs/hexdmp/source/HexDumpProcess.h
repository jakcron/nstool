#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/IFile.h>
#include <fnd/Vec.h>
#include <fnd/SharedPtr.h>

#include "common.h"

class HexDumpProcess
{
public:
	HexDumpProcess();

	void process();

	void setInputFile(const fnd::SharedPtr<fnd::IFile>& file);
	void setCliOutputMode(CliOutputMode type);

	void setShowAsciiRepresentation(bool enable);
	void setShowOffset(bool enable);
	void setByteGroupingSize(const sOptional<size_t>& var);
	void setReadOffset(const sOptional<size_t>& var);
	void setReadSize(const sOptional<size_t>& var);

private:
	const std::string kModuleName = "HexDumpProcess";
	static const size_t kDefaultRowLen = 0x10;
	static const size_t kDefaultReadLen = 0x1000;
	static const size_t kDefaultByteGrouping = 0x1;
	static const size_t kCacheSize = 0x40000;

	fnd::SharedPtr<fnd::IFile> mFile;
	CliOutputMode mCliOutputMode;

	bool mShowAsciiRepresentation;
	bool mShowOffset;
	sOptional<size_t> mByteGroupingSize;
	sOptional<size_t> mReadOffset;
	sOptional<size_t> mReadSize;

	fnd::Vec<byte_t> mCache;

	void printRow(const byte_t* data_row, size_t offset, size_t row_len, size_t print_len, size_t byte_grouping) const;
};