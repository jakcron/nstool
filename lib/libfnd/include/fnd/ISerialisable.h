#pragma once
#include <fnd/types.h>
#include <fnd/Vec.h>

namespace fnd
{
	class ISerialisable
	{
	public:
		virtual ~ISerialisable() = default;

		// serialise
		virtual void toBytes() = 0;
		// deserialise
		virtual void fromBytes(const byte_t* data, size_t len) = 0;

		// get byte vector
		virtual const fnd::Vec<byte_t>& getBytes() const = 0;

		// clear data
		virtual void clear() = 0;
	};
}