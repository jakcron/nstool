#include "KcBinary.h"



nx::KcBinary::KcBinary()
{}

nx::KcBinary::KcBinary(const KcBinary & other)
{
	copyFrom(other);
}

nx::KcBinary::KcBinary(const u8 * bytes, size_t len)
{
	importBinary(bytes, len);
}

bool nx::KcBinary::operator==(const KcBinary & other) const
{
	return isEqual(other);
}

bool nx::KcBinary::operator!=(const KcBinary & other) const
{
	return !isEqual(other);
}

void nx::KcBinary::operator=(const KcBinary & other)
{
	copyFrom(other);
}

const u8 * nx::KcBinary::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::KcBinary::getSize() const
{
	return mBinaryBlob.getSize();
}

void nx::KcBinary::exportBinary()
{
	fnd::List<KernelCapability> caps;

	// get kernel capabiliteis
	mThreadInfo.exportKernelCapabilityList(caps);
	mSystemCalls.exportKernelCapabilityList(caps);
	mMemoryMap.exportKernelCapabilityList(caps);
	mInterupts.exportKernelCapabilityList(caps);
	mMiscParams.exportKernelCapabilityList(caps);
	mKernelVersion.exportKernelCapabilityList(caps);
	mHandleTableSize.exportKernelCapabilityList(caps);
	mMiscFlags.exportKernelCapabilityList(caps);

	// allocate memory
	mBinaryBlob.alloc(caps.getSize() * sizeof(u32));

	// write to binary
	u32* raw_caps = (u32*)mBinaryBlob.getBytes();
	for (size_t i = 0; i < caps.getSize(); i++)
	{
		raw_caps[i] = le_word(caps[i].getCap());
	}
}

void nx::KcBinary::importBinary(const u8 * bytes, size_t len)
{
	if ((len % sizeof(u32)) != 0)
	{
		throw fnd::Exception(kModuleName, "KernelCapability list must be aligned to 4 bytes");
	}

	fnd::List<KernelCapability> threadInfoCaps;
	fnd::List<KernelCapability> systemCallCaps;
	fnd::List<KernelCapability> memoryMapCaps;
	fnd::List<KernelCapability> interuptCaps;
	fnd::List<KernelCapability> miscParamCaps;
	fnd::List<KernelCapability> kernelVersionCaps;
	fnd::List<KernelCapability> handleTableSizeCaps;
	fnd::List<KernelCapability> miscFlagsCaps;

	const u32* raw_caps = (const u32*)bytes;
	size_t cap_num = len / sizeof(u32);
	KernelCapability cap;
	for (size_t i = 0; i < cap_num; i++)
	{
		cap.setCap(le_word(raw_caps[i]));
		switch (cap.getType())
		{
			case (KernelCapability::KC_THREAD_INFO) :
				threadInfoCaps.addElement(cap);
				break;
			case (KernelCapability::KC_ENABLE_SYSTEM_CALLS):
				systemCallCaps.addElement(cap);
				break;
			case (KernelCapability::KC_MEMORY_MAP):
			case (KernelCapability::KC_IO_MEMORY_MAP):
				memoryMapCaps.addElement(cap);
				break;
			case (KernelCapability::KC_ENABLE_INTERUPTS):
				interuptCaps.addElement(cap);
				break;
			case (KernelCapability::KC_MISC_PARAMS):
				miscParamCaps.addElement(cap);
				break;
			case (KernelCapability::KC_KERNEL_VERSION):
				kernelVersionCaps.addElement(cap);
				break;
			case (KernelCapability::KC_HANDLE_TABLE_SIZE):
				handleTableSizeCaps.addElement(cap);
				break;
			case (KernelCapability::KC_MISC_FLAGS):
				miscFlagsCaps.addElement(cap);
				break;
			default:
				throw fnd::Exception(kModuleName, "Unsupported kernel capability type");
		}
	}

	mThreadInfo.importKernelCapabilityList(threadInfoCaps);
	mSystemCalls.importKernelCapabilityList(systemCallCaps);
	mMemoryMap.importKernelCapabilityList(memoryMapCaps);
	mInterupts.importKernelCapabilityList(interuptCaps);
	mMiscParams.importKernelCapabilityList(miscParamCaps);
	mKernelVersion.importKernelCapabilityList(kernelVersionCaps);
	mHandleTableSize.importKernelCapabilityList(handleTableSizeCaps);
	mMiscFlags.importKernelCapabilityList(miscFlagsCaps);
}

void nx::KcBinary::clear()
{
	mBinaryBlob.clear();
	mThreadInfo.clear();
	mSystemCalls.clear();
	mMemoryMap.clear();
	mInterupts.clear();
	mMiscParams.clear();
	mKernelVersion.clear();
	mHandleTableSize.clear();
	mMiscFlags.clear();
}

const nx::ThreadInfoHandler & nx::KcBinary::getThreadInfo() const
{
	return mThreadInfo;
}

nx::ThreadInfoHandler & nx::KcBinary::getThreadInfo()
{
	return mThreadInfo;
}

const nx::SystemCallHandler & nx::KcBinary::getSystemCalls() const
{
	return mSystemCalls;
}

nx::SystemCallHandler & nx::KcBinary::getSystemCalls()
{
	return mSystemCalls;
}

const nx::MemoryMappingHandler & nx::KcBinary::getMemoryMaps() const
{
	return mMemoryMap;
}

nx::MemoryMappingHandler & nx::KcBinary::getMemoryMaps()
{
	return mMemoryMap;
}

const nx::InteruptHandler & nx::KcBinary::getInterupts() const
{
	return mInterupts;
}

nx::InteruptHandler & nx::KcBinary::getInterupts()
{
	return mInterupts;
}

const nx::MiscParamsHandler & nx::KcBinary::getMiscParams() const
{
	return mMiscParams;
}

nx::MiscParamsHandler & nx::KcBinary::getMiscParams()
{
	return mMiscParams;
}

const nx::KernelVersionHandler & nx::KcBinary::getKernelVersion() const
{
	return mKernelVersion;
}

nx::KernelVersionHandler & nx::KcBinary::getKernelVersion()
{
	return mKernelVersion;
}

const nx::HandleTableSizeHandler & nx::KcBinary::getHandleTableSize() const
{
	return mHandleTableSize;
}

nx::HandleTableSizeHandler & nx::KcBinary::getHandleTableSize()
{
	return mHandleTableSize;
}

const nx::MiscFlagsHandler & nx::KcBinary::getMiscFlags() const
{
	return mMiscFlags;
}

nx::MiscFlagsHandler & nx::KcBinary::getMiscFlags()
{
	return mMiscFlags;
}

bool nx::KcBinary::isEqual(const KcBinary & other) const
{
	return (mThreadInfo == other.mThreadInfo) \
		&& (mSystemCalls == other.mSystemCalls) \
		&& (mMemoryMap == other.mMemoryMap) \
		&& (mInterupts == other.mInterupts) \
		&& (mMiscParams == other.mMiscParams) \
		&& (mKernelVersion == other.mKernelVersion) \
		&& (mHandleTableSize == other.mHandleTableSize) \
		&& (mMiscFlags == other.mMiscFlags);
}

void nx::KcBinary::copyFrom(const KcBinary & other)
{
	mThreadInfo = other.mThreadInfo;
	mSystemCalls = other.mSystemCalls;
	mMemoryMap = other.mMemoryMap;
	mInterupts = other.mInterupts;
	mMiscParams = other.mMiscParams;
	mKernelVersion = other.mKernelVersion;
	mHandleTableSize = other.mHandleTableSize;
	mMiscFlags = other.mMiscFlags;
}
