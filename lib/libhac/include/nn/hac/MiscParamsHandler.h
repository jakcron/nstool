#pragma once
#include <nn/hac/IKernelCapabilityHandler.h>
#include <nn/hac/MiscParamsEntry.h>

namespace nn
{
namespace hac
{
	class MiscParamsHandler :
		public IKernelCapabilityHandler
	{
	public:
		MiscParamsHandler();

		void operator=(const MiscParamsHandler& other);
		bool operator==(const MiscParamsHandler& other) const;
		bool operator!=(const MiscParamsHandler& other) const;

		// kernel capabilty list in/out
		void importKernelCapabilityList(const fnd::List<KernelCapabilityEntry>& caps);
		void exportKernelCapabilityList(fnd::List<KernelCapabilityEntry>& caps) const;
		void clear();
		bool isSet() const;

		// variables
		byte_t getProgramType() const;
		void setProgramType(byte_t type);

	private:
		const std::string kModuleName = "MISC_PARAMS_HANDLER";
		static const size_t kMaxKernelCapNum = 1;

		bool mIsSet;
		MiscParamsEntry mEntry;
	};
}
}