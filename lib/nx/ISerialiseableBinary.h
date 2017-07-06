#pragma once
#include <fnd/types.h>

namespace nx
{
	class ISerialiseableBinary
	{
	public:
		//virtual bool operator==(const ISerialiseableBinary& other) = 0;
		//virtual void operator=(const ISerialiseableBinary& other) = 0;

		virtual const u8* getBytes() const = 0;
		virtual size_t getSize() const = 0;

		virtual void exportBinary() = 0;
		virtual void importBinary(const u8* bytes) = 0;
		virtual void importBinary(const u8* bytes, size_t len) = 0;
	};
}

