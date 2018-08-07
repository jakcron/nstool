#pragma once
#include <fnd/types.h>
#include <fnd/List.h>
#include <nx/KernelCapabilityEntry.h>

namespace nx
{
	class IKernelCapabilityHandler
	{
	public:
		virtual void importKernelCapabilityList(const fnd::List<KernelCapabilityEntry>& caps) = 0;
		virtual void exportKernelCapabilityList(fnd::List<KernelCapabilityEntry>& caps) const = 0;
		virtual void clear() = 0;
		virtual bool isSet() const = 0;
	};
}

