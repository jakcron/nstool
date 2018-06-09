#pragma once
#include <nx/nro.h>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	class NroHeader : 
		public fnd::ISerialiseableBinary
	{
	public:
		struct sRoCrt
		{
			byte_t data[nro::kRoCrtSize];

			void operator=(const sRoCrt& other)
			{
				memcpy(data, other.data, nro::kRoCrtSize);
			}

			bool operator==(const sRoCrt& other) const
			{
				return memcmp(data, other.data, nro::kRoCrtSize) == 0;
			}

			bool operator!=(const sRoCrt& other) const
			{
				return !(*this == other);
			}
		};

		struct sModuleId
		{
			byte_t data[nro::kModuleIdSize];

			void operator=(const sModuleId& other)
			{
				memcpy(data, other.data, nro::kModuleIdSize);
			}

			bool operator==(const sModuleId& other) const
			{
				return memcmp(data, other.data, nro::kModuleIdSize) == 0;
			}

			bool operator!=(const sModuleId& other) const
			{
				return !(*this == other);
			}
		};

		struct sSection
		{
			uint32_t memory_offset;
			uint32_t size;

			void operator=(const sSection& other)
			{
				memory_offset = other.memory_offset;
				size = other.size;
			}

			bool operator==(const sSection& other) const
			{
				return (memory_offset == other.memory_offset) \
					&& (size == other.size);
			}

			bool operator!=(const sSection& other) const
			{
				return !(*this == other);
			}
		};

		NroHeader();
		NroHeader(const NroHeader& other);
		NroHeader(const byte_t* bytes, size_t len);

		bool operator==(const NroHeader& other) const;
		bool operator!=(const NroHeader& other) const;
		void operator=(const NroHeader& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const byte_t* bytes, size_t len);

		// variables
		void clear();
		
		const sRoCrt& getRoCrt() const;
		void setRoCrt(const sRoCrt& ro_crt);

		uint32_t getNroSize() const;
		void setNroSize(uint32_t size);

		const sSection& getTextInfo() const;
		void setTextInfo(const sSection& info);

		const sSection& getRoInfo() const;
		void setRoInfo(const sSection& info);

		const sSection& getDataInfo() const;
		void setDataInfo(const sSection& info);

		uint32_t getBssSize() const;
		void setBssSize(uint32_t size);

		const sModuleId& getModuleId() const;
		void setModuleId(const sModuleId& id);

		const sSection& getRoEmbeddedInfo() const;
		void setRoEmbeddedInfo(const sSection& info);

		const sSection& getRoDynStrInfo() const;
		void setRoDynStrInfo(const sSection& info);

		const sSection& getRoDynSymInfo() const;
		void setRoDynSymInfo(const sSection& info);
	private:
		const std::string kModuleName = "NRO_HEADER";

		// binary
		fnd::MemoryBlob mBinaryBlob;

		// data
		sRoCrt mRoCrt;
		uint32_t mNroSize;
		sSection mTextInfo;
		sSection mRoInfo;
		sSection mDataInfo;
		uint32_t mBssSize;
		sModuleId mModuleId;
		sSection mRoEmbeddedInfo;
		sSection mRoDynStrInfo;
		sSection mRoDynSymInfo;
		
		// helpers
		bool isEqual(const NroHeader& other) const;
		void copyFrom(const NroHeader& other);
	};

}