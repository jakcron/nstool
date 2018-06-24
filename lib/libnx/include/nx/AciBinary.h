#pragma once
#include <string>
#include <fnd/List.h>
#include <nx/AciHeader.h>
#include <nx/FacBinary.h>
#include <nx/SacBinary.h>
#include <nx/KcBinary.h>

namespace nx
{
	class AciBinary :
		public AciHeader
	{
	public:
		AciBinary();
		AciBinary(const AciBinary& other);

		void operator=(const AciBinary& other);
		bool operator==(const AciBinary& other) const;
		bool operator!=(const AciBinary& other) const;

		// export/import binary
		virtual void toBytes();
		virtual void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		virtual void clear();

		const FacBinary& getFac() const;
		void setFac(const FacBinary& fac);

		const SacBinary& getSac() const;
		void setSac(const SacBinary& sac);

		const KcBinary& getKc() const;
		void setKc(const KcBinary& kc);

	private:
		const std::string kModuleName = "ACI_BINARY";

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		FacBinary mFac;
		SacBinary mSac;
		KcBinary mKc;
	};
}

