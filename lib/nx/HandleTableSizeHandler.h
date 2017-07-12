#pragma once
#include <nx/IKernelCapabilityHandler.h>
#include <nx/HandleTableSizeEntry.h>

namespace nx
{
	class HandleTableSizeHandler :
		public IKernelCapabilityHandler
	{
	public:
		HandleTableSizeHandler();

		bool operator==(const HandleTableSizeHandler& other) const;
		bool operator!=(const HandleTableSizeHandler& other) const;
		void operator=(const HandleTableSizeHandler& other);

		// kernel capabilty list in/out
		void importKernelCapabilityList(const fnd::List<KernelCapability>& caps);
		void exportKernelCapabilityList(fnd::List<KernelCapability>& caps) const;
		void clear();
		bool isSet() const;

		// variables
		u16 getHandleTableSize() const;
		void setHandleTableSize(u16 size);

	private:
		const std::string kModuleName = "HANDLE_TABLE_SIZE_HANDLER";
		static const size_t kMaxKernelCapNum = 1;

		bool mIsSet;
		HandleTableSizeEntry mEntry;

		void copyFrom(const HandleTableSizeHandler& other);
		bool isEqual(const HandleTableSizeHandler& other) const;
	};
}

