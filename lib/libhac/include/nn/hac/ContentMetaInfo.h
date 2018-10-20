#pragma once
#include <string>
#include <cstring>
#include <fnd/IByteModel.h>
#include <fnd/List.h>
#include <nn/hac/cnmt.h>

namespace nn
{
namespace hac
{
	class ContentMetaInfo : 
		public fnd::IByteModel
	{
	public:
		ContentMetaInfo();
		ContentMetaInfo(const ContentMetaInfo& other);

		void operator=(const ContentMetaInfo& other);
		bool operator==(const ContentMetaInfo& other) const;
		bool operator!=(const ContentMetaInfo& other) const;

		// IByteModel
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		uint64_t getTitleId() const;
		void setTitleId(uint64_t title_id);
		
		uint32_t getVersion() const;
		void setVersion(uint32_t ver);
		
		cnmt::ContentMetaType getContentMetaType() const;
		void setContentMetaType(cnmt::ContentMetaType type);
		
		byte_t getAttributes() const;
		void setAttributes(byte_t attr);
		
	private:
		const std::string kModuleName = "CONTENT_META_INFO";

		// byte model
		fnd::Vec<byte_t> mRawBinary;

		// variables
		uint64_t mTitleId;
		uint32_t mVersion;
		cnmt::ContentMetaType mType;
		byte_t mAttributes;
	};
}
}