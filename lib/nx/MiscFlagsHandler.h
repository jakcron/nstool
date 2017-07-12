#pragma once
#include <nx/IKernelCapabilityHandler.h>
#include <nx/MiscFlagsEntry.h>

namespace nx
{
	class MiscFlagsHandler :
		public IKernelCapabilityHandler
	{
	public:
		enum Flags
		{
			FLAG_ENABLE_DEBUG = BIT(0),
			FLAG_FORCE_DEBUG = BIT(1),
			FLAG_UNK02 = BIT(2),
			FLAG_UNK03 = BIT(3),
			FLAG_UNK04 = BIT(4),
			FLAG_UNK05 = BIT(5),
			FLAG_UNK06 = BIT(6),
			FLAG_UNK07 = BIT(7),
			FLAG_UNK08 = BIT(8),
			FLAG_UNK09 = BIT(9),
			FLAG_UNK10 = BIT(10),
			FLAG_UNK11 = BIT(11),
			FLAG_UNK12 = BIT(12),
			FLAG_UNK13 = BIT(13),
			FLAG_UNK14 = BIT(14),
		};

		MiscFlagsHandler();

		bool operator==(const MiscFlagsHandler& other) const;
		bool operator!=(const MiscFlagsHandler& other) const;
		void operator=(const MiscFlagsHandler& other);

		// kernel capabilty list in/out
		void importKernelCapabilityList(const fnd::List<KernelCapability>& caps);
		void exportKernelCapabilityList(fnd::List<KernelCapability>& caps) const;
		void clear();
		bool isSet() const;

		// variables
		u32 getFlags() const;
		void setFlags(u32 flags);
	private:
		const std::string kModuleName = "MISC_FLAGS_HANDLER";
		static const size_t kMaxKernelCapNum = 1;

		bool mIsSet;
		MiscFlagsEntry mEntry;

		void copyFrom(const MiscFlagsHandler& other);
		bool isEqual(const MiscFlagsHandler& other) const;
	};

}
