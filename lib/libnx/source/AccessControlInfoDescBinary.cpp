#include <nx/AccessControlInfoDescBinary.h>

nx::AccessControlInfoDescBinary::AccessControlInfoDescBinary()
{
	clear();
}

nx::AccessControlInfoDescBinary::AccessControlInfoDescBinary(const AccessControlInfoDescBinary & other)
{
	*this = other;
}

void nx::AccessControlInfoDescBinary::operator=(const AccessControlInfoDescBinary & other)
{
	mRawBinary = other.mRawBinary;
	mNcaHeaderSignature2Key = other.mNcaHeaderSignature2Key;
	mFlags = other.mFlags;
	mProgramIdRestrict = other.mProgramIdRestrict;
	mFileSystemAccessControl = other.mFileSystemAccessControl;
	mServiceAccessControl = other.mServiceAccessControl;
	mKernelCapabilities = other.mKernelCapabilities;
}

bool nx::AccessControlInfoDescBinary::operator==(const AccessControlInfoDescBinary & other) const
{
	return (mNcaHeaderSignature2Key == other.mNcaHeaderSignature2Key) \
		&& (mFlags == other.mFlags) \
		&& (mProgramIdRestrict == other.mProgramIdRestrict) \
		&& (mFileSystemAccessControl == other.mFileSystemAccessControl) \
		&& (mServiceAccessControl == other.mServiceAccessControl) \
		&& (mKernelCapabilities == other.mKernelCapabilities);
}

bool nx::AccessControlInfoDescBinary::operator!=(const AccessControlInfoDescBinary & other) const
{
	return !(*this == other);
}

void nx::AccessControlInfoDescBinary::toBytes()
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

	fac.offset = align(sizeof(sAciDescHeader), aci::kSectionAlignSize);
	fac.size = (uint32_t)mFileSystemAccessControl.getBytes().size();
	sac.offset = align(fac.offset + fac.size, aci::kSectionAlignSize);
	sac.size = (uint32_t)mServiceAccessControl.getBytes().size();
	kc.offset = align(sac.offset + sac.size, aci::kSectionAlignSize);
	kc.size = (uint32_t)mKernelCapabilities.getBytes().size();

	// get total size
	size_t total_size = _MAX(_MAX(fac.offset + fac.size, sac.offset + sac.size), kc.offset + kc.size); 

	mRawBinary.alloc(total_size);
	sAciDescHeader* hdr = (sAciDescHeader*)mRawBinary.data();

	// set rsa modulus
	memcpy(hdr->nca_rsa_signature2_modulus, mNcaHeaderSignature2Key.modulus, crypto::rsa::kRsa2048Size);

	// set type
	hdr->st_magic = aci::kAciDescStructMagic;

	// set "acid size"
	hdr->signed_size = total_size - crypto::rsa::kRsa2048Size;

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

void nx::AccessControlInfoDescBinary::fromBytes(const byte_t* data, size_t len)
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
	memcpy(mNcaHeaderSignature2Key.modulus, hdr.nca_rsa_signature2_modulus, crypto::rsa::kRsa2048Size);

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

const fnd::Vec<byte_t>& nx::AccessControlInfoDescBinary::getBytes() const
{
	return mRawBinary;
}

void nx::AccessControlInfoDescBinary::generateSignature(const crypto::rsa::sRsa2048Key& key)
{
	if (mRawBinary.size() == 0)
		toBytes();

	byte_t hash[crypto::sha::kSha256HashLen];
	crypto::sha::Sha256(mRawBinary.data() + crypto::rsa::kRsa2048Size, mRawBinary.size() - crypto::rsa::kRsa2048Size, hash);

	if (crypto::rsa::pkcs::rsaSign(key, crypto::sha::HASH_SHA256, hash, mRawBinary.data()) != 0)
	{
		throw fnd::Exception(kModuleName, "Failed to sign Access Control Info Desc");
	}
}

void nx::AccessControlInfoDescBinary::validateSignature(const crypto::rsa::sRsa2048Key& key) const
{
	if (mRawBinary.size() == 0)
		throw fnd::Exception(kModuleName, "No Access Control Info Desc binary exists to verify");

	byte_t hash[crypto::sha::kSha256HashLen];
	crypto::sha::Sha256(mRawBinary.data() + crypto::rsa::kRsa2048Size, mRawBinary.size() - crypto::rsa::kRsa2048Size, hash);

	if (crypto::rsa::pss::rsaVerify(key, crypto::sha::HASH_SHA256, hash, mRawBinary.data()) != 0)
	{
		throw fnd::Exception(kModuleName, "Failed to verify Access Control Info Desc");
	}
}

void nx::AccessControlInfoDescBinary::clear()
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

const crypto::rsa::sRsa2048Key& nx::AccessControlInfoDescBinary::getNcaHeaderSignature2Key() const
{
	return mNcaHeaderSignature2Key;
}

void nx::AccessControlInfoDescBinary::setNcaHeaderSignature2Key(const crypto::rsa::sRsa2048Key& key)
{
	mNcaHeaderSignature2Key = key;
}

const fnd::List<nx::aci::Flag>& nx::AccessControlInfoDescBinary::getFlagList() const
{
	return mFlags;
}

void nx::AccessControlInfoDescBinary::setFlagList(const fnd::List<nx::aci::Flag>& flags)
{
	mFlags = flags;
}

const nx::AccessControlInfoDescBinary::sProgramIdRestrict&  nx::AccessControlInfoDescBinary::getProgramIdRestrict() const
{
	return mProgramIdRestrict;
}

void nx::AccessControlInfoDescBinary::setProgramIdRestrict(const sProgramIdRestrict& pid_restrict)
{
	mProgramIdRestrict = pid_restrict;
}

const nx::FileSystemAccessControlBinary& nx::AccessControlInfoDescBinary::getFileSystemAccessControl() const
{
	return mFileSystemAccessControl;
}

void nx::AccessControlInfoDescBinary::setFileSystemAccessControl(const nx::FileSystemAccessControlBinary& fac)
{
	mFileSystemAccessControl = fac;
}

const nx::SacBinary& nx::AccessControlInfoDescBinary::getServiceAccessControl() const
{
	return mServiceAccessControl;
}

void nx::AccessControlInfoDescBinary::setServiceAccessControl(const nx::SacBinary& sac)
{
	mServiceAccessControl = sac;
}

const nx::KcBinary& nx::AccessControlInfoDescBinary::getKernelCapabilities() const
{
	return mKernelCapabilities;
}

void nx::AccessControlInfoDescBinary::setKernelCapabilities(const nx::KcBinary& kc)
{
	mKernelCapabilities = kc;
}