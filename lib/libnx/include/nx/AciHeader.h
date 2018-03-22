#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/MemoryBlob.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	class AciHeader :
		public fnd::ISerialiseableBinary
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
		AciHeader(const byte_t* bytes, size_t len);

		bool operator==(const AciHeader& other) const;
		bool operator!=(const AciHeader& other) const;
		void operator=(const AciHeader& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		virtual void exportBinary();
		virtual void importBinary(const byte_t* bytes, size_t len);

		// variables
		virtual void clear();
		size_t getAciSize() const;

		// ACI0 only
		uint64_t getProgramId() const;
		void setProgramId(uint64_t program_id);
		
		// ACID only
		size_t getAcidSize() const;
		//void setAcidSize(size_t size);
		uint64_t getProgramIdMin() const;
		void setProgramIdMin(uint64_t program_id);
		uint64_t getProgramIdMax() const;
		void setProgramIdMax(uint64_t program_id);

		// ACID & ACI0
		void setHeaderOffset(size_t offset);
		AciType getAciType() const;
		void setAciType(AciType type);
		bool isProduction() const;
		void setIsProduction(bool isProduction);
		const sSection& getFacPos() const;
		void setFacSize(size_t size);
		const sSection& getSacPos() const;
		void setSacSize(size_t size);
		const sSection& getKcPos() const;
		void setKcSize(size_t size);

	private:
		const std::string kModuleName = "ACI_HEADER";
		const std::string kAciStructSig = "ACI0";
		const std::string kAciDescStructSig = "ACID";
		static const size_t kAciAlignSize = 0x10;

#pragma pack(push, 1)
		struct sAciHeader
		{
		private:
			byte_t signature_[4];
			uint32_t size_; // includes prefacing signature, set only in ACID made by SDK (it enables easy resigning)
			byte_t reserved_0[4];
			uint32_t flags_; // set in ACID only
			uint64_t program_id_; // set only in ACI0 (since ACID is generic)
			uint64_t program_id_max_;
			struct sAciSection
			{
			private:
				uint32_t offset_; // aligned by 0x10 from the last one
				uint32_t size_;
			public:
				uint32_t offset() const { return le_word(offset_); }
				void set_offset(uint32_t offset) { offset_ = le_word(offset); }

				uint32_t size() const { return le_word(size_); }
				void set_size(uint32_t size) { size_ = le_word(size); }
			} fac_, sac_, kc_;
		public:
			const char* signature() const { return (const char*)signature_; }
			void set_signature(const char* signature) { memcpy(signature_, signature, 4); }

			uint32_t size() const { return le_word(size_); }
			void set_size(uint32_t size) { size_ = le_word(size); }

			uint32_t flags() const { return le_word(flags_); }
			void set_flags(uint32_t flags) { flags_ = le_word(flags); }

			uint64_t program_id() const { return le_dword(program_id_); }
			void set_program_id(uint64_t program_id) { program_id_ = le_dword(program_id); }

			uint64_t program_id_min() const { return program_id(); }
			void set_program_id_min(uint64_t program_id) { set_program_id(program_id); }

			uint64_t program_id_max() const { return le_dword(program_id_max_); }
			void set_program_id_max(uint64_t program_id) { program_id_max_ = le_dword(program_id); }

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

		// ACI variables
		uint64_t mProgramId;

		// ACID variables
		size_t mAcidSize;
		uint64_t mProgramIdMin;
		uint64_t mProgramIdMax;

		// ACI(D) variables
		size_t mHeaderOffset;
		AciType mType;
		bool mIsProduction;
		sSection mFac, mSac, mKc;

		void calculateSectionOffsets();
		bool isEqual(const AciHeader& other) const;
		void copyFrom(const AciHeader& other);
	};
}

