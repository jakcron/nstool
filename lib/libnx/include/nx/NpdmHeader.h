#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/MemoryBlob.h>
#include <fnd/ISerialiseableBinary.h>
#include <nx/npdm.h>

namespace nx
{
	class NpdmHeader :
		public fnd::ISerialiseableBinary
	{
	public:
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

		NpdmHeader();
		NpdmHeader(const NpdmHeader& other);
		NpdmHeader(const byte_t* bytes, size_t len);

		bool operator==(const NpdmHeader& other) const;
		bool operator!=(const NpdmHeader& other) const;
		void operator=(const NpdmHeader& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const byte_t* bytes, size_t len);

		// variables
		void clear();
		size_t getNpdmSize() const;

		npdm::InstructionType getInstructionType() const;
		void setInstructionType(npdm::InstructionType type);

		npdm::ProcAddrSpaceType getProcAddressSpaceType() const;
		void setProcAddressSpaceType(npdm::ProcAddrSpaceType type);

		byte_t getMainThreadPriority() const;
		void setMainThreadPriority(byte_t priority);

		byte_t getMainThreadCpuId() const;
		void setMainThreadCpuId(byte_t cpu_id);

		uint32_t getVersion() const;
		void setVersion(uint32_t version);

		uint32_t getMainThreadStackSize() const;
		void setMainThreadStackSize(uint32_t size);

		const std::string& getName() const;
		void setName(const std::string& name);

		const std::string& getProductCode() const;
		void setProductCode(const std::string& product_code);

		const sSection& getAciPos() const;
		void setAciSize(size_t size);

		const sSection& getAcidPos() const;
		void setAcidSize(size_t size);
	private:
		const std::string kModuleName = "NPDM_HEADER";
		
		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		npdm::InstructionType mInstructionType;
		npdm::ProcAddrSpaceType mProcAddressSpaceType;
		byte_t mMainThreadPriority;
		byte_t mMainThreadCpuId;
		uint32_t mVersion;
		uint32_t mMainThreadStackSize;
		std::string mName;
		std::string mProductCode;
		sSection mAciPos;
		sSection mAcidPos;

		void calculateOffsets();
		bool isEqual(const NpdmHeader& other) const;
		void copyFrom(const NpdmHeader& other);
	};
}

