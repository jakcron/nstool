#include <nx/NpdmBinary.h>

#include <fnd/SimpleTextOutput.h>

nx::NpdmBinary::NpdmBinary()
{
	clear();
}

nx::NpdmBinary::NpdmBinary(const NpdmBinary & other) :
	NpdmBinary()
{
	*this = other;
}

void nx::NpdmBinary::operator=(const NpdmBinary & other)
{
	mRawBinary = other.mRawBinary;
	mInstructionType = other.mInstructionType;
	mProcAddressSpaceType = other.mProcAddressSpaceType;
	mMainThreadPriority = other.mMainThreadPriority;
	mMainThreadCpuId = other.mMainThreadCpuId;
	mVersion = other.mVersion;
	mMainThreadStackSize = other.mMainThreadStackSize;
	mName = other.mName;
	mProductCode = other.mProductCode;
	mAci = other.mAci;
	mAcid = other.mAcid;
}

bool nx::NpdmBinary::operator==(const NpdmBinary & other) const
{
	return (mInstructionType == other.mInstructionType) \
		&& (mProcAddressSpaceType == other.mProcAddressSpaceType) \
		&& (mMainThreadPriority == other.mMainThreadPriority) \
		&& (mMainThreadCpuId == other.mMainThreadCpuId) \
		&& (mVersion == other.mVersion) \
		&& (mMainThreadStackSize == other.mMainThreadStackSize) \
		&& (mName == other.mName) \
		&& (mProductCode == other.mProductCode) \
		&& (mAci == other.mAci) \
		&& (mAcid == other.mAcid);
}

bool nx::NpdmBinary::operator!=(const NpdmBinary & other) const
{
	return !(*this == other);
}

void nx::NpdmBinary::toBytes()
{
	if (mAcid.getBytes().size() == 0)
		mAcid.toBytes();

	if (mAci.getBytes().size() == 0)
		mAci.toBytes();


	// determine section layout
	struct sLayout {
		uint32_t offset, size;
	} acid, aci;

	acid.offset = (uint32_t)align(sizeof(sNpdmHeader), npdm::kSectionAlignSize);
	acid.size = (uint32_t)mAcid.getBytes().size();
	aci.offset = (uint32_t)(acid.offset + align(acid.size, npdm::kSectionAlignSize));
	aci.size = (uint32_t)mAci.getBytes().size();
	

	// get total size
	size_t total_size = _MAX(_MAX(acid.offset + acid.size, aci.offset + aci.size), align(sizeof(sNpdmHeader), npdm::kSectionAlignSize)); 

	mRawBinary.alloc(total_size);
	sNpdmHeader* hdr = (sNpdmHeader*)mRawBinary.data();

	// set type
	hdr->st_magic = npdm::kNpdmStructMagic;

	// set variables
	byte_t flag = ((byte_t)(mInstructionType & 1) | (byte_t)((mProcAddressSpaceType & 3) << 1)) & 0xf;
	hdr->flags = flag;
	hdr->main_thread_priority = mMainThreadPriority;
	hdr->main_thread_cpu_id = mMainThreadCpuId;
	hdr->version = mVersion;
	hdr->main_thread_stack_size = mMainThreadStackSize;
	strncpy(hdr->name, mName.c_str(), npdm::kNameMaxLen);
	strncpy(hdr->product_code, mProductCode.c_str(), npdm::kProductCodeMaxLen);

	// set offset/size
	hdr->aci.offset = aci.offset;
	hdr->aci.size = aci.size;
	hdr->acid.offset = acid.offset;
	hdr->acid.size = acid.size;

	// write aci & acid
	if (mAci.getBytes().size() > 0)
	{
		memcpy(mRawBinary.data() + aci.offset, mAci.getBytes().data(), mAci.getBytes().size());
	}
	if (mAcid.getBytes().size() > 0)
	{
		memcpy(mRawBinary.data() + acid.offset, mAcid.getBytes().data(), mAcid.getBytes().size());
	}
}

