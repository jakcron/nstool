#pragma once
#include <string>
#include <fnd/ISerialisable.h>
#include <fnd/List.h>
#include <nx/fac.h>

namespace nx
{
	class FacHeader :
		public fnd::ISerialisable
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

		FacHeader();
		FacHeader(const FacHeader& other);

		void operator=(const FacHeader& other);
		bool operator==(const FacHeader& other) const;
		bool operator!=(const FacHeader& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();
		size_t getFacSize() const;

		uint32_t getFormatVersion() const;
		void setFormatVersion(uint32_t version);

		const fnd::List<fac::FsAccessFlag>& getFsaRightsList() const;
		void setFsaRightsList(const fnd::List<fac::FsAccessFlag>& list);

		const sSection& getContentOwnerIdPos() const;
		void setContentOwnerIdSize(size_t size);

		const sSection& getSaveDataOwnerIdPos() const;;
		void setSaveDataOwnerIdSize(size_t size);

	private:
		const std::string kModuleName = "FAC_HEADER";
		
		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		uint32_t mVersion;
		fnd::List<fac::FsAccessFlag> mFsaRights;
		sSection mContentOwnerIdPos;
		sSection mSaveDataOwnerIdPos;

		void calculateOffsets();
	};
}

