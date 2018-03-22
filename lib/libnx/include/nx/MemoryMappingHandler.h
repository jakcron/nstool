#pragma once
#include <nx/IKernelCapabilityHandler.h>

namespace nx
{
	class MemoryMappingHandler :
		public IKernelCapabilityHandler
	{
	public:
		enum MemoryPerm
		{
			MEM_RW,
			MEM_RO
		};

		enum MappingType
		{
			MAP_IO,
			MAP_STATIC
		};

		struct sMemoryMapping
		{
			uint32_t addr; // page index
			uint32_t size; // page num
			MemoryPerm perm;
			MappingType type;

			bool operator==(const sMemoryMapping& other) const
			{
				return (addr == other.addr) \
					&& (size == other.size) \
					&& (perm == other.perm) \
					&& (type == other.type);
			}

			bool operator!=(const sMemoryMapping& other) const
			{
				return !operator==(other);
			}

			const sMemoryMapping& operator=(const sMemoryMapping& other)
			{
				addr = other.addr;
				size = other.size;
				perm = other.perm;
				type = other.type;

				return *this;
			}
		};

		MemoryMappingHandler();

		bool operator==(const MemoryMappingHandler& other) const;
		bool operator!=(const MemoryMappingHandler& other) const;
		void operator=(const MemoryMappingHandler& other);

		// kernel capabilty list in/out
		void importKernelCapabilityList(const fnd::List<KernelCapability>& caps);
		void exportKernelCapabilityList(fnd::List<KernelCapability>& caps) const;
		void clear();
		bool isSet() const;

		const fnd::List<sMemoryMapping>& getMemoryMaps() const;
		const fnd::List<sMemoryMapping>& getIoMemoryMaps() const;

	private:
		const std::string kModuleName = "MEMORY_MAPPING_HANDLER";
		static const uint32_t kMaxPageAddr = BIT(24) - 1;
		static const uint32_t kMaxPageNum = BIT(20) - 1;

		bool mIsSet;
		fnd::List<sMemoryMapping> mMemRange;
		fnd::List<sMemoryMapping> mMemPage;

		void copyFrom(const MemoryMappingHandler& other);
		bool isEqual(const MemoryMappingHandler& other) const;
	};
}

