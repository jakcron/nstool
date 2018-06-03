#include <nx/NpdmHeader.h>



nx::NpdmHeader::NpdmHeader()
{
	clear();
}

nx::NpdmHeader::NpdmHeader(const NpdmHeader & other)
{
	copyFrom(other);
}

nx::NpdmHeader::NpdmHeader(const byte_t * bytes, size_t len)
{
	importBinary(bytes, len);
}

bool nx::NpdmHeader::operator==(const NpdmHeader & other) const
{
	return isEqual(other);
}

bool nx::NpdmHeader::operator!=(const NpdmHeader & other) const
{
	return isEqual(other);
}

void nx::NpdmHeader::operator=(const NpdmHeader & other)
{
	copyFrom(other);
}

const byte_t * nx::NpdmHeader::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::NpdmHeader::getSize() const
{
	return mBinaryBlob.getSize();
}

void nx::NpdmHeader::calculateOffsets()
{
	mAcidPos.offset = align(sizeof(sNpdmHeader), npdm::kNpdmAlignSize);
	mAciPos.offset = mAcidPos.offset + align(mAcidPos.size, npdm::kNpdmAlignSize);
}

bool nx::NpdmHeader::isEqual(const NpdmHeader & other) const
{
	return (mInstructionType == other.mInstructionType) \
		&& (mProcAddressSpaceType == other.mProcAddressSpaceType) \
		&& (mMainThreadPriority == other.mMainThreadPriority) \
		&& (mMainThreadCpuId == other.mMainThreadCpuId) \
		&& (mVersion == other.mVersion) \
		&& (mMainThreadStackSize == other.mMainThreadStackSize) \
		&& (mName == other.mName) \
		&& (mProductCode == other.mProductCode) \
		&& (mAciPos == other.mAciPos) \
		&& (mAcidPos == other.mAcidPos);
}

void nx::NpdmHeader::copyFrom(const NpdmHeader & other)
{
	if (other.getSize())
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		mInstructionType = other.mInstructionType;
		mProcAddressSpaceType = other.mProcAddressSpaceType;
		mMainThreadPriority = other.mMainThreadPriority;
		mMainThreadCpuId = other.mMainThreadCpuId;
		mVersion = other.mVersion;
		mMainThreadStackSize = other.mMainThreadStackSize;
		mName = other.mName;
		mProductCode = other.mProductCode;
		mAciPos = other.mAciPos;
		mAcidPos = other.mAcidPos;
	}
}

void nx::NpdmHeader::exportBinary()
{
	mBinaryBlob.alloc(sizeof(sNpdmHeader));
	sNpdmHeader* hdr = (sNpdmHeader*)mBinaryBlob.getBytes();

	hdr->signature = npdm::kNpdmStructSig;
	byte_t flag = ((byte_t)(mInstructionType & 1) | (byte_t)((mProcAddressSpaceType & 3) << 1)) & 0xf;
	hdr->flags = flag;
	hdr->main_thread_priority = mMainThreadPriority;
	hdr->main_thread_cpu_id = mMainThreadCpuId;
	hdr->version = mVersion;
	hdr->main_thread_stack_size = mMainThreadStackSize;
	strncpy(hdr->name, mName.c_str(), npdm::kNameMaxLen);
	strncpy(hdr->product_code, mProductCode.c_str(), npdm::kProductCodeMaxLen);
	
	calculateOffsets();
	hdr->aci.offset = (uint32_t)mAciPos.offset;
	hdr->aci.size = (uint32_t)mAciPos.size;
	hdr->acid.offset = (uint32_t)mAcidPos.offset;
	hdr->acid.size = (uint32_t)mAcidPos.size;
}

