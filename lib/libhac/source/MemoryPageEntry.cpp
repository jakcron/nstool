#include <nx/MemoryPageEntry.h>

nx::MemoryPageEntry::MemoryPageEntry() :
	mCap(kc::KC_INVALID),
	mPage(0),
	mFlag(false),
	mUseFlag(false)
{}

nx::MemoryPageEntry::MemoryPageEntry(const KernelCapabilityEntry & kernel_cap) :
	mCap(kc::KC_INVALID),
	mPage(0),
	mFlag(false),
	mUseFlag(false)
{
	setKernelCapability(kernel_cap);
}

nx::MemoryPageEntry::MemoryPageEntry(uint32_t page) :
	mCap(kc::KC_INVALID),
	mPage(0),
	mFlag(false),
	mUseFlag(false)
{
	setPage(page);
}

nx::MemoryPageEntry::MemoryPageEntry(uint32_t page, bool flag) :
	mCap(kc::KC_INVALID),
	mPage(0),
	mFlag(false),
	mUseFlag(true)
{
	setPage(page);
	setFlag(flag);
}

void nx::MemoryPageEntry::operator=(const MemoryPageEntry& other)
{
	mPage = other.mPage;
	mFlag = other.mFlag;
	mUseFlag = other.mUseFlag;
	updateCapField();
}

bool nx::MemoryPageEntry::operator==(const MemoryPageEntry& other) const
{
	return (mPage == other.mPage) \
		&& (mFlag == other.mFlag) \
		&& (mUseFlag == other.mUseFlag);
}

bool nx::MemoryPageEntry::operator!=(const MemoryPageEntry& other) const
{
	return !(*this == other);
}

const nx::KernelCapabilityEntry & nx::MemoryPageEntry::getKernelCapability() const
{
	return mCap;
}

void nx::MemoryPageEntry::setKernelCapability(const KernelCapabilityEntry & kernel_cap)
{
	if (kernel_cap.getType() != kc::KC_MEMORY_MAP && kernel_cap.getType() != kc::KC_IO_MEMORY_MAP)
	{
		throw fnd::Exception(kModuleName, "KernelCapabilityEntry is not type 'MemoryMap' or 'IOMemoryMap");
	}

	mCap = kernel_cap;
	processCapField();
}

uint32_t nx::MemoryPageEntry::getPage() const
{
	return mPage;
}

void nx::MemoryPageEntry::setPage(uint32_t page)
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
