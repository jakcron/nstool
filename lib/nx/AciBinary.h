#pragma once
#include <string>
#include <fnd/memory_blob.h>
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
		AciBinary(const u8* bytes, size_t len);

		bool operator==(const AciBinary& other) const;
		bool operator!=(const AciBinary& other) const;
		void operator=(const AciBinary& other);

		// to be used after export
		const u8* getBytes() const;
		size_t getSize() const;

		// export/import binary
		virtual void exportBinary();
		virtual void importBinary(const u8* bytes, size_t len);

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
		fnd::MemoryBlob mBinaryBlob;

		// variables
		FacBinary mFac;
		SacBinary mSac;
		KcBinary mKc;

		bool isEqual(const AciBinary& other) const;
		void copyFrom(const AciBinary& other);
	};
}

