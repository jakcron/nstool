#include <nn/hac/AccessControlInfoBinary.h>

nn::hac::AccessControlInfoBinary::AccessControlInfoBinary()
{
	clear();
}

nn::hac::AccessControlInfoBinary::AccessControlInfoBinary(const AccessControlInfoBinary & other)
{
	*this = other;
}

void nn::hac::AccessControlInfoBinary::operator=(const AccessControlInfoBinary & other)
{
	mRawBinary = other.mRawBinary;
	mProgramId = other.mProgramId;
	mFileSystemAccessControl = other.mFileSystemAccessControl;
	mServiceAccessControl = other.mServiceAccessControl;
	mKernelCapabilities = other.mKernelCapabilities;
}

bool nn::hac::AccessControlInfoBinary::operator==(const AccessControlInfoBinary & other) const
{
	return (mProgramId == other.mProgramId) \
		&& (mFileSystemAccessControl == other.mFileSystemAccessControl) \
		&& (mServiceAccessControl == other.mServiceAccessControl) \
		&& (mKernelCapabilities == other.mKernelCapabilities);
}

bool nn::hac::AccessControlInfoBinary::operator!=(const AccessControlInfoBinary & other) const
{
	return !(*this == other);
}

void nn::hac::AccessControlInfoBinary::toBytes()
{
	// serialise the sections
	mFileSystemAccessControl.toBytes();
	mServiceAccessControl.toBytes();
	mKernelCapabilities.toBytes();

	// determine section layout
	struct sLayout {
		uint32_t offset, size;
	} fac, sac, kc;

	fac.offset = (uint32_t)align(sizeof(sAciHeader), aci::kSectionAlignSize);
	fac.size = (uint32_t)mFileSystemAccessControl.getBytes().size();
	sac.offset = (uint32_t)align(fac.offset + fac.size, aci::kSectionAlignSize);
	sac.size = (uint32_t)mServiceAccessControl.getBytes().size();
	kc.offset = (uint32_t)align(sac.offset + sac.size, aci::kSectionAlignSize);
	kc.size = (uint32_t)mKernelCapabilities.getBytes().size();

	// get total size
	size_t total_size = _MAX(_MAX(fac.offset + fac.size, sac.offset + sac.size), kc.offset + kc.size); 

	mRawBinary.alloc(total_size);
	sAciHeader* hdr = (sAciHeader*)mRawBinary.data();

	// set type
	hdr->st_magic = aci::kAciStructMagic;

	// set program id
	hdr->program_id = mProgramId;

	// set offset/size
	hdr->fac.offset = fac.offset;
	hdr->fac.size = fac.size;
	hdr->sac.offset = sac.offset;
	hdr->sac.size = sac.size;
	hdr->kc.offset = kc.offset;
	hdr->kc.size = kc.size;

	// write data
	memcpy(mRawBinary.data() + fac.offset, mFileSystemAccessControl.getBytes().data(), fac.size);
	memcpy(mRawBinary.data() + sac.offset, mServiceAccessControl.getBytes().data(), sac.size);
	memcpy(mRawBinary.data() + kc.offset, mKernelCapabilities.getBytes().data(), kc.size);
}

void nn::hac::AccessControlInfoBinary::fromBytes(const byte_t* data, size_t len)
{
	// check size
	if (len < sizeof(sAciHeader))
	{
		throw fnd::Exception(kModuleName, "AccessControlInfo binary is too small");
	}
	
	// clear variables
	clear();

	// save a copy of the header
	sAciHeader hdr;
	memcpy((void*)&hdr, data, sizeof(sAciHeader));

	// check magic
	if (hdr.st_magic.get() != aci::kAciStructMagic)
	{
		throw fnd::Exception(kModuleName, "AccessControlInfo header corrupt");
	}
	
	// get total size
	size_t total_size = _MAX(_MAX(hdr.fac.offset.get() + hdr.fac.size.get(), hdr.sac.offset.get() + hdr.sac.size.get()), hdr.kc.offset.get() + hdr.kc.size.get()); 

	// validate binary size
	if (len < total_size)
	{
		throw fnd::Exception(kModuleName, "AccessControlInfo binary is too small");
	}

	// allocate memory for header
	mRawBinary.alloc(total_size);
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	// save variables
	mProgramId = hdr.program_id.get();
	mFileSystemAccessControl.fromBytes(mRawBinary.data() + hdr.fac.offset.get(), hdr.fac.size.get());
	mServiceAccessControl.fromBytes(mRawBinary.data() + hdr.sac.offset.get(), hdr.sac.size.get());
	mKernelCapabilities.fromBytes(mRawBinary.data() + hdr.kc.offset.get(), hdr.kc.size.get());
}

const fnd::Vec<byte_t>& nn::hac::AccessControlInfoBinary::getBytes() const
{
	return mRawBinary;
}

void nn::hac::AccessControlInfoBinary::clear()
{
	mRawBinary.clear();
	mProgramId = 0;
	mFileSystemAccessControl.clear();
	mServiceAccessControl.clear();
	mKernelCapabilities.clear();
}

uint64_t nn::hac::AccessControlInfoBinary::getProgramId() const
{
	return mProgramId;
}

void nn::hac::AccessControlInfoBinary::setProgramId(uint64_t program_id)
{
	mProgramId = program_id;
}

const nn::hac::FileSystemAccessControlBinary& nn::hac::AccessControlInfoBinary::getFileSystemAccessControl() const
{
	return mFileSystemAccessControl;
}

void nn::hac::AccessControlInfoBinary::setFileSystemAccessControl(const nn::hac::FileSystemAccessControlBinary& fac)
{
	mFileSystemAccessControl = fac;
}

const nn::hac::ServiceAccessControlBinary& nn::hac::AccessControlInfoBinary::getServiceAccessControl() const
{
	return mServiceAccessControl;
}

void nn::hac::AccessControlInfoBinary::setServiceAccessControl(const nn::hac::ServiceAccessControlBinary& sac)
{
	mServiceAccessControl = sac;
}

const nn::hac::KernelCapabilityBinary& nn::hac::AccessControlInfoBinary::getKernelCapabilities() const
{
	return mKernelCapabilities;
}

void nn::hac::AccessControlInfoBinary::setKernelCapabilities(const nn::hac::KernelCapabilityBinary& kc)
{
	mKernelCapabilities = kc;
}