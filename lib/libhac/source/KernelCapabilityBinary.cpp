#include <nn/hac/KernelCapabilityBinary.h>

nn::hac::KernelCapabilityBinary::KernelCapabilityBinary()
{}

nn::hac::KernelCapabilityBinary::KernelCapabilityBinary(const KernelCapabilityBinary & other)
{
	*this = other;
}

void nn::hac::KernelCapabilityBinary::operator=(const KernelCapabilityBinary & other)
{
	clear();
	mRawBinary = other.mRawBinary;
	mThreadInfo = other.mThreadInfo;
	mSystemCalls = other.mSystemCalls;
	mMemoryMap = other.mMemoryMap;
	mInterupts = other.mInterupts;
	mMiscParams = other.mMiscParams;
	mKernelVersion = other.mKernelVersion;
	mHandleTableSize = other.mHandleTableSize;
	mMiscFlags = other.mMiscFlags;
}

bool nn::hac::KernelCapabilityBinary::operator==(const KernelCapabilityBinary & other) const
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

bool nn::hac::KernelCapabilityBinary::operator!=(const KernelCapabilityBinary & other) const
{
	return !(*this == other);
}

void nn::hac::KernelCapabilityBinary::toBytes()
{
	fnd::List<KernelCapabilityEntry> caps;

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
	mRawBinary.alloc(caps.size() * sizeof(uint32_t));

	// write to binary
	uint32_t* raw_caps = (uint32_t*)mRawBinary.data();
	for (size_t i = 0; i < caps.size(); i++)
	{
		raw_caps[i] = le_word(caps[i].getCap());
	}
}

void nn::hac::KernelCapabilityBinary::fromBytes(const byte_t * data, size_t len)
{
	if ((len % sizeof(uint32_t)) != 0)
	{
		throw fnd::Exception(kModuleName, "KernelCapabilityEntry list must be aligned to 4 bytes");
	}

	// save copy of KernelCapabilityBinary
	mRawBinary.alloc(len);
	memcpy(mRawBinary.data(), data, len);

	fnd::List<KernelCapabilityEntry> threadInfoCaps;
	fnd::List<KernelCapabilityEntry> systemCallCaps;
	fnd::List<KernelCapabilityEntry> memoryMapCaps;
	fnd::List<KernelCapabilityEntry> interuptCaps;
	fnd::List<KernelCapabilityEntry> miscParamCaps;
	fnd::List<KernelCapabilityEntry> kernelVersionCaps;
	fnd::List<KernelCapabilityEntry> handleTableSizeCaps;
	fnd::List<KernelCapabilityEntry> miscFlagsCaps;

	const uint32_t* raw_caps = (const uint32_t*)mRawBinary.data();
	size_t cap_num = mRawBinary.size() / sizeof(uint32_t);
	KernelCapabilityEntry cap;
	for (size_t i = 0; i < cap_num; i++)
	{
		cap.setCap(le_word(raw_caps[i]));
		switch (cap.getType())
		{
			case (kc::KC_THREAD_INFO) :
				threadInfoCaps.addElement(cap);
				break;
			case (kc::KC_ENABLE_SYSTEM_CALLS):
				systemCallCaps.addElement(cap);
				break;
			case (kc::KC_MEMORY_MAP):
			case (kc::KC_IO_MEMORY_MAP):
				memoryMapCaps.addElement(cap);
				break;
			case (kc::KC_ENABLE_INTERUPTS):
				interuptCaps.addElement(cap);
				break;
			case (kc::KC_MISC_PARAMS):
				miscParamCaps.addElement(cap);
				break;
			case (kc::KC_KERNEL_VERSION):
				kernelVersionCaps.addElement(cap);
				break;
			case (kc::KC_HANDLE_TABLE_SIZE):
				handleTableSizeCaps.addElement(cap);
				break;
			case (kc::KC_MISC_FLAGS):
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

const fnd::Vec<byte_t>& nn::hac::KernelCapabilityBinary::getBytes() const
{
	return mRawBinary;
}

void nn::hac::KernelCapabilityBinary::clear()
{
	mRawBinary.clear();
	mThreadInfo.clear();
	mSystemCalls.clear();
	mMemoryMap.clear();
	mInterupts.clear();
	mMiscParams.clear();
	mKernelVersion.clear();
	mHandleTableSize.clear();
	mMiscFlags.clear();
}

const nn::hac::ThreadInfoHandler & nn::hac::KernelCapabilityBinary::getThreadInfo() const
{
	return mThreadInfo;
}

nn::hac::ThreadInfoHandler & nn::hac::KernelCapabilityBinary::getThreadInfo()
{
	return mThreadInfo;
}

const nn::hac::SystemCallHandler & nn::hac::KernelCapabilityBinary::getSystemCalls() const
{
	return mSystemCalls;
}

nn::hac::SystemCallHandler & nn::hac::KernelCapabilityBinary::getSystemCalls()
{
	return mSystemCalls;
}

const nn::hac::MemoryMappingHandler & nn::hac::KernelCapabilityBinary::getMemoryMaps() const
{
	return mMemoryMap;
}

nn::hac::MemoryMappingHandler & nn::hac::KernelCapabilityBinary::getMemoryMaps()
{
	return mMemoryMap;
}

const nn::hac::InteruptHandler & nn::hac::KernelCapabilityBinary::getInterupts() const
{
	return mInterupts;
}

nn::hac::InteruptHandler & nn::hac::KernelCapabilityBinary::getInterupts()
{
	return mInterupts;
}

const nn::hac::MiscParamsHandler & nn::hac::KernelCapabilityBinary::getMiscParams() const
{
	return mMiscParams;
}

nn::hac::MiscParamsHandler & nn::hac::KernelCapabilityBinary::getMiscParams()
{
	return mMiscParams;
}

const nn::hac::KernelVersionHandler & nn::hac::KernelCapabilityBinary::getKernelVersion() const
{
	return mKernelVersion;
}

nn::hac::KernelVersionHandler & nn::hac::KernelCapabilityBinary::getKernelVersion()
{
	return mKernelVersion;
}

const nn::hac::HandleTableSizeHandler & nn::hac::KernelCapabilityBinary::getHandleTableSize() const
{
	return mHandleTableSize;
}

nn::hac::HandleTableSizeHandler & nn::hac::KernelCapabilityBinary::getHandleTableSize()
{
	return mHandleTableSize;
}

const nn::hac::MiscFlagsHandler & nn::hac::KernelCapabilityBinary::getMiscFlags() const
{
	return mMiscFlags;
}

nn::hac::MiscFlagsHandler & nn::hac::KernelCapabilityBinary::getMiscFlags()
{
	return mMiscFlags;
}