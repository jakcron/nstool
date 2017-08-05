#pragma once
#include <fnd/types.h>

namespace fnd
{
	class ISerialiseableBinary
	{
	public:
		virtual const u8* getBytes() const = 0;
		virtual size_t getSize() const = 0;

		virtual void exportBinary() = 0;
		virtual void importBinary(const u8* bytes, size_t len) = 0;

		virtual void clear() = 0;
	};
}