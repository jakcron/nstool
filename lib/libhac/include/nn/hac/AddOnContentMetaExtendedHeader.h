#pragma once
#include <string>
#include <cstring>
#include <fnd/IByteModel.h>
#include <nn/hac/define/cnmt.h>

namespace nn
{
namespace hac
{
	class AddOnContentMetaExtendedHeader :
		public fnd::IByteModel
	{
	public:
		AddOnContentMetaExtendedHeader();
		AddOnContentMetaExtendedHeader(const AddOnContentMetaExtendedHeader& other);

		void operator=(const AddOnContentMetaExtendedHeader& other);
		bool operator==(const AddOnContentMetaExtendedHeader& other) const;
		bool operator!=(const AddOnContentMetaExtendedHeader& other) const;

		// IByteModel
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		uint64_t getApplicationId() const;
		void setApplicationId(uint64_t application_id);

		uint32_t getRequiredApplicationVersion() const;
		void setRequiredApplicationVersion(uint32_t app_ver);
	private:
		const std::string kModuleName = "ADD_ON_CONTENT_META_EXTENDED_HEADER";

		// binary blob
		fnd::Vec<byte_t> mRawBinary;

		// variables
		uint64_t mApplicationId;
		uint32_t mRequiredApplicationVersion;
	};
}
}