void nx::NpdmHeader::importBinary(const byte_t * bytes, size_t len)
{
	if (len < sizeof(sNpdmHeader))
	{
		throw fnd::Exception(kModuleName, "NPDM header too small");
	}
	
	clear();

	mBinaryBlob.alloc(sizeof(sNpdmHeader));
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());
	sNpdmHeader* hdr = (sNpdmHeader*)mBinaryBlob.getBytes();

	if (hdr->signature.get() != npdm::kNpdmStructSig)
	{
		throw fnd::Exception(kModuleName, "NPDM header corrupt");
	}

	byte_t flag = hdr->flags & 0xf;
	mInstructionType = (npdm::InstructionType)(flag & 1);
	mProcAddressSpaceType = (npdm::ProcAddrSpaceType)((flag >> 1) & 3);
	mMainThreadPriority = hdr->main_thread_priority;
	mMainThreadCpuId = hdr->main_thread_cpu_id;
	mVersion = hdr->version.get();
	mMainThreadStackSize = hdr->main_thread_stack_size.get();
	mName = std::string(hdr->name, npdm::kNameMaxLen);
	if (mName[0] == '\0')
	{
		mName.clear();
	}
	mProductCode = std::string(hdr->product_code, npdm::kProductCodeMaxLen);
	if (mProductCode[0] == '\0')
	{
		mProductCode.clear();
	}
	mAciPos.offset = hdr->aci.offset.get();
	mAciPos.size = hdr->aci.size.get();
	mAcidPos.offset = hdr->acid.offset.get();
	mAcidPos.size = hdr->acid.size.get();
}

void nx::NpdmHeader::clear()
{
	mBinaryBlob.clear();
	mInstructionType = npdm::INSTR_64BIT;
	mProcAddressSpaceType = npdm::ADDR_SPACE_64BIT;
	mMainThreadPriority = 0;
	mMainThreadCpuId = 0;
	mVersion = 0;
	mMainThreadStackSize = 0;
	mName.clear();
	mProductCode.clear();
	mAciPos.offset = 0;
	mAciPos.size = 0;
	mAcidPos.offset = 0;
	mAcidPos.size = 0;
}

size_t nx::NpdmHeader::getNpdmSize() const
{
	return MAX(mAcidPos.offset + mAcidPos.size, mAciPos.offset + mAciPos.size);
}

nx::npdm::InstructionType nx::NpdmHeader::getInstructionType() const
{
	return mInstructionType;
}

void nx::NpdmHeader::setInstructionType(npdm::InstructionType type)
{
	mInstructionType = type;
}

nx::npdm::ProcAddrSpaceType nx::NpdmHeader::getProcAddressSpaceType() const
{
	return mProcAddressSpaceType;
}

void nx::NpdmHeader::setProcAddressSpaceType(npdm::ProcAddrSpaceType type)
{
	mProcAddressSpaceType = type;
}

byte_t nx::NpdmHeader::getMainThreadPriority() const
{
	return mMainThreadPriority;
}

void nx::NpdmHeader::setMainThreadPriority(byte_t priority)
{
	if (priority > npdm::kMaxPriority)
	{
		throw fnd::Exception(kModuleName, "Illegal main thread priority (range 0-63)");
	}

	mMainThreadPriority = priority;
}

byte_t nx::NpdmHeader::getMainThreadCpuId() const
{
	return mMainThreadCpuId;
}

void nx::NpdmHeader::setMainThreadCpuId(byte_t core_num)
{
	mMainThreadCpuId = core_num;
}

uint32_t nx::NpdmHeader::getVersion() const
{
	return mVersion;
}

void nx::NpdmHeader::setVersion(uint32_t version)
{
	mVersion = version;
}

uint32_t nx::NpdmHeader::getMainThreadStackSize() const
{
	return mMainThreadStackSize;
}

void nx::NpdmHeader::setMainThreadStackSize(uint32_t size)
{
	mMainThreadStackSize = size;
}

const std::string & nx::NpdmHeader::getName() const
{
	return mName;
}

void nx::NpdmHeader::setName(const std::string & name)
{
	if (name.length() > npdm::kNameMaxLen)
	{
		throw fnd::Exception(kModuleName, "Name is too long");
	}

	mName = name;
}

const std::string & nx::NpdmHeader::getProductCode() const
{
	return mProductCode;
}

void nx::NpdmHeader::setProductCode(const std::string & product_code)
{
	if (product_code.length() > npdm::kProductCodeMaxLen)
	{
		throw fnd::Exception(kModuleName, "Product Code is too long");
	}

	mProductCode = product_code;
}

const nx::NpdmHeader::sSection & nx::NpdmHeader::getAciPos() const
{
	return mAciPos;
}

void nx::NpdmHeader::setAciSize(size_t size)
{
	mAciPos.size = size;
}

const nx::NpdmHeader::sSection & nx::NpdmHeader::getAcidPos() const
{
	return mAcidPos;
}

void nx::NpdmHeader::setAcidSize(size_t size)
{
	mAcidPos.size = size;
}
