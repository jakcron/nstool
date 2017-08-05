#pragma once
#include <nx/IKernelCapabilityHandler.h>
#include <nx/KernelVersionEntry.h>

namespace nx
{
	class KernelVersionHandler :
		public IKernelCapabilityHandler
	{
	public:
		KernelVersionHandler();

		bool operator==(const KernelVersionHandler& other) const;
		bool operator!=(const KernelVersionHandler& other) const;
		void operator=(const KernelVersionHandler& other);

		// kernel capabilty list in/out
		void importKernelCapabilityList(const fnd::List<KernelCapability>& caps);
		void exportKernelCapabilityList(fnd::List<KernelCapability>& caps) const;
		void clear();
		bool isSet() const;

		// variables
		u16 getVerMajor() const;
		void setVerMajor(u16 major);
		u8 getVerMinor() const;
		void setVerMinor(u8 minor);

	private:
		const std::string kModuleName = "KERNEL_VERSION_HANDLER";
		static const size_t kMaxKernelCapNum = 1;

		bool mIsSet;
		KernelVersionEntry mEntry;

		void copyFrom(const KernelVersionHandler& other);
		bool isEqual(const KernelVersionHandler& other) const;
	};
}

