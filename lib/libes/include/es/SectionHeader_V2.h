#pragma once
#include <string>
#include <fnd/MemoryBlob.h>
#include <fnd/ISerialiseableBinary.h>
#include <es/ticket.h>

namespace es
{
	class SectionHeader_V2 :
		public fnd::ISerialiseableBinary
	{
	public:
		SectionHeader_V2();
		SectionHeader_V2(const SectionHeader_V2& other);
		SectionHeader_V2(const byte_t* bytes, size_t len);

		bool operator==(const SectionHeader_V2& other) const;
		bool operator!=(const SectionHeader_V2& other) const;
		void operator=(const SectionHeader_V2& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		virtual void exportBinary();
		virtual void importBinary(const byte_t* bytes, size_t len);

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
		fnd::MemoryBlob mBinaryBlob;

		// variables
		uint32_t mSectionOffset;
		uint32_t mRecordSize;
		uint32_t mSectionSize;
		uint16_t mRecordNum;
		ticket::SectionType mSectionType;

		// helpers
		bool isEqual(const SectionHeader_V2& other) const;
		void copyFrom(const SectionHeader_V2& other);

	};

}
