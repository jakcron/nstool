#pragma once
#include <string>
#include <fnd/types.h>
#include <fnd/memory_blob.h>
#include <nx/ISerialiseableBinary.h>

namespace nx
{
	class SacEntry : public ISerialiseableBinary
	{
	public:
		SacEntry();
		SacEntry(const std::string& name, bool isServer);
		SacEntry(const SacEntry& other);

		bool operator==(const SacEntry& other) const;
		bool operator!=(const SacEntry& other) const;
		void operator=(const SacEntry& other);

		// to be used after export
		const u8* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const u8* bytes);
		void importBinary(const u8* bytes, size_t len);

		// variables
		bool isServer() const;
		void setIsServer(bool isServer);
		const std::string& getName() const;
		void setName(const std::string& name);
	private:
		const std::string kModuleName = "SAC_ENTRY";
		static const size_t kMaxServiceNameLen = 8;

		enum SacEntryFlag
		{
			SAC_IS_SERVER = BIT(7),
			SAC_NAME_LEN_MASK = BIT(7) - 1
		};

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		bool mIsServer;
		std::string mName;

		bool isEqual(const SacEntry& other) const;
		void copyFrom(const SacEntry& other);
	};
}