#pragma once
#include <fnd/types.h>

namespace fnd
{
	class ISerialiseableBinary
	{
	public:
		virtual const byte_t* getBytes() const = 0;
		virtual size_t getSize() const = 0;

		virtual void exportBinary() = 0;
		virtual void importBinary(const byte_t* bytes, size_t len) = 0;

		virtual void clear() = 0;
	};
}