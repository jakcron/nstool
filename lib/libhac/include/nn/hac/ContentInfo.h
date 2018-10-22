#pragma once
#include <string>
#include <cstring>
#include <fnd/IByteModel.h>
#include <nn/hac/cnmt.h>

namespace nn
{
namespace hac
{
	class ContentInfo :
		public fnd::IByteModel
	{
	public:
		ContentInfo();
		ContentInfo(const ContentInfo& other);

		void operator=(const ContentInfo& other);
		bool operator==(const ContentInfo& other) const;
		bool operator!=(const ContentInfo& other) const;

		// IByteModel
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		const fnd::sha::sSha256Hash& getContentHash() const;
		void setContentHash(const fnd::sha::sSha256Hash& hash);

		const cnmt::sContentId& getContentId() const;
		void setContentId(const cnmt::sContentId& content_id);

		size_t getContentSize() const;
		void setContentSize(size_t size);

		cnmt::ContentType getContentType() const;
		void setContentType(cnmt::ContentType type);

		byte_t getIdOffset() const;
		void setIdOffset(byte_t id_offset);

	private:
		const std::string kModuleName = "CONTENT_INFO";

		// binary blob
		fnd::Vec<byte_t> mRawBinary;

		// variables
		fnd::sha::sSha256Hash mHash;
		cnmt::sContentId mContentId;
		size_t mSize;
		cnmt::ContentType mType;
		byte_t mIdOffset;
	};
}
}