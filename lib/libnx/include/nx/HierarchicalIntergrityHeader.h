#pragma once
#include <nx/hierarchicalintergrity.h>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <fnd/ISerialiseableBinary.h>

namespace nx
{
	class HierarchicalIntergrityHeader :
		public fnd::ISerialiseableBinary
	{
	public:

		HierarchicalIntergrityHeader();
		HierarchicalIntergrityHeader(const HierarchicalIntergrityHeader& other);
		HierarchicalIntergrityHeader(const byte_t* bytes, size_t len);

		bool operator==(const HierarchicalIntergrityHeader& other) const;
		bool operator!=(const HierarchicalIntergrityHeader& other) const;
		void operator=(const HierarchicalIntergrityHeader& other);

		// to be used after export
		const byte_t* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const byte_t* bytes, size_t len);

		// variables
		void clear();


	private:
		const std::string kModuleName = "HIERARCHICAL_INTERGRITY_HEADER";

		// binary
		fnd::MemoryBlob mBinaryBlob;

		// data


		uint64_t blockNumToSize(uint32_t block_num) const;
		uint32_t sizeToBlockNum(uint64_t real_size) const;
		bool isEqual(const HierarchicalIntergrityHeader& other) const;
		void copyFrom(const HierarchicalIntergrityHeader& other);
	};

}