#include <nn/hac/Meta.h>

#include <fnd/SimpleTextOutput.h>

nn::hac::Meta::Meta()
{
	clear();
}

nn::hac::Meta::Meta(const Meta & other) :
	Meta()
{
	*this = other;
}

void nn::hac::Meta::operator=(const Meta & other)
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

bool nn::hac::Meta::operator==(const Meta & other) const
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

bool nn::hac::Meta::operator!=(const Meta & other) const
{
	return !(*this == other);
}

void nn::hac::Meta::toBytes()
{
	if (mAcid.getBytes().size() == 0)
		mAcid.toBytes();

	if (mAci.getBytes().size() == 0)
		mAci.toBytes();


	// determine section layout
	struct sLayout {
		uint32_t offset, size;
	} acid, aci;

	acid.offset = (uint32_t)align(sizeof(sMetaHeader), meta::kSectionAlignSize);
	acid.size = (uint32_t)mAcid.getBytes().size();
	aci.offset = (uint32_t)(acid.offset + align(acid.size, meta::kSectionAlignSize));
	aci.size = (uint32_t)mAci.getBytes().size();
	

	// get total size
	size_t total_size = _MAX(_MAX(acid.offset + acid.size, aci.offset + aci.size), align(sizeof(sMetaHeader), meta::kSectionAlignSize)); 

	mRawBinary.alloc(total_size);
	sMetaHeader* hdr = (sMetaHeader*)mRawBinary.data();

	// set type
	hdr->st_magic = meta::kMetaStructMagic;

	// set variables
	byte_t flag = ((byte_t)(mInstructionType & 1) | (byte_t)((mProcAddressSpaceType & 3) << 1)) & 0xf;
	hdr->flags = flag;
	hdr->main_thread_priority = mMainThreadPriority;
	hdr->main_thread_cpu_id = mMainThreadCpuId;
	hdr->version = mVersion;
	hdr->main_thread_stack_size = mMainThreadStackSize;
	strncpy(hdr->name, mName.c_str(), meta::kNameMaxLen);
	strncpy(hdr->product_code, mProductCode.c_str(), meta::kProductCodeMaxLen);

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

void nn::hac::Meta::fromBytes(const byte_t* data, size_t len)
{
	// check size
	if (len < sizeof(sMetaHeader))
	{
		throw fnd::Exception(kModuleName, "META binary is too small");
	}
	
	// clear variables
	clear();

	// save a copy of the header
	sMetaHeader hdr;
	memcpy((void*)&hdr, data, sizeof(sMetaHeader));

	// check magic
	if (hdr.st_magic.get() != meta::kMetaStructMagic)
	{
		throw fnd::Exception(kModuleName, "META header corrupt (unrecognised struct signature)");
	}

	// save variables
	byte_t flag = hdr.flags & 0xf;
	mInstructionType = (meta::InstructionType)(flag & 1);
	mProcAddressSpaceType = (meta::ProcAddrSpaceType)((flag >> 1) & 3);
	mMainThreadPriority = hdr.main_thread_priority;
	mMainThreadCpuId = hdr.main_thread_cpu_id;
	mVersion = hdr.version.get();
	mMainThreadStackSize = hdr.main_thread_stack_size.get();
	mName = std::string(hdr.name, _MIN(strlen(hdr.name), meta::kNameMaxLen));
	mProductCode = std::string(hdr.product_code, _MIN(strlen(hdr.product_code), meta::kProductCodeMaxLen));

	// total size
	size_t total_size = _MAX(_MAX(hdr.acid.offset.get() + hdr.acid.size.get(), hdr.aci.offset.get() + hdr.aci.size.get()), sizeof(sMetaHeader));

	// check size
	if (total_size > len)
	{
		throw fnd::Exception(kModuleName, "META binary too small");
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

const fnd::Vec<byte_t>& nn::hac::Meta::getBytes() const
{
	return mRawBinary;
}

void nn::hac::Meta::clear()
{
	mRawBinary.clear();
	mInstructionType = meta::INSTR_64BIT;
	mProcAddressSpaceType = meta::ADDR_SPACE_64BIT;
	mMainThreadPriority = 0;
	mMainThreadCpuId = 0;
	mVersion = 0;
	mMainThreadStackSize = 0;
	mName.clear();
	mProductCode.clear();
	mAci.clear();
	mAcid.clear();
}

nn::hac::meta::InstructionType nn::hac::Meta::getInstructionType() const
{
	return mInstructionType;
}

void nn::hac::Meta::setInstructionType(meta::InstructionType type)
{
	mInstructionType = type;
}

nn::hac::meta::ProcAddrSpaceType nn::hac::Meta::getProcAddressSpaceType() const
{
	return mProcAddressSpaceType;
}

void nn::hac::Meta::setProcAddressSpaceType(meta::ProcAddrSpaceType type)
{
	mProcAddressSpaceType = type;
}

byte_t nn::hac::Meta::getMainThreadPriority() const
{
	return mMainThreadPriority;
}

void nn::hac::Meta::setMainThreadPriority(byte_t priority)
{
	if (priority > meta::kMaxPriority)
	{
		throw fnd::Exception(kModuleName, "Illegal main thread priority (range 0-63)");
	}

	mMainThreadPriority = priority;
}

byte_t nn::hac::Meta::getMainThreadCpuId() const
{
	return mMainThreadCpuId;
}

void nn::hac::Meta::setMainThreadCpuId(byte_t core_num)
{
	mMainThreadCpuId = core_num;
}

uint32_t nn::hac::Meta::getVersion() const
{
	return mVersion;
}

void nn::hac::Meta::setVersion(uint32_t version)
{
	mVersion = version;
}

uint32_t nn::hac::Meta::getMainThreadStackSize() const
{
	return mMainThreadStackSize;
}

void nn::hac::Meta::setMainThreadStackSize(uint32_t size)
{
	mMainThreadStackSize = size;
}

const std::string & nn::hac::Meta::getName() const
{
	return mName;
}

void nn::hac::Meta::setName(const std::string & name)
{
	if (name.length() > meta::kNameMaxLen)
	{
		throw fnd::Exception(kModuleName, "Name is too long");
	}

	mName = name;
}

const std::string & nn::hac::Meta::getProductCode() const
{
	return mProductCode;
}

void nn::hac::Meta::setProductCode(const std::string & product_code)
{
	if (product_code.length() > meta::kProductCodeMaxLen)
	{
		throw fnd::Exception(kModuleName, "Product Code is too long");
	}

	mProductCode = product_code;
}

const nn::hac::AccessControlInfo & nn::hac::Meta::getAci() const
{
	return mAci;
}

void nn::hac::Meta::setAci(const AccessControlInfo & aci)
{
	mAci = aci;
}

const nn::hac::AccessControlInfoDesc & nn::hac::Meta::getAcid() const
{
	return mAcid;
}

void nn::hac::Meta::setAcid(const AccessControlInfoDesc & acid)
{
	mAcid = acid;
}