#include <nn/hac/KernelInitialProcessHeader.h>

nn::hac::KernelInitialProcessHeader::KernelInitialProcessHeader()
{
	clear();
}

nn::hac::KernelInitialProcessHeader::KernelInitialProcessHeader(const KernelInitialProcessHeader& other)
{
	*this = other;
}

void nn::hac::KernelInitialProcessHeader::operator=(const KernelInitialProcessHeader& other)
{
	clear();
	this->mName = other.mName;
	this->mTitleId = other.mTitleId;
	this->mProcessCategory = other.mProcessCategory;
	this->mFlagList = other.mFlagList;
	this->mMainThreadPriority = other.mMainThreadPriority;
	this->mMainThreadCpuId = other.mMainThreadCpuId;
	this->mMainThreadStackSize = other.mMainThreadStackSize;
	this->mTextInfo = other.mTextInfo;
	this->mRoInfo = other.mRoInfo;
	this->mDataInfo = other.mDataInfo;
	this->mBssSize = other.mBssSize;
	this->mKernelCapabilities = other.mKernelCapabilities;
}

bool nn::hac::KernelInitialProcessHeader::operator==(const KernelInitialProcessHeader& other) const
{
	return (this->mName == other.mName) \
		&& (this->mTitleId == other.mTitleId) \
		&& (this->mProcessCategory == other.mProcessCategory) \
		&& (this->mFlagList == other.mFlagList) \
		&& (this->mMainThreadPriority == other.mMainThreadPriority) \
		&& (this->mMainThreadCpuId == other.mMainThreadCpuId) \
		&& (this->mMainThreadStackSize == other.mMainThreadStackSize) \
		&& (this->mTextInfo == other.mTextInfo) \
		&& (this->mRoInfo == other.mRoInfo) \
		&& (this->mDataInfo == other.mDataInfo) \
		&& (this->mBssSize == other.mBssSize) \
		&& (this->mKernelCapabilities == other.mKernelCapabilities);
}

bool nn::hac::KernelInitialProcessHeader::operator!=(const KernelInitialProcessHeader& other) const
{
	return !(*this == other);
}

void nn::hac::KernelInitialProcessHeader::toBytes()
{
	mRawBinary.alloc(sizeof(sKipHeader));
	nn::hac::sKipHeader* hdr = (nn::hac::sKipHeader*)mRawBinary.data();

	// set header identifers
	hdr->st_magic = kip::kKipStructMagic;
	
	// variable to store flags before commiting to header
	byte_t flags = 0;

	// properties
	strncpy(hdr->name, mName.c_str(), kip::kNameMaxLen);
	hdr->title_id = mTitleId;
	hdr->process_category = mProcessCategory;
	hdr->main_thread_priority = mMainThreadPriority;
	hdr->main_thread_cpu_id = mMainThreadCpuId;
	hdr->main_thread_stack_size = mMainThreadStackSize;

	// kernel caps
	mKernelCapabilities.toBytes();
	if (mKernelCapabilities.getBytes().size() > kip::kKernCapabilitySize)
	{
		throw fnd::Exception(kModuleName, "Too many kernel capabilities");
	}
	memcpy(hdr->capabilities, mKernelCapabilities.getBytes().data(), mKernelCapabilities.getBytes().size());
	
	// stub remaining entries
	if (mKernelCapabilities.getBytes().size() <  kip::kKernCapabilitySize)
	{
		memset(hdr->capabilities + mKernelCapabilities.getBytes().size(), 0xff, kip::kKernCapabilitySize - mKernelCapabilities.getBytes().size());
	}	

	// flags
	for (size_t i = 0; i < mFlagList.size(); i++)
	{
		switch(mFlagList[i])
		{
			case (kip::FLAG_TEXT_COMPRESS) :
			case (kip::FLAG_RO_COMPRESS) :
			case (kip::FLAG_DATA_COMPRESS) :
				break;
			default:
				flags |= _BIT(mFlagList[i] & 7);
				break;
		}
	}

	// set bss size
	hdr->bss.file_size = 0;
	hdr->bss.memory_offset = 0;
	hdr->bss.memory_size = mBssSize;

	// set text segment
	hdr->text.memory_offset = mTextInfo.memory_layout.offset;
	hdr->text.memory_size = mTextInfo.memory_layout.size;
	hdr->text.file_size = mTextInfo.file_layout.size;
	if (mTextInfo.is_compressed)
	{
		flags |= _BIT(kip::FLAG_TEXT_COMPRESS);
	}

	// set ro segment
	hdr->ro.memory_offset = mRoInfo.memory_layout.offset;
	hdr->ro.memory_size = mRoInfo.memory_layout.size;
	hdr->ro.file_size = mRoInfo.file_layout.size;
	if (mRoInfo.is_compressed)
	{
		flags |= _BIT(kip::FLAG_TEXT_COMPRESS);
	}

	// set data segment
	hdr->data.memory_offset = mDataInfo.memory_layout.offset;
	hdr->data.memory_size = mDataInfo.memory_layout.size;
	hdr->data.file_size = mDataInfo.file_layout.size;
	if (mDataInfo.is_compressed)
	{
		flags |= _BIT(kip::FLAG_TEXT_COMPRESS);
	}

	hdr->flags = flags;
}