void nx::NpdmBinary::fromBytes(const byte_t* data, size_t len)
{
	// check size
	if (len < sizeof(sNpdmHeader))
	{
		throw fnd::Exception(kModuleName, "NPDM binary is too small");
	}
	
	// clear variables
	clear();

	// save a copy of the header
	sNpdmHeader hdr;
	memcpy((void*)&hdr, data, sizeof(sNpdmHeader));

	// check magic
	if (hdr.st_magic.get() != npdm::kNpdmStructMagic)
	{
		throw fnd::Exception(kModuleName, "NPDM header corrupt");
	}

	// save variables
	byte_t flag = hdr.flags & 0xf;
	mInstructionType = (npdm::InstructionType)(flag & 1);
	mProcAddressSpaceType = (npdm::ProcAddrSpaceType)((flag >> 1) & 3);
	mMainThreadPriority = hdr.main_thread_priority;
	mMainThreadCpuId = hdr.main_thread_cpu_id;
	mVersion = hdr.version.get();
	mMainThreadStackSize = hdr.main_thread_stack_size.get();
	mName = std::string(hdr.name, npdm::kNameMaxLen);
	if (mName[0] == '\0')
	{
		mName.clear();
	}
	mProductCode = std::string(hdr.product_code, npdm::kProductCodeMaxLen);
	if (mProductCode[0] == '\0')
	{
		mProductCode.clear();
	}

	// total size
	size_t total_size = _MAX(_MAX(hdr.acid.offset.get() + hdr.acid.size.get(), hdr.aci.offset.get() + hdr.aci.size.get()), sizeof(sNpdmHeader));

	// check size
	if (total_size > len)
	{
		throw fnd::Exception(kModuleName, "NPDM binary too small");
	}

	// save local copy
	mRawBinary.alloc(total_size);
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	// import Aci/Acid
	if (hdr.aci.size.get())
	{
		mAci.fromBytes(mRawBinary.data() + hdr.aci.offset.get(), hdr.aci.size.get());
	}
	if (hdr.acid.size.get())
	{
		mAcid.fromBytes(mRawBinary.data() + hdr.acid.offset.get(), hdr.acid.size.get());
	}	
}

const fnd::Vec<byte_t>& nx::NpdmBinary::getBytes() const
{
	return mRawBinary;
}

void nx::NpdmBinary::clear()
{
	mRawBinary.clear();
	mInstructionType = npdm::INSTR_64BIT;
	mProcAddressSpaceType = npdm::ADDR_SPACE_64BIT;
	mMainThreadPriority = 0;
	mMainThreadCpuId = 0;
	mVersion = 0;
	mMainThreadStackSize = 0;
	mName.clear();
	mProductCode.clear();
	mAci.clear();
	mAcid.clear();
}

nx::npdm::InstructionType nx::NpdmBinary::getInstructionType() const
{
	return mInstructionType;
}

void nx::NpdmBinary::setInstructionType(npdm::InstructionType type)
{
	mInstructionType = type;
}

nx::npdm::ProcAddrSpaceType nx::NpdmBinary::getProcAddressSpaceType() const
{
	return mProcAddressSpaceType;
}

void nx::NpdmBinary::setProcAddressSpaceType(npdm::ProcAddrSpaceType type)
{
	mProcAddressSpaceType = type;
}

byte_t nx::NpdmBinary::getMainThreadPriority() const
{
	return mMainThreadPriority;
}

void nx::NpdmBinary::setMainThreadPriority(byte_t priority)
{
	if (priority > npdm::kMaxPriority)
	{
		throw fnd::Exception(kModuleName, "Illegal main thread priority (range 0-63)");
	}

	mMainThreadPriority = priority;
}

byte_t nx::NpdmBinary::getMainThreadCpuId() const
{
	return mMainThreadCpuId;
}

void nx::NpdmBinary::setMainThreadCpuId(byte_t core_num)
{
	mMainThreadCpuId = core_num;
}

uint32_t nx::NpdmBinary::getVersion() const
{
	return mVersion;
}

void nx::NpdmBinary::setVersion(uint32_t version)
{
	mVersion = version;
}

uint32_t nx::NpdmBinary::getMainThreadStackSize() const
{
	return mMainThreadStackSize;
}

void nx::NpdmBinary::setMainThreadStackSize(uint32_t size)
{
	mMainThreadStackSize = size;
}

const std::string & nx::NpdmBinary::getName() const
{
	return mName;
}

void nx::NpdmBinary::setName(const std::string & name)
{
	if (name.length() > npdm::kNameMaxLen)
	{
		throw fnd::Exception(kModuleName, "Name is too long");
	}

	mName = name;
}

const std::string & nx::NpdmBinary::getProductCode() const
{
	return mProductCode;
}

void nx::NpdmBinary::setProductCode(const std::string & product_code)
{
	if (product_code.length() > npdm::kProductCodeMaxLen)
	{
		throw fnd::Exception(kModuleName, "Product Code is too long");
	}

	mProductCode = product_code;
}

const nx::AccessControlInfoBinary & nx::NpdmBinary::getAci() const
{
	return mAci;
}

void nx::NpdmBinary::setAci(const AccessControlInfoBinary & aci)
{
	mAci = aci;
}

const nx::AccessControlInfoDescBinary & nx::NpdmBinary::getAcid() const
{
	return mAcid;
}

void nx::NpdmBinary::setAcid(const AccessControlInfoDescBinary & acid)
{
	mAcid = acid;
}