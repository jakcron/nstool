#include <nx/SacEntry.h>

nx::SacEntry::SacEntry()
{
	clear();
}

nx::SacEntry::SacEntry(const std::string & name, bool isServer) :
	mIsServer(isServer),
	mName(name)
{
	toBytes();
}

nx::SacEntry::SacEntry(const SacEntry & other)
{
	*this = other;
}

void nx::SacEntry::operator=(const SacEntry & other)
{
	if (other.getBytes().size())
	{
		fromBytes(other.getBytes().data(), other.getBytes().size());
	}
	else
	{
		clear();
		this->mIsServer = other.mIsServer;
		this->mName = other.mName;
	}
}

bool nx::SacEntry::operator==(const SacEntry & other) const
{
	return (mIsServer == other.mIsServer) \
		&& (mName == other.mName);
}

bool nx::SacEntry::operator!=(const SacEntry & other) const
{
	return !(*this == other);
}


void nx::SacEntry::toBytes()
{
	try {
		mRawBinary.alloc(mName.size() + 1);
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
	mRawBinary[0] = (mIsServer ? SAC_IS_SERVER : 0) | ((mName.length()-1) & SAC_NAME_LEN_MASK); // bug?
	memcpy(mRawBinary.data() + 1, mName.c_str(), mName.length());
}

void nx::SacEntry::fromBytes(const byte_t* data, size_t len)
{
	bool isServer = (data[0] & SAC_IS_SERVER) == SAC_IS_SERVER;
	size_t nameLen = (data[0] & SAC_NAME_LEN_MASK) + 1; // bug?

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

	mRawBinary.alloc(nameLen + 1);
	memcpy(mRawBinary.data(), data, mRawBinary.size());

	mIsServer = isServer;
	mName = std::string((const char*)(mRawBinary.data() + 1), nameLen);
}

const fnd::Vec<byte_t>& nx::SacEntry::getBytes() const
{
	return mRawBinary;
}

void nx::SacEntry::clear()
{
	mIsServer = false;
	mName.clear();
}

bool nx::SacEntry::isServer() const
{
	return mIsServer;
}

void nx::SacEntry::setIsServer(bool isServer)
{
	mIsServer = isServer;
}

const std::string & nx::SacEntry::getName() const
{
	return mName;
}

void nx::SacEntry::setName(const std::string & name)
{
	if (name.length() > kMaxServiceNameLen)
	{
		throw fnd::Exception(kModuleName, "Service name string too long (max 8 chars)");
	}

	mName = name;
}