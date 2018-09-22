/*
CodeBinaryHeader.h
(c) 2018 Jakcron

This is a header to prepend to raw EXEFS .code binaries that provide enough data to be equivalent to an ELF.
*/

#pragma once
#include <string>

#include <fnd/ISerialisable.h>
#include <fnd/List.h>
#include <nn/ctr/code.h>
#include <nn/ctr/CodeSegment.h>


namespace nn
{
	namespace ctr
	{
		class CodeBinaryHeader : public fnd::ISerialisable
		{
		public:
			// constructors
			CodeBinaryHeader();
			CodeBinaryHeader(const CodeBinaryHeader& other);

			void operator=(const CodeBinaryHeader& other);
			bool operator==(const CodeBinaryHeader& other) const;
			bool operator!=(const CodeBinaryHeader& other) const;

			// export/import binary
			void toBytes();
			void fromBytes(const byte_t* data, size_t len);
			const fnd::Vec<byte_t>& getBytes() const;

			// variables
			void clear();

			// mutators
			void setCodeBinOffset(uint32_t offset);
			uint32_t getCodeBinOffset() const;

			void setCodeBinSize(uint32_t size);
			uint32_t getCodeBinSize() const;

			void setStackSize(uint32_t size);
			uint32_t getStackSize() const;

			void setBssSize(uint32_t size);
			uint32_t getBssSize() const;

			void setTextSegment(const ctr::CodeSegment& segment);
			const ctr::CodeSegment& getTextSegment() const;

			void setRODataSegment(const ctr::CodeSegment& segment);
			const ctr::CodeSegment& getRODataSegment() const;

			void setDataSegment(const ctr::CodeSegment& segment);
			const ctr::CodeSegment& getDataSegment() const;

		private:
			const std::string kModuleName = "CODE_BINARY_HEADER";


			// serialised data
			fnd::Vec<byte_t> mRawBinary;

			// members
			uint32_t mCodeBinOffset;
			uint32_t mCodeBinSize;
			uint32_t mStackSize;
			uint32_t mBssSize;
			CodeSegment mSegmentList[code::kCodeSegmentNum];

		};
	}
}
