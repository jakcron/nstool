#pragma once
#include <fnd/types.h>

namespace fnd
{
	class IFile
	{
	public:
		virtual ~IFile() = default;

		virtual size_t size() = 0;
		virtual void seek(size_t offset) = 0;
		virtual void read(byte_t* out, size_t len) = 0;
		virtual void read(byte_t* out, size_t offset, size_t len) = 0;
		virtual void write(const byte_t* out, size_t len) = 0;
		virtual void write(const byte_t* out, size_t offset, size_t len) = 0;
	};
}
