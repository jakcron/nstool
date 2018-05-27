#pragma once
#include <nx/xci.h>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	class NsoHeader : 
		public fnd::ISerialiseableBinary
	{
	public:
		NsoHeader();
		NsoHeader(const NsoHeader& other);
		NsoHeader(const byte_t* bytes, size_t len);

		bool operator==(const NsoHeader& other) const;
		bool operator!=(const NsoHeader& other) const;
		void operator=(const NsoHeader& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const byte_t* bytes, size_t len);

		// variables
		void clear();
		

	private:
		const std::string kModuleName = "NSO_HEADER";

		// binary
		fnd::MemoryBlob mBinaryBlob;

		// data
		
		// helpers
		bool isEqual(const NsoHeader& other) const;
		void copyFrom(const NsoHeader& other);
	};

}