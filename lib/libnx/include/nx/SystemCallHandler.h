#pragma once
#include <nx/IKernelCapabilityHandler.h>

namespace nx
{
	class SystemCallHandler :
		public IKernelCapabilityHandler
	{
	public:
		static const uint32_t kMaxSystemCall = (BIT(3) * 24) - 1;

		SystemCallHandler();

		bool operator==(const SystemCallHandler& other) const;
		bool operator!=(const SystemCallHandler& other) const;
		void operator=(const SystemCallHandler& other);

		// kernel capabilty list in/out
		void importKernelCapabilityList(const fnd::List<KernelCapability>& caps);
		void exportKernelCapabilityList(fnd::List<KernelCapability>& caps) const;
		void clear();
		bool isSet() const;

		// variables
		const fnd::List<uint8_t>& getSystemCalls() const;
		void setSystemCallList(const fnd::List<uint8_t>& calls);

	private:
		const std::string kModuleName = "SYSTEM_CALL_HANDLER";
		static const size_t kSyscallTotalEntryNum = (kMaxSystemCall / 24) + 1;

		bool mIsSet;
		fnd::List<uint8_t> mSystemCalls;

		void copyFrom(const SystemCallHandler& other);
		bool isEqual(const SystemCallHandler& other) const;
	};

}
