#include <nn/hac/MemoryMappingHandler.h>
#include <nn/hac/MemoryPageEntry.h>

nn::hac::MemoryMappingHandler::MemoryMappingHandler() :
	mIsSet(false)
{}

void nn::hac::MemoryMappingHandler::operator=(const MemoryMappingHandler & other)
{
	mIsSet = other.mIsSet;
	mMemRange = other.mMemRange;
	mMemPage = other.mMemPage;
}

bool nn::hac::MemoryMappingHandler::operator==(const MemoryMappingHandler & other) const
{
	return (mIsSet == other.mIsSet) \
		&& (mMemRange == other.mMemRange) \
		&& (mMemPage == other.mMemPage);
}

bool nn::hac::MemoryMappingHandler::operator!=(const MemoryMappingHandler & other) const
{
	return !(*this == other);
}

void nn::hac::MemoryMappingHandler::importKernelCapabilityList(const fnd::List<KernelCapabilityEntry>& caps)
{
	if (caps.size() == 0)
		return;

	// get entry list
	fnd::List<MemoryPageEntry> entries;
	for (size_t i = 0; i < caps.size(); i++)
	{
		entries.addElement(caps[i]);
	}

	mMemRange.clear();
	mMemPage.clear();
	for (size_t i = 0; i < entries.size();)
	{
		// has flag means "MemMap"
		if (entries[i].isMultiplePages())
		{
			// this entry is the last one or the next one isn't a memory map
			if ((i + 1) == entries.size() || entries[i+1].isMultiplePages() == false)
			{
				throw fnd::Exception(kModuleName, "No paired entry");
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
			mMemRange.addElement({ entries[i].getPage(), entries[i+1].getPage(), entries[i].getFlag() ? MEM_RO : MEM_RW, entries[i+1].getFlag() ? MAP_STATIC : MAP_IO });

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
			mMemPage.addElement({ entries[i].getPage(), 1, MEM_RW, MAP_IO });

			// increment i by one
			i += 1;
		}
	}

	mIsSet = true;
}

void nn::hac::MemoryMappingHandler::exportKernelCapabilityList(fnd::List<KernelCapabilityEntry>& caps) const
{
	if (isSet() == false)
		return;

	MemoryPageEntry cap;

	// "mem maps"
	cap.setMapMultiplePages(true);
	for (size_t i = 0; i < mMemRange.size(); i++)
	{
		cap.setPage(mMemRange[i].addr & kMaxPageAddr);
		cap.setFlag(mMemRange[i].perm == MEM_RO);
		caps.addElement(cap.getKernelCapability());

		cap.setPage(mMemRange[i].size & kMaxPageNum);
		cap.setFlag(mMemRange[i].type == MAP_STATIC);
		caps.addElement(cap.getKernelCapability());
	}

	// "io maps"
	cap.setMapMultiplePages(false);
	for (size_t i = 0; i < mMemPage.size(); i++)
	{
		cap.setPage(mMemPage[i].addr & kMaxPageAddr);
		caps.addElement(cap.getKernelCapability());
	}
}

void nn::hac::MemoryMappingHandler::clear()
{
	mIsSet = false;
	mMemRange.clear();
	mMemPage.clear();
}

bool nn::hac::MemoryMappingHandler::isSet() const
{
	return mIsSet;
}

const fnd::List<nn::hac::MemoryMappingHandler::sMemoryMapping>& nn::hac::MemoryMappingHandler::getMemoryMaps() const
{
	return mMemRange;
}

const fnd::List<nn::hac::MemoryMappingHandler::sMemoryMapping>& nn::hac::MemoryMappingHandler::getIoMemoryMaps() const
{
	return mMemPage;
}