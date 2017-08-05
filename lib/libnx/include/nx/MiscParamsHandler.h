#pragma once
#include <nx/IKernelCapabilityHandler.h>
#include <nx/MiscParamsEntry.h>

namespace nx
{
	class MiscParamsHandler :
		public IKernelCapabilityHandler
	{
	public:
		MiscParamsHandler();

		bool operator==(const MiscParamsHandler& other) const;
		bool operator!=(const MiscParamsHandler& other) const;
		void operator=(const MiscParamsHandler& other);

		// kernel capabilty list in/out
		void importKernelCapabilityList(const fnd::List<KernelCapability>& caps);
		void exportKernelCapabilityList(fnd::List<KernelCapability>& caps) const;
		void clear();
		bool isSet() const;

		// variables
		u8 getProgramType() const;
		void setProgramType(u8 type);

	private:
		const std::string kModuleName = "MISC_PARAMS_HANDLER";
		static const size_t kMaxKernelCapNum = 1;

		bool mIsSet;
		MiscParamsEntry mEntry;

		void copyFrom(const MiscParamsHandler& other);
		bool isEqual(const MiscParamsHandler& other) const;
	};
}

