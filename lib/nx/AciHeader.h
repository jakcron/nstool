#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/memory_blob.h>
#include <nx/ISerialiseableBinary.h>

namespace nx
{
	class AciHeader : public ISerialiseableBinary
	{
	public:
		enum AciType
		{
			TYPE_ACI0,	// for Access Control Info
			TYPE_ACID	// for Access Control Info Desc
		};

		struct sSection
		{
			size_t offset;
			size_t size;

			void operator=(const sSection& other)
			{
				offset = other.offset;
				size = other.size;
			}

			bool operator==(const sSection& other) const
			{
				return (offset == other.offset) \
					&& (size == other.size);
			}

			bool operator!=(const sSection& other) const
			{
				return !operator==(other);
			}
		};

		AciHeader();
		AciHeader(const AciHeader& other);
		AciHeader(const u8* bytes);

		bool operator==(const AciHeader& other) const;
		bool operator!=(const AciHeader& other) const;
		void operator=(const AciHeader& other);

		// to be used after export
		const u8* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const u8* bytes);
		void importBinary(const u8* bytes, size_t len);

		// variables
		AciType getAciType() const;
		void setAciType(AciType type);
		u64 getProgramId() const;
		void setProgramId(u64 program_id);
		const sSection& getFileAccessControl() const;
		void setFileAccessControl(u32 size);
		const sSection& getServiceAccessControl() const;
		void setServiceAccessControl(u32 size);
		const sSection& getKernelCapabilities() const;
		void setKernelCapabilities(u32 size);

	private:
		const std::string kModuleName = "ACI_HEADER";
		const std::string kAciStructSig = "ACI0";
		const std::string kAciDescStructSig = "ACID";
		static const size_t kAciAlignSize = 0x10;

#pragma pack(push, 1)
		struct sAciHeader
		{
		private:
			u8 signature_[4];
			u32 size_; // includes prefacing signature, set only in ACID since it is signed
			u8 reserved_1[8];
			u64 program_id_; // set only in ACI0 (since ACID is generic)
			u8 reserved_2[8];
			struct sAciSection
			{
			private:
				u32 offset_; // aligned by 0x10 from the last one
				u32 size_;
			public:
				u32 offset() const { return le_word(offset_); }
				void set_offset(u32 offset) { offset_ = le_word(offset); }

				u32 size() const { return le_word(size_); }
				void set_size(u32 size) { size_ = le_word(size); }
			} fac_, sac_, kc_;
			u8 reserved_3[8];
		public:
			const char* signature() const { return (const char*)signature_; }
			void set_signature(const char* signature) { memcpy(signature_, signature, 4); }

			u32 size() const { return le_word(size_); }
			void set_size(u32 size) { size_ = le_word(size); }

			u64 program_id() const { return le_dword(program_id_); }
			void set_program_id(u64 program_id) { program_id_ = le_dword(program_id); }

			const sAciSection& fac() const { return fac_; }
			sAciSection& fac() { return fac_; }

			const sAciSection& sac() const { return sac_; }
			sAciSection& sac() { return sac_; }

			const sAciSection& kc() const { return kc_; }
			sAciSection& kc() { return kc_; }
		};
#pragma pack(pop)

		// raw data
		fnd::MemoryBlob mBinaryBlob;

		// variables
		AciType mType;
		u64 mProgramId;
		sSection mFac, mSac, mKc;

		void clearVariables();
		void calculateSectionOffsets();
		bool isEqual(const AciHeader& other) const;
		void copyFrom(const AciHeader& other);
	};
}

