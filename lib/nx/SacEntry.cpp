#include "SacEntry.h"

SacEntry::SacEntry() :
	mIsServer(false),
	mName("")
{
}

SacEntry::SacEntry(const SacEntry & other)
{
	importBinary(other.getBytes(), other.getSize());
}

SacEntry::SacEntry(const u8 * bytes)
{
	importBinary(bytes);
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

	if (mName.length() > kMaxServiceNameLen)
	{
		throw fnd::Exception(kModuleName, "Service name string too long (max 8 chars)");
	}

	// copy data into binary blob
	mBinaryBlob[0] = (mIsServer ? SAC_IS_SERVER : 0) | (mName.length() & SAC_NAME_LEN_MASK);
	memcpy(mBinaryBlob.getBytes() + 1, mName.c_str(), mName.length());
}

void SacEntry::importBinary(const u8 * bytes)
{
	bool isServer = (bytes[0] & SAC_IS_SERVER) == SAC_IS_SERVER;
	size_t nameLen = (bytes[0] & SAC_NAME_LEN_MASK);
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

void SacEntry::importBinary(const u8 * bytes, size_t len)
{
	importBinary(bytes);
	if (getSize() != len)
	{
		throw fnd::Exception(kModuleName, "SAC Entry has unexpected size");
	}
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
