#pragma once
#include <nn/hac/IKernelCapabilityHandler.h>
#include <nn/hac/ThreadInfoEntry.h>

namespace nn
{
namespace hac
{
	class ThreadInfoHandler :
		public IKernelCapabilityHandler
	{
	public:
		ThreadInfoHandler();

		void operator=(const ThreadInfoHandler& other);
		bool operator==(const ThreadInfoHandler& other) const;
		bool operator!=(const ThreadInfoHandler& other) const;

		// kernel capabilty list in/out
		void importKernelCapabilityList(const fnd::List<KernelCapabilityEntry>& caps);
		void exportKernelCapabilityList(fnd::List<KernelCapabilityEntry>& caps) const;
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
	};
}
}