void nn::hac::KernelInitialProcessHeader::fromBytes(const byte_t* data, size_t len)
{
	// check input data size
	if (len < sizeof(sKipHeader))
	{
		throw fnd::Exception(kModuleName, "KIP header size is too small");
	}

	// clear internal members
	clear();

	// allocate internal local binary copy
	mRawBinary.alloc(sizeof(sKipHeader));
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	// get sKipHeader ptr
	const nn::hac::sKipHeader* hdr = (const nn::hac::sKipHeader*)mRawBinary.data();
	
	// check KIP signature
	if (hdr->st_magic.get() != kip::kKipStructMagic)
	{
		throw fnd::Exception(kModuleName, "KIP header corrupt (unrecognised header signature)");
	}

	// properties
	if (hdr->name[0] != 0)
		mName = std::string(hdr->name, _MIN(strlen(hdr->name), kip::kNameMaxLen));
	mTitleId = hdr->title_id.get();
	mProcessCategory = (kip::ProcessCategory)hdr->process_category.get();
	mMainThreadPriority = hdr->main_thread_priority;
	mMainThreadCpuId = hdr->main_thread_cpu_id;
	mMainThreadStackSize = hdr->main_thread_stack_size.get();
	mKernelCapabilities.fromBytes(hdr->capabilities, kip::kKernCapabilitySize);
	
	for (byte_t i = 0; i < 8; i++)
	{
		if (_HAS_BIT(hdr->flags, i))
		{
			switch(i)
			{
			case (kip::FLAG_TEXT_COMPRESS) :
			case (kip::FLAG_RO_COMPRESS) :
			case (kip::FLAG_DATA_COMPRESS) :
				break;
			default:
				mFlagList.addElement((kip::HeaderFlags)i);
				break;
			}
		}
	}

	// code segment info
	mTextInfo.file_layout.offset = sizeof(sKipHeader);
	mTextInfo.file_layout.size = hdr->text.file_size.get();
	mTextInfo.memory_layout.offset = hdr->text.memory_offset.get();
	mTextInfo.memory_layout.size = hdr->text.memory_size.get();
	mTextInfo.is_compressed = _HAS_BIT(hdr->flags, kip::FLAG_TEXT_COMPRESS);

	mRoInfo.file_layout.offset = mTextInfo.file_layout.offset + mTextInfo.file_layout.size;
	mRoInfo.file_layout.size = hdr->ro.file_size.get();
	mRoInfo.memory_layout.offset = hdr->ro.memory_offset.get();
	mRoInfo.memory_layout.size = hdr->ro.memory_size.get();
	mRoInfo.is_compressed = _HAS_BIT(hdr->flags, kip::FLAG_RO_COMPRESS);

	mDataInfo.file_layout.offset = mRoInfo.file_layout.offset + mRoInfo.file_layout.size;
	mDataInfo.file_layout.size = hdr->data.file_size.get();
	mDataInfo.memory_layout.offset = hdr->data.memory_offset.get();
	mDataInfo.memory_layout.size = hdr->data.memory_size.get();
	mDataInfo.is_compressed = _HAS_BIT(hdr->flags, kip::FLAG_DATA_COMPRESS);

	mBssSize = hdr->bss.memory_size.get();
}

