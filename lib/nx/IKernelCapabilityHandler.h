#pragma once
#include <fnd/types.h>
#include <fnd/List.h>
#include <nx/KernelCapability.h>

namespace nx
{
	class IKernelCapabilityHandler
	{
	public:
		virtual void importKernelCapabilityList(const fnd::List<KernelCapability>& caps) = 0;
		virtual const fnd::List<KernelCapability>& exportKernelCapabilityList() = 0;
	};
}

