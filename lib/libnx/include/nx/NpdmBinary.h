#pragma once
#include <string>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <nx/NpdmHeader.h>
#include <nx/AciBinary.h>
#include <nx/AcidBinary.h>


namespace nx
{
	class NpdmBinary :
		public NpdmHeader
	{
	public:
		NpdmBinary();
		NpdmBinary(const NpdmBinary& other);
		NpdmBinary(const byte_t* bytes, size_t len);

		bool operator==(const NpdmBinary& other) const;
		bool operator!=(const NpdmBinary& other) const;
		void operator=(const NpdmBinary& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const byte_t* bytes, size_t len);

		// variables
		void clear();

		const AciBinary& getAci() const;
		void setAci(const AciBinary& aci);

		const AcidBinary& getAcid() const;
		void setAcid(const AcidBinary& acid);
	private:
		const std::string kModuleName = "NPDM_BINARY";

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		AciBinary mAci;
		AcidBinary mAcid;

		bool isEqual(const NpdmBinary& other) const;
		void copyFrom(const NpdmBinary& other);
	};

}
