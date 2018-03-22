#pragma once
#include <string>
#include <fnd/MemoryBlob.h>
#include <fnd/ISerialiseableBinary.h>

namespace es
{
	class ETicketSectionHeader_V2 :
		public fnd::ISerialiseableBinary
	{
	public:
		enum SectionType
		{
			PERMANENT = 1,
			SUBSCRIPTION = 2,
			CONTENT = 3,
			CONTENT_CONSUMPTION = 4,
			ACCESS_TITLE = 5,
			LIMITED_RESOURCE = 6,
		};

		ETicketSectionHeader_V2();
		ETicketSectionHeader_V2(const ETicketSectionHeader_V2& other);
		ETicketSectionHeader_V2(const byte_t* bytes, size_t len);

		bool operator==(const ETicketSectionHeader_V2& other) const;
		bool operator!=(const ETicketSectionHeader_V2& other) const;
		void operator=(const ETicketSectionHeader_V2& other);

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

		SectionType getSectionType() const;
		void setSectionType(SectionType type);

	private:
		const std::string kModuleName = "ETICKET_SECTION_HEADER_V2";
#pragma pack (push, 1)
		struct sSectionHeader_v2
		{
		private:
			uint32_t section_offset_;
			uint32_t record_size_;
			uint32_t section_size_;
			uint16_t record_num_;
			uint16_t section_type_;
		public:
			uint32_t section_offset() const { return le_word(section_offset_); }
			void set_section_offset(uint32_t offset) { section_offset_ = le_word(offset); }

			uint32_t record_size() const { return le_word(record_size_); }
			void set_record_size(uint32_t size) { record_size_ = le_word(size); }

			uint32_t section_size() const { return le_word(section_size_); }
			void set_section_size(uint32_t size) { section_size_ = le_word(size); }

			uint16_t record_num() const { return le_hword(record_num_); }
			void set_record_num(uint16_t num) { record_num_ = le_hword(num); }

			uint16_t section_type() const { return le_hword(section_type_); }
			void set_section_type(uint16_t type) { section_type_ = le_hword(type); }
		};
#pragma pack (pop)

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		uint32_t mSectionOffset;
		uint32_t mRecordSize;
		uint32_t mSectionSize;
		uint16_t mRecordNum;
		SectionType mSectionType;

		// helpers
		bool isEqual(const ETicketSectionHeader_V2& other) const;
		void copyFrom(const ETicketSectionHeader_V2& other);

	};

}
