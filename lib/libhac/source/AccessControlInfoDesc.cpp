#include <nn/hac/AccessControlInfoDesc.h>

nn::hac::AccessControlInfoDesc::AccessControlInfoDesc()
{
	clear();
}

nn::hac::AccessControlInfoDesc::AccessControlInfoDesc(const AccessControlInfoDesc & other)
{
	*this = other;
}

void nn::hac::AccessControlInfoDesc::operator=(const AccessControlInfoDesc & other)
{
	mRawBinary = other.mRawBinary;
	mContentArchiveHeaderSignature2Key = other.mContentArchiveHeaderSignature2Key;
	mFlags = other.mFlags;
	mProgramIdRestrict = other.mProgramIdRestrict;
	mFileSystemAccessControl = other.mFileSystemAccessControl;
	mServiceAccessControl = other.mServiceAccessControl;
	mKernelCapabilities = other.mKernelCapabilities;
}

bool nn::hac::AccessControlInfoDesc::operator==(const AccessControlInfoDesc & other) const
{
	return (mContentArchiveHeaderSignature2Key == other.mContentArchiveHeaderSignature2Key) \
		&& (mFlags == other.mFlags) \
		&& (mProgramIdRestrict == other.mProgramIdRestrict) \
		&& (mFileSystemAccessControl == other.mFileSystemAccessControl) \
		&& (mServiceAccessControl == other.mServiceAccessControl) \
		&& (mKernelCapabilities == other.mKernelCapabilities);
}

bool nn::hac::AccessControlInfoDesc::operator!=(const AccessControlInfoDesc & other) const
{
	return !(*this == other);
}

