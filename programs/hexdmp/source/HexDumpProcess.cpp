#include "HexDumpProcess.h"
#include <iomanip>
#include <iostream>
#include <cctype>

HexDumpProcess::HexDumpProcess():
	mFile(),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mShowAsciiRepresentation(true),
	mShowOffset(true)
{

}

void HexDumpProcess::process()
{
	if (*mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	mCache.alloc(kCacheSize);

	size_t offset, size, byte_grouping, row_len, row_num;

	if (mReadOffset.isSet)
		offset = *mReadOffset;
	else
		offset = 0;
	
	if (mReadSize.isSet)
		size = *mReadSize;
	else
		size = _MIN(kDefaultReadLen, (*mFile)->size());

	if (mByteGroupingSize.isSet)
		byte_grouping = *mByteGroupingSize;
	else
		byte_grouping = kDefaultByteGrouping;

	row_len = kDefaultRowLen;

	row_num = size / row_len + ((size % row_len) != 0);

	(*mFile)->read(mCache.data(), mCache.size());

	for (size_t i = 0; i < row_num; i++)
	{
		size_t print_len = _MIN(row_len, size - row_len*i);
		
		printRow(mCache.data() + (i * row_len), offset + (i * row_len), row_len, print_len, byte_grouping);
	}
	
}

void HexDumpProcess::setInputFile(const fnd::SharedPtr<fnd::IFile>& file)
{
	mFile = file;
}

void HexDumpProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void HexDumpProcess::setShowAsciiRepresentation(bool enable)
{
	mShowAsciiRepresentation = enable;
}

void HexDumpProcess::setShowOffset(bool enable)
{
	mShowOffset = enable;
}

void HexDumpProcess::setByteGroupingSize(const sOptional<size_t>& var)
{
	mByteGroupingSize = var;
}

void HexDumpProcess::setReadOffset(const sOptional<size_t>& var)
{
	mReadOffset = var;
}

void HexDumpProcess::setReadSize(const sOptional<size_t>& var)
{
	mReadSize = var;
}

void HexDumpProcess::printRow(const byte_t* data_row, size_t offset, size_t row_len, size_t print_len, size_t byte_grouping) const
{
	if (mShowOffset)
	{
		std::cout << std::hex << std::setw(8) << std::setfill('0') << offset << " ";
		std::cout << "| ";
	}

	for (size_t i = 0; i < row_len; i++)
	{
		if (i < print_len)
			std::cout << std::hex << std::setw(2) << std::setfill('0') << (uint32_t)data_row[i];
		else
			std::cout << "  ";

		if ((i+1) % byte_grouping == 0 || i+1 == row_len)
			std::cout << " ";
	}
	
	if (mShowAsciiRepresentation)
	{
		std::cout << "| ";
		for (size_t i = 0; i < row_len; i++)
		{
			if (i < print_len)
			{
				if (isprint((char)data_row[i]))
					std::cout << (char)data_row[i];
				else
					std::cout << ".";
			}
			else
				std::cout << " ";
		}
	}

	std::cout << std::endl;
}