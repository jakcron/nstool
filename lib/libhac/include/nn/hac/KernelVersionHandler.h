#pragma once
#include <nn/hac/IKernelCapabilityHandler.h>
#include <nn/hac/KernelVersionEntry.h>

namespace nn
{
namespace hac
{
	class KernelVersionHandler :
		public IKernelCapabilityHandler
	{
	public:
		KernelVersionHandler();

		void operator=(const KernelVersionHandler& other);
		bool operator==(const KernelVersionHandler& other) const;
		bool operator!=(const KernelVersionHandler& other) const;

		// kernel capabilty list in/out
		void importKernelCapabilityList(const fnd::List<KernelCapabilityEntry>& caps);
		void exportKernelCapabilityList(fnd::List<KernelCapabilityEntry>& caps) const;
		void clear();
		bool isSet() const;

		// variables
		uint16_t getVerMajor() const;
		void setVerMajor(uint16_t major);
		uint8_t getVerMinor() const;
		void setVerMinor(uint8_t minor);

	private:
		const std::string kModuleName = "KERNEL_VERSION_HANDLER";
		static const size_t kMaxKernelCapNum = 1;

		bool mIsSet;
		KernelVersionEntry mEntry;
	};
}
}