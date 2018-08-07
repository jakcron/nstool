#pragma once
#include <nn/hac/IKernelCapabilityHandler.h>
#include <nn/hac/InteruptEntry.h>

namespace nn
{
namespace hac
{
	class InteruptHandler :
		public IKernelCapabilityHandler
	{
	public:
		InteruptHandler();

		void operator=(const InteruptHandler& other);
		bool operator==(const InteruptHandler& other) const;
		bool operator!=(const InteruptHandler& other) const;

		// kernel capabilty list in/out
		void importKernelCapabilityList(const fnd::List<KernelCapabilityEntry>& caps);
		void exportKernelCapabilityList(fnd::List<KernelCapabilityEntry>& caps) const;
		void clear();
		bool isSet() const;

		// variables
		const fnd::List<uint16_t>& getInteruptList() const;
		void setInteruptList(const fnd::List<uint16_t>& interupts);

	private:
		const std::string kModuleName = "INTERUPT_HANDLER";

		bool mIsSet;
		fnd::List<uint16_t> mInterupts;
	};
}
}