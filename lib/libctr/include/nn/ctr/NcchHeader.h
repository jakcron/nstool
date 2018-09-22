/*
NcchHeader.h
(c) 2018 Jakcron

This is a 0x100 byte header used to map/hash sections in CXI/CFA files, is preceeded by a RSA-2048 signature over 
these 0x100 bytes in such files. It is also used in CCI files in the CardInfo structure

#Implementation Details
	- sizes/offsets are in blocks (usually 512 bytes)
	- all offsets in the raw structure are from the start of the file, not the start of the header.
*/
#pragma once
#include <string>

#include <fnd/types.h>
#include <fnd/sha.h>
#include <fnd/Vec.h>
#include <fnd/List.h>
#include <fnd/ISerialisable.h>
#include <nn/ctr/ProgramId.h>
#include <nn/ctr/ncch.h>

namespace nn
{
namespace ctr
{
	class NcchHeader : public fnd::ISerialisable
	{
	public:
		class Layout
		{
		public:
			Layout() :
				mOffset(0),
				mSize(0),
				mHashedSize(0),
				mHash()
			{}

			void operator=(const Layout& other)
			{
				mOffset = other.mOffset;
				mSize = other.mSize;
				mHashedSize = other.mHashedSize;
				mHash = other.mHash;
			}

			bool operator==(const Layout& other) const
			{
				return (mOffset == other.mOffset) \
					&& (mSize == other.mSize) \
					&& (mHashedSize == other.mHashedSize) \
					&& (mHash == other.mHash);
			}

			bool operator!=(const Layout& other) const
			{
				return !(*this == other);
			}

			void setOffset(size_t offset)
			{
				mOffset = offset;
			}

			size_t getOffset() const
			{
				return mOffset;
			}

			void setSize(size_t size)
			{
				mSize = size;
			}

			size_t getSize() const
			{
				return mSize;
			}

			void setHashedSize(size_t hashed_size)
			{
				mHashedSize = hashed_size;
			}

			size_t getHashedSize() const
			{
				return mHashedSize;
			}

			void setHash(const fnd::sha::sSha256Hash& hash)
			{
				mHash = hash;
			}

			const fnd::sha::sSha256Hash& getHash() const
			{
				return mHash;
			}
		private:
			size_t mOffset;
			size_t mSize;
			size_t mHashedSize;
			fnd::sha::sSha256Hash mHash;
		};

		// constructors
		NcchHeader();
		NcchHeader(const NcchHeader& other);

		void operator=(const NcchHeader& other);
		bool operator==(const NcchHeader& other) const;
		bool operator!=(const NcchHeader& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* data, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		// data layout mutators
		void setNcchBinarySize(size_t size);
		size_t getNcchBinarySize() const;

		void setExtendedHeaderLayout(const Layout layout);
		const Layout& getExtendedHeaderLayout() const;

		void setPlainRegionLayout(const Layout layout);
		const Layout& getPlainRegionLayout() const;

		void setLogoLayout(const Layout layout);
		const Layout& getLogoLayout() const;

		void setExefsLayout(const Layout layout);
		const Layout& getExefsLayout() const;

		void setRomfsLayout(const Layout layout);
		const Layout& getRomfsLayout() const;

		// property mutators
		void setNcchType(ncch::NcchType type);
		ncch::NcchType getNcchType() const;

		void setNewCryptoFlag(ncch::NewCryptoFlag flag);
		ncch::NewCryptoFlag getNewCryptoFlag() const;

		void setPlatform(ncch::Platform platform);
		ncch::Platform getPlatform() const;

		void setFormType(ncch::FormType type);
		ncch::FormType getFormType() const;

		void setContentType(ncch::ContentType type);
		ncch::ContentType getContentType() const;

		void setBlockSize(ncch::BlockSizeFlag flag);
		ncch::BlockSizeFlag getBlockSize() const;

		void setOtherFlagList(const fnd::List<ncch::OtherFlag>& list);
		const fnd::List<ncch::OtherFlag>& getOtherFlagList() const;

		void setCompanyCode(const std::string& company_code);
		const std::string& getCompanyCode() const;

		void setTitleId(const ctr::ProgramId& id);
		const ctr::ProgramId& getTitleId() const;

		void setProgramId(const ctr::ProgramId& id);
		const ctr::ProgramId& getProgramId() const;

		void setProductCode(const std::string& product_code);
		const std::string& getProductCode() const;

		void setSeedChecksum(uint32_t checksum);
		uint32_t getSeedChecksum() const;

	private:
		const std::string kModuleName = "NCCH_HEADER";

		// serialised data
		fnd::Vec<byte_t> mRawBinary;

		// members
		size_t mNcchBinarySize;
		Layout mExHeaderLayout;
		Layout mPlainRegionLayout;
		Layout mLogoLayout;
		Layout mExefsLayout;
		Layout mRomfsLayout;

		std::string mCompanyCode;
		std::string mProductCode;

		uint32_t mSeedChecksum;

		ncch::NcchType mNcchType;
		ProgramId mTitleId;
		ProgramId mProgramId;

		ncch::NewCryptoFlag mNewCryptoFlag;
		ncch::Platform mPlatform;
		ncch::FormType mFormType;
		ncch::ContentType mContentType;
		ncch::BlockSizeFlag mBlockSize;
		fnd::List<ncch::OtherFlag> mOtherFlagList;
	};
}
}