#include "MemoryPageEntry.h"



nx::MemoryPageEntry::MemoryPageEntry() :
	mCap(KernelCapability::KC_INVALID),
	mPage(0),
	mFlag(false),
	mUseFlag(false)
{}

nx::MemoryPageEntry::MemoryPageEntry(const KernelCapability & kernel_cap) :
	mCap(KernelCapability::KC_INVALID),
	mPage(0),
	mFlag(false),
	mUseFlag(false)
{
	setKernelCapability(kernel_cap);
}

nx::MemoryPageEntry::MemoryPageEntry(u32 page) :
	mCap(KernelCapability::KC_INVALID),
	mPage(0),
	mFlag(false),
	mUseFlag(false)
{
	setPage(page);
}

nx::MemoryPageEntry::MemoryPageEntry(u32 page, bool flag) :
	mCap(KernelCapability::KC_INVALID),
	mPage(0),
	mFlag(false),
	mUseFlag(true)
{
	setPage(page);
	setFlag(flag);
}

const nx::KernelCapability & nx::MemoryPageEntry::getKernelCapability() const
{
	return mCap;
}

void nx::MemoryPageEntry::setKernelCapability(const KernelCapability & kernel_cap)
{
	if (kernel_cap.getType() != KernelCapability::KC_MEMORY_MAP && kernel_cap.getType() != KernelCapability::KC_IO_MEMORY_MAP)
	{
		throw fnd::Exception(kModuleName, "KernelCapability is not type 'MemoryMap' or 'IOMemoryMap");
	}

	mCap = kernel_cap;
	processCapField();
}

u32 nx::MemoryPageEntry::getPage() const
{
	return mPage;
}

void nx::MemoryPageEntry::setPage(u32 page)
{
	if (page > kMaxPage)
	{
		throw fnd::Exception(kModuleName, "Illegal memory page. (range: 0x000000 - 0xFFFFFF)");
	}

	mPage = page;
	updateCapField();
}

bool nx::MemoryPageEntry::getFlag() const
{
	return mFlag;
}

void nx::MemoryPageEntry::setFlag(bool flag)
{
	mFlag = flag;
	updateCapField();
}

bool nx::MemoryPageEntry::isMultiplePages() const
{
	return mUseFlag;
}

void nx::MemoryPageEntry::setMapMultiplePages(bool useFlag)
{
	mUseFlag = useFlag;
}
