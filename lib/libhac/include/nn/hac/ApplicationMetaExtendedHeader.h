#pragma once
#include <string>
#include <cstring>
#include <fnd/IByteModel.h>
#include <nn/hac/define/cnmt.h>

namespace nn
{
namespace hac
{
	class ApplicationMetaExtendedHeader :
		public fnd::IByteModel
	{
	public:
		ApplicationMetaExtendedHeader();
		ApplicationMetaExtendedHeader(const ApplicationMetaExtendedHeader& other);

		void operator=(const ApplicationMetaExtendedHeader& other);
		bool operator==(const ApplicationMetaExtendedHeader& other) const;
		bool operator!=(const ApplicationMetaExtendedHeader& other) const;

		// IByteModel
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		uint64_t getPatchId() const;
		void setPatchId(uint64_t patch_id);

		uint32_t getRequiredSystemVersion() const;
		void setRequiredSystemVersion(uint32_t sys_ver);
	private:
		const std::string kModuleName = "APPLICATION_META_EXTENDED_HEADER";

		// binary blob
		fnd::Vec<byte_t> mRawBinary;

		// variables
		uint64_t mPatchId;
		uint32_t mRequiredSystemVersion;
	};
}
}