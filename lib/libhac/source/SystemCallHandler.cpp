#include <nn/hac/SystemCallHandler.h>
#include <nn/hac/SystemCallEntry.h>

nn::hac::SystemCallHandler::SystemCallHandler() :
	mIsSet(false),
	mSystemCalls()
{}

void nn::hac::SystemCallHandler::operator=(const SystemCallHandler & other)
{
	mIsSet = other.mIsSet;
	mSystemCalls = other.mSystemCalls;
}

bool nn::hac::SystemCallHandler::operator==(const SystemCallHandler & other) const
{
	return (mIsSet == other.mIsSet) \
		&& (mSystemCalls == other.mSystemCalls);
}

bool nn::hac::SystemCallHandler::operator!=(const SystemCallHandler & other) const
{
	return !(*this == other);
}

void nn::hac::SystemCallHandler::importKernelCapabilityList(const fnd::List<KernelCapabilityEntry>& caps)
{
	if (caps.size() == 0)
		return;

	SystemCallEntry entry;

	uint8_t syscallUpper, syscall;
	for (size_t i = 0; i < caps.size(); i++)
	{
		entry.setKernelCapability(caps[i]);
		syscallUpper = 24 * entry.getSystemCallUpperBits();
		for (uint8_t j = 0; j < 24; j++)
		{
			syscall = syscallUpper + j;
			if (((entry.getSystemCallLowerBits() >> j) & 1) == 1)
			{
				mSystemCalls.hasElement(syscall) == false ? mSystemCalls.addElement(syscall) : throw fnd::Exception(kModuleName, "SystemCall already added");
			}
		}
	}


	mIsSet = true;
}

void nn::hac::SystemCallHandler::exportKernelCapabilityList(fnd::List<KernelCapabilityEntry>& caps) const
{
	if (isSet() == false)
		return;

	SystemCallEntry entries[kSyscallTotalEntryNum];
	for (size_t i = 0; i < kSyscallTotalEntryNum; i++)
	{
		entries[i].setSystemCallUpperBits((uint32_t)i);
		entries[i].setSystemCallLowerBits(0);
	}

	for (size_t i = 0; i < mSystemCalls.size(); i++)
	{
		if (mSystemCalls[i] > kMaxSystemCall)
		{
			throw fnd::Exception(kModuleName, "Illegal SystemCall. (range: 0x00-0xBF inclusive)");
		}

		entries[mSystemCalls[i] / 24].setSystemCallLowerBits(entries[mSystemCalls[i] / 24].getSystemCallLowerBits() | BIT(mSystemCalls[i] % 24));
	}

	for (size_t i = 0; i < kSyscallTotalEntryNum; i++)
	{
		if (entries[i].getSystemCallLowerBits() != 0)
		{
			caps.addElement(entries[i].getKernelCapability());
		}
	}
}

void nn::hac::SystemCallHandler::clear()
{
	mIsSet = false;
	mSystemCalls.clear();
}

bool nn::hac::SystemCallHandler::isSet() const
{
	return mIsSet;
}

const fnd::List<uint8_t>& nn::hac::SystemCallHandler::getSystemCalls() const
{
	return mSystemCalls;
}

void nn::hac::SystemCallHandler::setSystemCallList(const fnd::List<uint8_t>& calls)
{
	mSystemCalls.clear();
	for (size_t i = 0; i < calls.size(); i++)
	{
		if (mSystemCalls[i] > kMaxSystemCall)
		{
			throw fnd::Exception(kModuleName, "Illegal SystemCall. (range: 0x00-0xBF inclusive)");
		}

		mSystemCalls.hasElement(calls[i]) == false ? mSystemCalls.addElement(calls[i]) : throw fnd::Exception(kModuleName, "SystemCall already added");
	}

	mIsSet = true;
}