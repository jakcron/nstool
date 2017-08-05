#include <nx/SacEntry.h>

using namespace nx;

SacEntry::SacEntry() :
	mIsServer(false),
	mName("")
{
}

SacEntry::SacEntry(const std::string & name, bool isServer) :
	mIsServer(isServer),
	mName(name)
{
	exportBinary();
}

SacEntry::SacEntry(const SacEntry & other)
{
	copyFrom(other);
}

bool SacEntry::operator==(const SacEntry & other) const
{
	return isEqual(other);
}

bool SacEntry::operator!=(const SacEntry & other) const
{
	return !isEqual(other);
}

void SacEntry::operator=(const SacEntry & other)
{
	copyFrom(other);
}

const u8 * SacEntry::getBytes() const
{
	return mBinaryBlob.getBytes();
}

size_t SacEntry::getSize() const
{
	return mBinaryBlob.getSize();
}

void SacEntry::exportBinary()
{
	try {
		mBinaryBlob.alloc(mName.size() + 1);
	}
	catch (const fnd::Exception& e)
	{
		throw fnd::Exception(kModuleName, "Failed to allocate memory for SacEntry: " + std::string(e.what()));
	}

	if (mName.length() == 0)
	{
		throw fnd::Exception(kModuleName, "Service name is empty");
	}

	if (mName.length() > kMaxServiceNameLen)
	{
		throw fnd::Exception(kModuleName, "Service name string too long (max 8 chars)");
	}

	// copy data into binary blob
	mBinaryBlob[0] = (mIsServer ? SAC_IS_SERVER : 0) | ((mName.length()-1) & SAC_NAME_LEN_MASK); // bug?
	memcpy(mBinaryBlob.getBytes() + 1, mName.c_str(), mName.length());
}

void SacEntry::importBinary(const u8 * bytes, size_t len)
{
	bool isServer = (bytes[0] & SAC_IS_SERVER) == SAC_IS_SERVER;
	size_t nameLen = (bytes[0] & SAC_NAME_LEN_MASK) + 1; // bug?

	if (nameLen+1 > len)
	{
		throw fnd::Exception(kModuleName, "SAC entry is too small");
	}

	if (nameLen == 0)
	{
		throw fnd::Exception(kModuleName, "SAC entry has no service name");
	}
	else if (nameLen > kMaxServiceNameLen)
	{
		throw fnd::Exception(kModuleName, "Service name string too long (max 8 chars)");
	}

	mBinaryBlob.alloc(nameLen + 1);
	memcpy(mBinaryBlob.getBytes(), bytes, mBinaryBlob.getSize());

	mIsServer = isServer;
	mName = std::string((const char*)(mBinaryBlob.getBytes() + 1), nameLen);
}

void nx::SacEntry::clear()
{
	mIsServer = false;
	mName.clear();
}

bool SacEntry::isServer() const
{
	return mIsServer;
}

void SacEntry::setIsServer(bool isServer)
{
	mIsServer = isServer;
}

const std::string & SacEntry::getName() const
{
	return mName;
}

void SacEntry::setName(const std::string & name)
{
	if (name.length() > kMaxServiceNameLen)
	{
		throw fnd::Exception(kModuleName, "Service name string too long (max 8 chars)");
	}

	mName = name;
}

bool SacEntry::isEqual(const SacEntry & other) const
{
	return (mIsServer == other.mIsServer) \
		&& (mName == other.mName);
}

void SacEntry::copyFrom(const SacEntry & other)
{
	if (other.getSize())
	{
		importBinary(other.getBytes(), other.getSize());
	}
	else
	{
		this->mIsServer = other.mIsServer;
		this->mName = other.mName;
	}
}
