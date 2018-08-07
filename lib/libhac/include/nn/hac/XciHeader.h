#pragma once
#include <nn/hac/xci.h>
#include <fnd/ISerialisable.h>
#include <fnd/List.h>

namespace nn
{
namespace hac
{
	class XciHeader : 
		public fnd::ISerialisable
	{
	public:
		XciHeader();
		XciHeader(const XciHeader& other);

		void operator=(const XciHeader& other);
		bool operator==(const XciHeader& other) const;
		bool operator!=(const XciHeader& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();
		uint32_t getRomAreaStartPage() const;
		void setRomAreaStartPage(uint32_t startPage);
		uint32_t getBackupAreaStartPage() const;
		void setBackupAreaStartPage(uint32_t startPage);
		byte_t getKekIndex() const;
		void setKekIndex(byte_t kekIndex);
		byte_t getTitleKeyDecIndex() const;
		void setTitleKeyDecIndex(byte_t index);
		byte_t getRomSizeType() const;
		void setRomSizeType(byte_t romSizeType);
		byte_t getCardHeaderVersion() const;
		void setCardHeaderVersion(byte_t version);
		byte_t getFlags() const;
		void setFlags(byte_t flags);
		uint64_t getPackageId() const;
		void setPackageId(uint64_t id);
		uint32_t getValidDataEndPage() const;
		void setValidDataEndPage(uint32_t page);
		const fnd::aes::sAesIvCtr& getAesCbcIv() const;
		void setAesCbcIv(const fnd::aes::sAesIvCtr& iv);
		uint64_t getPartitionFsAddress() const;
		void setPartitionFsAddress(uint64_t address);
		uint64_t getPartitionFsSize() const;
		void setPartitionFsSize(uint64_t size);
		const fnd::sha::sSha256Hash& getPartitionFsHash() const;
		void setPartitionFsHash(const fnd::sha::sSha256Hash& hash);
		const fnd::sha::sSha256Hash& getInitialDataHash() const;
		void setInitialDataHash(const fnd::sha::sSha256Hash& hash);
		uint32_t getSelSec() const;
		void setSelSec(uint32_t sel_sec);
		uint32_t getSelT1Key() const;
		void setSelT1Key(uint32_t sel_t1_key);
		uint32_t getSelKey() const;
		void setSelKey(uint32_t sel_key);
		uint32_t getLimAreaPage() const;
		void setLimAreaPage(uint32_t page);

		uint32_t getFwVerMajor() const;
		void setFwVerMajor(uint32_t ver);
		uint32_t getFwVerMinor() const;
		void setFwVerMinor(uint32_t ver);
		uint32_t getAccCtrl1() const;
		void setAccCtrl1(uint32_t acc_ctrl_1);
		uint32_t getWait1TimeRead() const;
		void setWait1TimeRead(uint32_t seconds);
		uint32_t getWait2TimeRead() const;
		void setWait2TimeRead(uint32_t seconds);
		uint32_t getWait1TimeWrite() const;
		void setWait1TimeWrite(uint32_t seconds);
		uint32_t getWait2TimeWrite() const;
		void setWait2TimeWrite(uint32_t seconds);
		uint32_t getFwMode() const;
		void setFwMode(uint32_t fw_mode);
		uint32_t getUppVersion() const;
		void setUppVersion(uint32_t version);
		const byte_t* getUppHash() const;
		void setUppHash(const byte_t* hash);
		uint64_t getUppId() const;
		void setUppId(uint64_t id);

	private:
		const std::string kModuleName = "XCI_HEADER";

		// binary
		fnd::Vec<byte_t> mRawBinary;

		// data
		uint32_t mRomAreaStartPage;
		uint32_t mBackupAreaStartPage;
		byte_t mKekIndex;
		byte_t mTitleKeyDecIndex;
		byte_t mRomSize;
		byte_t mCardHeaderVersion;
		byte_t mFlags;
		uint64_t mPackageId;
		uint32_t mValidDataEndPage;
		fnd::aes::sAesIvCtr mAesCbcIv;
		uint64_t mPartitionFsHeaderAddress;
		uint64_t mPartitionFsHeaderSize;
		fnd::sha::sSha256Hash mPartitionFsHeaderHash;
		fnd::sha::sSha256Hash mInitialDataHash;
		uint32_t mSelSec;
		uint32_t mSelT1Key;
		uint32_t mSelKey;
		uint32_t mLimAreaPage;

		// Encrypted Data
		uint32_t mFwVersion[2];
		uint32_t mAccCtrl1;
		uint32_t mWait1TimeRead;
		uint32_t mWait2TimeRead;
		uint32_t mWait1TimeWrite;
		uint32_t mWait2TimeWrite;
		uint32_t mFwMode;
		uint32_t mUppVersion;
		byte_t mUppHash[8];
		uint64_t mUppId;
	};
}
}