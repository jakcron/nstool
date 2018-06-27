#pragma once
#include <string>
#include <fnd/List.h>
#include <nx/NpdmHeader.h>
#include <nx/AccessControlInfoBinary.h>
#include <nx/AccessControlInfoDescBinary.h>


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

		const AccessControlInfoBinary& getAci() const;
		void setAci(const AccessControlInfoBinary& aci);

		const AccessControlInfoDescBinary& getAcid() const;
		void setAcid(const AccessControlInfoDescBinary& acid);
	private:
		const std::string kModuleName = "NPDM_BINARY";

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		AccessControlInfoBinary mAci;
		AccessControlInfoDescBinary mAcid;
	};

}
