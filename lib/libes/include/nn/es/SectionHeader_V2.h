#pragma once
#include <string>
#include <fnd/ISerialisable.h>
#include <nn/es/ticket.h>

namespace nn
{
namespace es
{
	class SectionHeader_V2 :
		public fnd::ISerialisable
	{
	public:
		SectionHeader_V2();
		SectionHeader_V2(const SectionHeader_V2& other);

		void operator=(const SectionHeader_V2& other);
		bool operator==(const SectionHeader_V2& other) const;
		bool operator!=(const SectionHeader_V2& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* data, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		virtual void clear();

		uint32_t getSectionOffset() const;
		void setSectionOffset(uint32_t offset);

		uint32_t getRecordSize() const;
		void setRecordSize(uint32_t size);

		uint32_t getSectionSize() const;
		void getSectionSize(uint32_t size);

		uint16_t getRecordNum() const;
		void setRecordNum(uint16_t record_num);

		ticket::SectionType getSectionType() const;
		void setSectionType(ticket::SectionType type);

	private:
		const std::string kModuleName = "SECTION_HEADER_V2";
		
		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		uint32_t mSectionOffset;
		uint32_t mRecordSize;
		uint32_t mSectionSize;
		uint16_t mRecordNum;
		ticket::SectionType mSectionType;
	};
}
}