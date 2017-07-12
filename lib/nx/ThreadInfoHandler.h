#pragma once
#include <nx/IKernelCapabilityHandler.h>
#include <nx/ThreadInfoEntry.h>

namespace nx
{
	class ThreadInfoHandler :
		public IKernelCapabilityHandler
	{
	public:
		ThreadInfoHandler();

		bool operator==(const ThreadInfoHandler& other) const;
		bool operator!=(const ThreadInfoHandler& other) const;
		void operator=(const ThreadInfoHandler& other);

		// kernel capabilty list in/out
		void importKernelCapabilityList(const fnd::List<KernelCapability>& caps);
		void exportKernelCapabilityList(fnd::List<KernelCapability>& caps) const;
		void clear();
		bool isSet() const;

		// variables
		u8 getMinPriority() const;
		void setMinPriority(u8 priority);
		u8 getMaxPriority() const;
		void setMaxPriority(u8 priority);
		u8 getMinCoreNumber() const;
		void setMinCoreNumber(u8 core_num);
		u8 getMaxCoreNumber() const;
		void setMaxCoreNumber(u8 core_num);

	private:
		const std::string kModuleName = "THREAD_INFO_HANDLER";
		static const size_t kMaxKernelCapNum = 1;

		bool mIsSet;
		ThreadInfoEntry mEntry;

		void copyFrom(const ThreadInfoHandler& other);
		bool isEqual(const ThreadInfoHandler& other) const;
	};
}

