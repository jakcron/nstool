#pragma once
#include <string>
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

		void operator=(const NpdmBinary& other);
		bool operator==(const NpdmBinary& other) const;
		bool operator!=(const NpdmBinary& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		const AciBinary& getAci() const;
		void setAci(const AciBinary& aci);

		const AcidBinary& getAcid() const;
		void setAcid(const AcidBinary& acid);
	private:
		const std::string kModuleName = "NPDM_BINARY";

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		AciBinary mAci;
		AcidBinary mAcid;
	};

}
