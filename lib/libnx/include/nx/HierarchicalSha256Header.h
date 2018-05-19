#pragma once
#include <nx/hierarchicalsha256.h>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	class HierarchicalSha256Header :
		public fnd::ISerialiseableBinary
	{
	public:

		HierarchicalSha256Header();
		HierarchicalSha256Header(const HierarchicalSha256Header& other);
		HierarchicalSha256Header(const byte_t* bytes, size_t len);

		bool operator==(const HierarchicalSha256Header& other) const;
		bool operator!=(const HierarchicalSha256Header& other) const;
		void operator=(const HierarchicalSha256Header& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const byte_t* bytes, size_t len);

		// variables
		void clear();


	private:
		const std::string kModuleName = "HIERARCHICAL_SHA256_HEADER";

		// binary
		fnd::MemoryBlob mBinaryBlob;

		// data


		uint64_t blockNumToSize(uint32_t block_num) const;
		uint32_t sizeToBlockNum(uint64_t real_size) const;
		bool isEqual(const HierarchicalSha256Header& other) const;
		void copyFrom(const HierarchicalSha256Header& other);
	};

}