void nn::hac::AccessControlInfoDesc::toBytes()
{
	// serialise the sections
	mFileSystemAccessControl.toBytes();
	mServiceAccessControl.toBytes();
	mKernelCapabilities.toBytes();

	// determine section layout
	struct sLayout {
		uint32_t offset, size;
	} fac, sac, kc;

	fac.offset = (uint32_t)align(sizeof(sAciDescHeader), aci::kSectionAlignSize);
	fac.size = (uint32_t)mFileSystemAccessControl.getBytes().size();
	sac.offset = (uint32_t)align(fac.offset + fac.size, aci::kSectionAlignSize);
	sac.size = (uint32_t)mServiceAccessControl.getBytes().size();
	kc.offset = (uint32_t)align(sac.offset + sac.size, aci::kSectionAlignSize);
	kc.size = (uint32_t)mKernelCapabilities.getBytes().size();

	// get total size
	size_t total_size = _MAX(_MAX(fac.offset + fac.size, sac.offset + sac.size), kc.offset + kc.size); 

	mRawBinary.alloc(total_size);
	sAciDescHeader* hdr = (sAciDescHeader*)mRawBinary.data();

	// set rsa modulus
	memcpy(hdr->nca_rsa_signature2_modulus, mContentArchiveHeaderSignature2Key.modulus, fnd::rsa::kRsa2048Size);

	// set type
	hdr->st_magic = aci::kAciDescStructMagic;

	// set "acid size"
	hdr->signed_size = (uint32_t)(total_size - fnd::rsa::kRsa2048Size);

	// set flags
	uint32_t flags = 0;
	for (size_t i = 0; i < mFlags.size(); i++)
		flags |= _BIT(mFlags[i]);
	hdr->flags = flags;

	// set program id restrict settings
	hdr->program_id_min = mProgramIdRestrict.min;
	hdr->program_id_max = mProgramIdRestrict.max;

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

void nn::hac::AccessControlInfoDesc::fromBytes(const byte_t* data, size_t len)
{
	// check size
	if (len < sizeof(sAciDescHeader))
	{
		throw fnd::Exception(kModuleName, "AccessControlInfoDesc binary is too small");
	}
	
	// clear variables
	clear();

	// save a copy of the header
	sAciDescHeader hdr;
	memcpy((void*)&hdr, data, sizeof(sAciDescHeader));

	// check magic
	if (hdr.st_magic.get() != aci::kAciDescStructMagic)
	{
		throw fnd::Exception(kModuleName, "AccessControlInfoDesc header corrupt");
	}
	
	// get total size
	size_t total_size = _MAX(_MAX(hdr.fac.offset.get() + hdr.fac.size.get(), hdr.sac.offset.get() + hdr.sac.size.get()), hdr.kc.offset.get() + hdr.kc.size.get()); 

	// validate binary size
	if (len < total_size)
	{
		throw fnd::Exception(kModuleName, "AccessControlInfoDesc binary is too small");
	}

	// allocate memory for header
	mRawBinary.alloc(total_size);
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	// save variables
	memcpy(mContentArchiveHeaderSignature2Key.modulus, hdr.nca_rsa_signature2_modulus, fnd::rsa::kRsa2048Size);

	for (size_t i = 0; i < 32; i++)
	{
		if (_HAS_BIT(hdr.flags.get(), i))
			mFlags.addElement((aci::Flag)i);
	}

	mProgramIdRestrict.min = hdr.program_id_min.get();
	mProgramIdRestrict.max = hdr.program_id_max.get();

	mFileSystemAccessControl.fromBytes(mRawBinary.data() + hdr.fac.offset.get(), hdr.fac.size.get());
	mServiceAccessControl.fromBytes(mRawBinary.data() + hdr.sac.offset.get(), hdr.sac.size.get());
	mKernelCapabilities.fromBytes(mRawBinary.data() + hdr.kc.offset.get(), hdr.kc.size.get());
}

const fnd::Vec<byte_t>& nn::hac::AccessControlInfoDesc::getBytes() const
{
	return mRawBinary;
}

void nn::hac::AccessControlInfoDesc::generateSignature(const fnd::rsa::sRsa2048Key& key)
{
	if (mRawBinary.size() == 0)
		toBytes();

	byte_t hash[fnd::sha::kSha256HashLen];
	fnd::sha::Sha256(mRawBinary.data() + fnd::rsa::kRsa2048Size, mRawBinary.size() - fnd::rsa::kRsa2048Size, hash);

	if (fnd::rsa::pss::rsaSign(key, fnd::sha::HASH_SHA256, hash, mRawBinary.data()) != 0)
	{
		throw fnd::Exception(kModuleName, "Failed to sign Access Control Info Desc");
	}
}

void nn::hac::AccessControlInfoDesc::validateSignature(const fnd::rsa::sRsa2048Key& key) const
{
	if (mRawBinary.size() == 0)
		throw fnd::Exception(kModuleName, "No Access Control Info Desc binary exists to verify");

	byte_t hash[fnd::sha::kSha256HashLen];
	fnd::sha::Sha256(mRawBinary.data() + fnd::rsa::kRsa2048Size, mRawBinary.size() - fnd::rsa::kRsa2048Size, hash);

	if (fnd::rsa::pss::rsaVerify(key, fnd::sha::HASH_SHA256, hash, mRawBinary.data()) != 0)
	{
		throw fnd::Exception(kModuleName, "Failed to verify Access Control Info Desc");
	}
}

void nn::hac::AccessControlInfoDesc::clear()
{
	mRawBinary.clear();
	memset((void*)&mContentArchiveHeaderSignature2Key, 0, sizeof(mContentArchiveHeaderSignature2Key));
	mFlags.clear();
	mProgramIdRestrict.min = 0;
	mProgramIdRestrict.max = 0;
	mFileSystemAccessControl.clear();
	mServiceAccessControl.clear();
	mKernelCapabilities.clear();
}

const fnd::rsa::sRsa2048Key& nn::hac::AccessControlInfoDesc::getContentArchiveHeaderSignature2Key() const
{
	return mContentArchiveHeaderSignature2Key;
}

void nn::hac::AccessControlInfoDesc::setContentArchiveHeaderSignature2Key(const fnd::rsa::sRsa2048Key& key)
{
	mContentArchiveHeaderSignature2Key = key;
}

const fnd::List<nn::hac::aci::Flag>& nn::hac::AccessControlInfoDesc::getFlagList() const
{
	return mFlags;
}

void nn::hac::AccessControlInfoDesc::setFlagList(const fnd::List<nn::hac::aci::Flag>& flags)
{
	mFlags = flags;
}

const nn::hac::AccessControlInfoDesc::sProgramIdRestrict&  nn::hac::AccessControlInfoDesc::getProgramIdRestrict() const
{
	return mProgramIdRestrict;
}

void nn::hac::AccessControlInfoDesc::setProgramIdRestrict(const sProgramIdRestrict& pid_restrict)
{
	mProgramIdRestrict = pid_restrict;
}

const nn::hac::FileSystemAccessControl& nn::hac::AccessControlInfoDesc::getFileSystemAccessControl() const
{
	return mFileSystemAccessControl;
}

void nn::hac::AccessControlInfoDesc::setFileSystemAccessControl(const nn::hac::FileSystemAccessControl& fac)
{
	mFileSystemAccessControl = fac;
}

const nn::hac::ServiceAccessControl& nn::hac::AccessControlInfoDesc::getServiceAccessControl() const
{
	return mServiceAccessControl;
}

void nn::hac::AccessControlInfoDesc::setServiceAccessControl(const nn::hac::ServiceAccessControl& sac)
{
	mServiceAccessControl = sac;
}

const nn::hac::KernelCapabilityControl& nn::hac::AccessControlInfoDesc::getKernelCapabilities() const
{
	return mKernelCapabilities;
}

void nn::hac::AccessControlInfoDesc::setKernelCapabilities(const nn::hac::KernelCapabilityControl& kc)
{
	mKernelCapabilities = kc;
}