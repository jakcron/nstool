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
		uint8_t getMinPriority() const;
		void setMinPriority(uint8_t priority);
		uint8_t getMaxPriority() const;
		void setMaxPriority(uint8_t priority);
		uint8_t getMinCpuId() const;
		void setMinCpuId(uint8_t core_num);
		uint8_t getMaxCpuId() const;
		void setMaxCpuId(uint8_t core_num);

	private:
		const std::string kModuleName = "THREAD_INFO_HANDLER";
		static const size_t kMaxKernelCapNum = 1;

		bool mIsSet;
		ThreadInfoEntry mEntry;

		void copyFrom(const ThreadInfoHandler& other);
		bool isEqual(const ThreadInfoHandler& other) const;
	};
}

