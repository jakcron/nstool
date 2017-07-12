#include "MemoryMappingHandler.h"
#include "MemoryPageEntry.h"


nx::MemoryMappingHandler::MemoryMappingHandler() :
	mIsSet(false)
{}

bool nx::MemoryMappingHandler::operator==(const MemoryMappingHandler & other) const
{
	return isEqual(other);
}

bool nx::MemoryMappingHandler::operator!=(const MemoryMappingHandler & other) const
{
	return !isEqual(other);
}

void nx::MemoryMappingHandler::operator=(const MemoryMappingHandler & other)
{
	copyFrom(other);
}

void nx::MemoryMappingHandler::importKernelCapabilityList(const fnd::List<KernelCapability>& caps)
{
	fnd::List<MemoryPageEntry> entries;

	for (size_t i = 0; i < caps.getSize(); i++)
	{
		entries[i].setKernelCapability(caps[i]);
	}

	mMemRange.clear();
	mMemPage.clear();
	for (size_t i = 0; i < entries.getSize();)
	{
		// has flag means "MemMap"
		if (entries[i].isMultiplePages())
		{
			// this entry is the last one or the next one isn't a memory map
			if ((i + 1) == entries.getSize() || entries[i+1].isMultiplePages() == false)
			{
				throw fnd::Exception(kModuleName, "Illegal page address");
			}

			// check valid page address
			if (entries[i].getPage() > kMaxPageAddr)
			{
				throw fnd::Exception(kModuleName, "Illegal page address");
			}

			// check valid page num
			if (entries[i+1].getPage() > kMaxPageAddr)
			{
				throw fnd::Exception(kModuleName, "Illegal page num");
			}

			// add to list
			mMemRange.addElement({ entries[i].getPage(), entries[i+1].getPage(), !entries[i].getFlag(), !entries[i+1].getFlag() });

			// increment i by two
			i += 2;
		}
		// otherwise means "IoMemMap"
		else
		{
			// check valid page address
			if (entries[i].getPage() > kMaxPageAddr)
			{
				throw fnd::Exception(kModuleName, "Illegal page address");
			}

			// add to list
			mMemPage.addElement({ entries[i].getPage(), 1, true, true });

			// increment i by one
			i += 1;
		}
	}

	mIsSet = true;
}

void nx::MemoryMappingHandler::exportKernelCapabilityList(fnd::List<KernelCapability>& caps) const
{
	if (isSet() == false)
		return;

	MemoryPageEntry cap;

	// "mem maps"
	cap.setMapMultiplePages(true);
	for (size_t i = 0; i < mMemRange.getSize(); i++)
	{
		cap.setPage(mMemRange[i].addr & kMaxPageAddr);
		cap.setFlag(!mMemRange[i].isRW);
		caps.addElement(cap.getKernelCapability());

		cap.setPage(mMemRange[i].size & kMaxPageNum);
		cap.setFlag(!mMemRange[i].isIO);
		caps.addElement(cap.getKernelCapability());
	}

	// "io maps"
	cap.setMapMultiplePages(false);
	for (size_t i = 0; i < mMemPage.getSize(); i++)
	{
		cap.setPage(mMemPage[i].addr & kMaxPageAddr);
		caps.addElement(cap.getKernelCapability());
	}
}

void nx::MemoryMappingHandler::clear()
{
	mIsSet = false;
	mMemRange.clear();
	mMemPage.clear();
}

bool nx::MemoryMappingHandler::isSet() const
{
	return mIsSet;
}

const fnd::List<nx::MemoryMappingHandler::sMemoryMapping>& nx::MemoryMappingHandler::getMemoryMaps() const
{
	return mMemRange;
}

const fnd::List<nx::MemoryMappingHandler::sMemoryMapping>& nx::MemoryMappingHandler::getIoMemoryMaps() const
{
	return mMemPage;
}

void nx::MemoryMappingHandler::copyFrom(const MemoryMappingHandler & other)
{
	mIsSet = other.mIsSet;
	mMemRange = other.mMemRange;
	mMemPage = other.mMemPage;
}

bool nx::MemoryMappingHandler::isEqual(const MemoryMappingHandler & other) const
{
	return (mIsSet == other.mIsSet) \
		&& (mMemRange == other.mMemRange) \
		&& (mMemPage == other.mMemPage);
}
