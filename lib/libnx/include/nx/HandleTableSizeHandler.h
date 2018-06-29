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

		void operator=(const HandleTableSizeHandler& other);
		bool operator==(const HandleTableSizeHandler& other) const;
		bool operator!=(const HandleTableSizeHandler& other) const;

		// kernel capabilty list in/out
		void importKernelCapabilityList(const fnd::List<KernelCapabilityEntry>& caps);
		void exportKernelCapabilityList(fnd::List<KernelCapabilityEntry>& caps) const;
		void clear();
		bool isSet() const;

		// variables
		uint16_t getHandleTableSize() const;
		void setHandleTableSize(uint16_t size);

	private:
		const std::string kModuleName = "HANDLE_TABLE_SIZE_HANDLER";
		static const size_t kMaxKernelCapNum = 1;

		bool mIsSet;
		HandleTableSizeEntry mEntry;
	};
}

