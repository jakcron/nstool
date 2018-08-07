#include <nn/hac/AccessControlInfoDescBinary.h>

nn::hac::AccessControlInfoDescBinary::AccessControlInfoDescBinary()
{
	clear();
}

nn::hac::AccessControlInfoDescBinary::AccessControlInfoDescBinary(const AccessControlInfoDescBinary & other)
{
	*this = other;
}

void nn::hac::AccessControlInfoDescBinary::operator=(const AccessControlInfoDescBinary & other)
{
	mRawBinary = other.mRawBinary;
	mNcaHeaderSignature2Key = other.mNcaHeaderSignature2Key;
	mFlags = other.mFlags;
	mProgramIdRestrict = other.mProgramIdRestrict;
	mFileSystemAccessControl = other.mFileSystemAccessControl;
	mServiceAccessControl = other.mServiceAccessControl;
	mKernelCapabilities = other.mKernelCapabilities;
}

bool nn::hac::AccessControlInfoDescBinary::operator==(const AccessControlInfoDescBinary & other) const
{
	return (mNcaHeaderSignature2Key == other.mNcaHeaderSignature2Key) \
		&& (mFlags == other.mFlags) \
		&& (mProgramIdRestrict == other.mProgramIdRestrict) \
		&& (mFileSystemAccessControl == other.mFileSystemAccessControl) \
		&& (mServiceAccessControl == other.mServiceAccessControl) \
		&& (mKernelCapabilities == other.mKernelCapabilities);
}

bool nn::hac::AccessControlInfoDescBinary::operator!=(const AccessControlInfoDescBinary & other) const
{
	return !(*this == other);
}

void nn::hac::AccessControlInfoDescBinary::toBytes()
{
	if (mFileSystemAccessControl.getBytes().size() == 0)
		mFileSystemAccessControl.toBytes();

	if (mServiceAccessControl.getBytes().size() == 0)
		mServiceAccessControl.toBytes();

	if (mKernelCapabilities.getBytes().size() == 0)
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
	memcpy(hdr->nca_rsa_signature2_modulus, mNcaHeaderSignature2Key.modulus, fnd::rsa::kRsa2048Size);

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
}

void nn::hac::AccessControlInfoDescBinary::fromBytes(const byte_t* data, size_t len)
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
	memcpy(mNcaHeaderSignature2Key.modulus, hdr.nca_rsa_signature2_modulus, fnd::rsa::kRsa2048Size);

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

const fnd::Vec<byte_t>& nn::hac::AccessControlInfoDescBinary::getBytes() const
{
	return mRawBinary;
}

void nn::hac::AccessControlInfoDescBinary::generateSignature(const fnd::rsa::sRsa2048Key& key)
{
	if (mRawBinary.size() == 0)
		toBytes();

	byte_t hash[fnd::sha::kSha256HashLen];
	fnd::sha::Sha256(mRawBinary.data() + fnd::rsa::kRsa2048Size, mRawBinary.size() - fnd::rsa::kRsa2048Size, hash);

	if (fnd::rsa::pkcs::rsaSign(key, fnd::sha::HASH_SHA256, hash, mRawBinary.data()) != 0)
	{
		throw fnd::Exception(kModuleName, "Failed to sign Access Control Info Desc");
	}
}

void nn::hac::AccessControlInfoDescBinary::validateSignature(const fnd::rsa::sRsa2048Key& key) const
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

void nn::hac::AccessControlInfoDescBinary::clear()
{
	mRawBinary.clear();
	memset((void*)&mNcaHeaderSignature2Key, 0, sizeof(mNcaHeaderSignature2Key));
	mFlags.clear();
	mProgramIdRestrict.min = 0;
	mProgramIdRestrict.max = 0;
	mFileSystemAccessControl.clear();
	mServiceAccessControl.clear();
	mKernelCapabilities.clear();
}

const fnd::rsa::sRsa2048Key& nn::hac::AccessControlInfoDescBinary::getNcaHeaderSignature2Key() const
{
	return mNcaHeaderSignature2Key;
}

void nn::hac::AccessControlInfoDescBinary::setNcaHeaderSignature2Key(const fnd::rsa::sRsa2048Key& key)
{
	mNcaHeaderSignature2Key = key;
}

const fnd::List<nn::hac::aci::Flag>& nn::hac::AccessControlInfoDescBinary::getFlagList() const
{
	return mFlags;
}

void nn::hac::AccessControlInfoDescBinary::setFlagList(const fnd::List<nn::hac::aci::Flag>& flags)
{
	mFlags = flags;
}

const nn::hac::AccessControlInfoDescBinary::sProgramIdRestrict&  nn::hac::AccessControlInfoDescBinary::getProgramIdRestrict() const
{
	return mProgramIdRestrict;
}

void nn::hac::AccessControlInfoDescBinary::setProgramIdRestrict(const sProgramIdRestrict& pid_restrict)
{
	mProgramIdRestrict = pid_restrict;
}

const nn::hac::FileSystemAccessControlBinary& nn::hac::AccessControlInfoDescBinary::getFileSystemAccessControl() const
{
	return mFileSystemAccessControl;
}

void nn::hac::AccessControlInfoDescBinary::setFileSystemAccessControl(const nn::hac::FileSystemAccessControlBinary& fac)
{
	mFileSystemAccessControl = fac;
}

const nn::hac::ServiceAccessControlBinary& nn::hac::AccessControlInfoDescBinary::getServiceAccessControl() const
{
	return mServiceAccessControl;
}

void nn::hac::AccessControlInfoDescBinary::setServiceAccessControl(const nn::hac::ServiceAccessControlBinary& sac)
{
	mServiceAccessControl = sac;
}

const nn::hac::KernelCapabilityBinary& nn::hac::AccessControlInfoDescBinary::getKernelCapabilities() const
{
	return mKernelCapabilities;
}

void nn::hac::AccessControlInfoDescBinary::setKernelCapabilities(const nn::hac::KernelCapabilityBinary& kc)
{
	mKernelCapabilities = kc;
}