const fnd::Vec<byte_t>& nn::hac::KernelInitialProcessHeader::getBytes() const
{
	return mRawBinary;
}

void nn::hac::KernelInitialProcessHeader::clear()
{
	mRawBinary.clear();
	mName.clear();
	mTitleId = 0;
	mProcessCategory = (kip::ProcessCategory)0;
	mFlagList.clear();
	mMainThreadPriority = 0;
	mMainThreadCpuId = 0;
	mMainThreadStackSize = 0;
	mTextInfo = sCodeSegment();
	mRoInfo = sCodeSegment();
	mDataInfo = sCodeSegment();
	mBssSize = 0;
	mKernelCapabilities.clear();;
}

const std::string& nn::hac::KernelInitialProcessHeader::getName() const
{
	return mName;
}

void nn::hac::KernelInitialProcessHeader::setName(const std::string& name)
{
	mName = name;
}

uint64_t nn::hac::KernelInitialProcessHeader::getTitleId() const
{
	return mTitleId;
}

void nn::hac::KernelInitialProcessHeader::setTitleId(uint64_t title_id)
{
	mTitleId = title_id;
}

nn::hac::kip::ProcessCategory nn::hac::KernelInitialProcessHeader::getProcessCategory() const
{
	return mProcessCategory;
}

void nn::hac::KernelInitialProcessHeader::setProcessCategory(kip::ProcessCategory cat)
{
	mProcessCategory = cat;
}

const fnd::List<nn::hac::kip::HeaderFlags>& nn::hac::KernelInitialProcessHeader::getFlagList() const
{
	return mFlagList;
}
void nn::hac::KernelInitialProcessHeader::setFlagList(const fnd::List<kip::HeaderFlags>& flags)
{
	mFlagList = flags;
}

byte_t nn::hac::KernelInitialProcessHeader::getMainThreadPriority() const
{
	return mMainThreadPriority;
}

void nn::hac::KernelInitialProcessHeader::setMainThreadPriority(byte_t priority)
{
	mMainThreadPriority = priority;
}

byte_t nn::hac::KernelInitialProcessHeader::getMainThreadCpuId() const
{
	return mMainThreadCpuId;
}

void nn::hac::KernelInitialProcessHeader::setMainThreadCpuId(byte_t cpu_id)
{
	mMainThreadCpuId = cpu_id;
}

uint32_t nn::hac::KernelInitialProcessHeader::getMainThreadStackSize() const
{
	return mMainThreadStackSize;
}

void nn::hac::KernelInitialProcessHeader::setMainThreadStackSize(uint32_t size)
{
	mMainThreadStackSize = size;
}

const nn::hac::KernelInitialProcessHeader::sCodeSegment& nn::hac::KernelInitialProcessHeader::getTextSegmentInfo() const
{
	return mTextInfo;
}

void nn::hac::KernelInitialProcessHeader::setTextSegmentInfo(const sCodeSegment& info)
{
	mTextInfo = info;
}

const nn::hac::KernelInitialProcessHeader::sCodeSegment& nn::hac::KernelInitialProcessHeader::getRoSegmentInfo() const
{
	return mRoInfo;
}

void nn::hac::KernelInitialProcessHeader::setRoSegmentInfo(const sCodeSegment& info)
{
	mRoInfo = info;
}

const nn::hac::KernelInitialProcessHeader::sCodeSegment& nn::hac::KernelInitialProcessHeader::getDataSegmentInfo() const
{
	return mDataInfo;
}

void nn::hac::KernelInitialProcessHeader::setDataSegmentInfo(const sCodeSegment& info)
{
	mDataInfo = info;
}

uint32_t nn::hac::KernelInitialProcessHeader::getBssSize() const
{
	return mBssSize;
}

void nn::hac::KernelInitialProcessHeader::setBssSize(uint32_t size)
{
	mBssSize = size;
}

const nn::hac::KernelCapabilityControl& nn::hac::KernelInitialProcessHeader::getKernelCapabilities() const
{
	return mKernelCapabilities;
}

void nn::hac::KernelInitialProcessHeader::setKernelCapabilities(const KernelCapabilityControl& kc)
{
	mKernelCapabilities = kc;
}