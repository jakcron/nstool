#include "NcaProcess.h"
#include <fnd/SimpleTextOutput.h>
#include <nx/NcaUtils.h>

std::string kFormatVersionStr[]
{
	"NCA2",
	"NCA3"
};

std::string kDistributionTypeStr[]
{
	"Download",
	"Game Card"
};

std::string kContentTypeStr[]
{
	"Program",
	"Meta",
	"Control",
	"Manual",
	"Data"
};

std::string kEncryptionTypeStr[]
{
	"Auto",
	"None",
	"AesXts",
	"AesCtr",
	"AesCtrEx"
};

std::string kHashTypeStr[]
{
	"Auto",
	"UNKNOWN_1",
	"HierarchicalSha256",
	"HierarchicalIntegrity"
};

std::string kFormatTypeStr[]
{
	"RomFs",
	"PartitionFs"
};

std::string kKaekIndexStr[]
{
	"Application",
	"Ocean",
	"System"
};

void NcaProcess::displayHeader()
{
	crypto::aes::sAes128Key zero_key;
	memset(zero_key.key, 0, sizeof(zero_key));

	printf("[NCA Header]\n");
	printf("  Format Type:     %s\n", kFormatVersionStr[mHdr.getFormatVersion()].c_str());
	printf("  Dist. Type:      %s\n", kDistributionTypeStr[mHdr.getDistributionType()].c_str());
	printf("  Content Type:    %s\n", kContentTypeStr[mHdr.getContentType()].c_str());
	printf("  Key Generation:  %d\n", mHdr.getKeyGeneration());
	printf("  Kaek Index:      %s (%d)\n", kKaekIndexStr[mHdr.getKaekIndex()].c_str(), mHdr.getKaekIndex());
	printf("  Size:            0x%" PRIx64 "\n", mHdr.getContentSize());
	printf("  ProgID:          0x%016" PRIx64 "\n", mHdr.getProgramId());
	printf("  Content Index:   %" PRIu32 "\n", mHdr.getContentIndex());
	uint32_t ver = mHdr.getSdkAddonVersion();
	printf("  SdkAddon Ver.:   v%d.%d.%d (v%" PRIu32 ")\n", (ver>>24 & 0xff),(ver>>16 & 0xff),(ver>>8 & 0xff), ver);
	printf("  RightsId:        ");
	fnd::SimpleTextOutput::hexDump(mHdr.getRightsId(), nx::nca::kRightsIdLen);
	printf("  Key Area Keys: (Encrypted)\n");
	for (size_t i = 0; i < mHdr.getEncAesKeys().getSize(); i++)
	{
		if (mHdr.getEncAesKeys()[i] != zero_key)
		{
			printf("    %2lu: ", i);
			fnd::SimpleTextOutput::hexDump(mHdr.getEncAesKeys()[i].key, crypto::aes::kAes128KeySize);
		}
	}

	if (mBodyKeyList.getSize() > 0)
	{
		printf("  Key Area Keys:\n");
		for (size_t i = 0; i < mBodyKeyList.getSize(); i++)
		{
			printf("    %2lu: ", i);
			fnd::SimpleTextOutput::hexDump(mBodyKeyList[i].key, crypto::aes::kAes128KeySize);
		}
	}
}

void NcaProcess::decryptBodyKeyList()
{
	crypto::aes::sAes128Key zero_key;
	memset(zero_key.key, 0, sizeof(zero_key));

	byte_t masterkey_rev = nx::NcaUtils::getMasterKeyRevisionFromKeyGeneration(mHdr.getKeyGeneration());
	byte_t keak_index = mHdr.getKaekIndex();

	crypto::aes::sAes128Key tmp_key;
	if (mKeyset->nca.key_area_key[keak_index][masterkey_rev] != zero_key)
	{
		for (size_t i = 0; i < 4; i++)
		{
			crypto::aes::AesEcbDecrypt(mHdr.getEncAesKeys()[i].key, 0x10, mKeyset->nca.key_area_key[keak_index][masterkey_rev].key, tmp_key.key);
			mBodyKeyList.addElement(tmp_key);
		}
	}
}

NcaProcess::NcaProcess() :
	mReader(nullptr),
	mOffset(0),
	mKeyset(nullptr),
	mCliOutputType(OUTPUT_NORMAL),
	mVerify(false)
{

}

void NcaProcess::process()
{
	fnd::MemoryBlob scratch;

	if (mReader == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}
	
	// read header block
	mReader->read((byte_t*)&mHdrBlock, mOffset, sizeof(nx::sNcaHeaderBlock));
	
	// decrypt header block
	nx::NcaUtils::decryptNcaHeader((byte_t*)&mHdrBlock, (byte_t*)&mHdrBlock, mKeyset->nca.header_key);

	// proccess main header
	mHdr.importBinary((byte_t*)&mHdrBlock.header, sizeof(nx::sNcaHeader));

	// decrypt key area
	decryptBodyKeyList();

	if (mCliOutputType >= OUTPUT_NORMAL)
		displayHeader();
}

void NcaProcess::setInputFile(fnd::IFile& reader)
{
	mReader = &reader;
}

void NcaProcess::setInputFileOffset(size_t offset)
{
	mOffset = offset;
}

void NcaProcess::setKeyset(const sKeyset* keyset)
{
	mKeyset = keyset;
}

void NcaProcess::setCliOutputMode(CliOutputType type)
{
	mCliOutputType = type;
}

void NcaProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}