#include "NpdmHeader.h"



nx::NpdmHeader::NpdmHeader()
{
	clearVariables();
}

nx::NpdmHeader::NpdmHeader(const NpdmHeader & other)
{
	copyFrom(other);
}

nx::NpdmHeader::NpdmHeader(const u8 * bytes, size_t len)
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

const u8 * nx::NpdmHeader::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t nx::NpdmHeader::getSize() const
{
	return mBinaryBlob.getSize();
}

void nx::NpdmHeader::clearVariables()
{
	mBinaryBlob.clear();
	mInstructionType = INSTR_64BIT;
	mProcAddressSpaceType = ADDR_SPACE_64BIT;
	mMainThreadPriority = 0;
	mMainThreadCoreNumber = 0;
	mVersion = 0;
	mMainThreadStackSize = 0;
	mName.clear();
	mProductCode.clear();
	mAciPos.offset = 0;
	mAciPos.size = 0;
	mAcidPos.offset = 0;
	mAcidPos.size = 0;
}

void nx::NpdmHeader::calculateOffsets()
{
	mAcidPos.offset = align(sizeof(sNpdmHeader), kNpdmAlignSize);
	mAciPos.offset = mAcidPos.offset + align(mAcidPos.size, kNpdmAlignSize);
}

bool nx::NpdmHeader::isEqual(const NpdmHeader & other) const
{
	return (mInstructionType == other.mInstructionType) \
		&& (mProcAddressSpaceType == other.mProcAddressSpaceType) \
		&& (mMainThreadPriority == other.mMainThreadPriority) \
		&& (mMainThreadCoreNumber == other.mMainThreadCoreNumber) \
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
		mMainThreadCoreNumber = other.mMainThreadCoreNumber;
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

	hdr->set_signature(kNpdmStructSig.c_str());
	u8 flag = ((u8)(mInstructionType & 1) | (u8)((mProcAddressSpaceType & 3) << 1)) & 0xf;
	hdr->set_flags(flag);
	hdr->set_main_thread_priority(mMainThreadPriority);
	hdr->set_main_thread_core_number(mMainThreadCoreNumber);
	hdr->set_version(mVersion);
	hdr->set_main_thread_stack_size(mMainThreadStackSize);
	hdr->set_name(mName.c_str());
	hdr->set_product_code(mProductCode.c_str());
	
	calculateOffsets();
	hdr->aci().set_offset(mAciPos.offset);
	hdr->aci().set_size(mAciPos.size);
	hdr->acid().set_offset(mAcidPos.offset);
	hdr->acid().set_size(mAcidPos.size);
}

void nx::NpdmHeader::importBinary(const u8 * bytes, size_t len)
{
	if (len < sizeof(sNpdmHeader))
	{
		throw fnd::Exception(kModuleName, "NPDM header too small");
	}
	
	clearVariables();

	mBinaryBlob.alloc(sizeof(sNpdmHeader));
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());
	sNpdmHeader* hdr = (sNpdmHeader*)mBinaryBlob.getBytes();

	if (memcmp(kNpdmStructSig.c_str(), hdr->signature(), 4) != 0)
	{
		throw fnd::Exception(kModuleName, "NPDM header corrupt");
	}

	u8 flag = hdr->flags() & 0xf;
	mInstructionType = (InstructionType)(flag & 1);
	mProcAddressSpaceType = (ProcAddrSpaceType)((flag >> 1) & 3);
	mMainThreadPriority = hdr->main_thread_priority();
	mMainThreadCoreNumber = hdr->main_thread_core_number();
	mVersion = hdr->version();
	mMainThreadStackSize = hdr->main_thread_stack_size();
	mName = std::string(hdr->name(), kNameMaxLen);
	mProductCode = std::string(hdr->product_code(), kProductCodeMaxLen);
	mAciPos.offset = hdr->aci().offset();
	mAciPos.size = hdr->aci().size();
	mAcidPos.offset = hdr->acid().offset();
	mAcidPos.size = hdr->acid().size();
}

void nx::NpdmHeader::clear()
{
	clearVariables();
}

size_t nx::NpdmHeader::getNpdmSize() const
{
	return MAX(mAcidPos.offset + mAcidPos.size, mAciPos.offset + mAciPos.size);
}

nx::NpdmHeader::InstructionType nx::NpdmHeader::getInstructionType() const
{
	return mInstructionType;
}

void nx::NpdmHeader::setInstructionType(InstructionType type)
{
	mInstructionType = type;
}

nx::NpdmHeader::ProcAddrSpaceType nx::NpdmHeader::getProcAddressSpaceType() const
{
	return mProcAddressSpaceType;
}

void nx::NpdmHeader::setProcAddressSpaceType(ProcAddrSpaceType type)
{
	mProcAddressSpaceType = type;
}

u8 nx::NpdmHeader::getMainThreadPriority() const
{
	return mMainThreadPriority;
}

void nx::NpdmHeader::setMainThreadPriority(u8 priority)
{
	if (priority > kMaxPriority)
	{
		throw fnd::Exception(kModuleName, "Illegal main thread priority (range 0-63)");
	}

	mMainThreadPriority = priority;
}

u8 nx::NpdmHeader::getMainThreadCoreNumber() const
{
	return mMainThreadCoreNumber;
}

void nx::NpdmHeader::setMainThreadCoreNumber(u8 core_num)
{
	mMainThreadCoreNumber = core_num;
}

u32 nx::NpdmHeader::getVersion() const
{
	return mVersion;
}

void nx::NpdmHeader::setVersion(u32 version)
{
	mVersion = version;
}

u32 nx::NpdmHeader::getMainThreadStackSize() const
{
	return mMainThreadStackSize;
}

void nx::NpdmHeader::setMainThreadStackSize(u32 size)
{
	mMainThreadStackSize = size;
}

const std::string & nx::NpdmHeader::getName() const
{
	return mName;
}

void nx::NpdmHeader::setName(const std::string & name)
{
	if (name.length() > kNameMaxLen)
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
	if (product_code.length() > kProductCodeMaxLen)
	{
		throw fnd::Exception(kModuleName, "Product Code is too long");
	}

	mProductCode = product_code;
}

const nx::NpdmHeader::sSection & nx::NpdmHeader::getAciPos() const
{
	return mAciPos;
}

void nx::NpdmHeader::setSetAciSize(size_t size)
{
	mAciPos.size = size;
}

const nx::NpdmHeader::sSection & nx::NpdmHeader::getAcidPos() const
{
	return mAcidPos;
}

void nx::NpdmHeader::setSetAcidSize(size_t size)
{
	mAcidPos.size = size;
}
