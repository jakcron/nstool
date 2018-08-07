#pragma once
#include <nn/hac/IKernelCapabilityHandler.h>
#include <nn/hac/MiscFlagsEntry.h>

namespace nn
{
namespace hac
{
	class MiscFlagsHandler :
		public IKernelCapabilityHandler
	{
	public:
		enum Flags
		{
			FLAG_ENABLE_DEBUG,
			FLAG_FORCE_DEBUG,
			FLAG_UNK02,
			FLAG_UNK03,
			FLAG_UNK04,
			FLAG_UNK05,
			FLAG_UNK06,
			FLAG_UNK07,
			FLAG_UNK08,
			FLAG_UNK09,
			FLAG_UNK10,
			FLAG_UNK11,
			FLAG_UNK12,
			FLAG_UNK13,
			FLAG_UNK14,
			FLAG_NUM
		};

		MiscFlagsHandler();

		void operator=(const MiscFlagsHandler& other);
		bool operator==(const MiscFlagsHandler& other) const;
		bool operator!=(const MiscFlagsHandler& other) const;

		// kernel capabilty list in/out
		void importKernelCapabilityList(const fnd::List<KernelCapabilityEntry>& caps);
		void exportKernelCapabilityList(fnd::List<KernelCapabilityEntry>& caps) const;
		void clear();
		bool isSet() const;

		// variables
		const fnd::List<Flags>& getFlagList() const;
		void setFlagList(fnd::List<Flags> flags);

	private:
		const std::string kModuleName = "MISC_FLAGS_HANDLER";
		static const size_t kMaxKernelCapNum = 1;

		bool mIsSet;
		fnd::List<Flags> mFlags;
	};
}
}