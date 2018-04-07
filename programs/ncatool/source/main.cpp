#include <cstdio>
#include <crypto/aes.h>
#include <fnd/io.h>
#include <fnd/MemoryBlob.h>
#include <fnd/SimpleTextOutput.h>
#include <nx/NXCrypto.h>
#include <nx/NcaHeader.h>
#include <nx/PfsHeader.h>
#include <inttypes.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

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


enum KeysetType
{
	KEYSET_DEV,
	KEYSET_PROD
};

static const byte_t* kNcaHeaderKey[2][2] = 
{
	{ crypto::aes::nx::dev::nca_header_key[0], crypto::aes::nx::dev::nca_header_key[1] },
	{ crypto::aes::nx::prod::nca_header_key[0], crypto::aes::nx::prod::nca_header_key[1] }
};

inline size_t sectorToOffset(size_t sector_index) { return sector_index * nx::nca::kSectorSize; }

void initNcaCtr(byte_t ctr[crypto::aes::kAesBlockSize], uint32_t generation)
{
	memset(ctr, 0, crypto::aes::kAesBlockSize);
	for (size_t i = 0; i < 4; i++)
	{
		ctr[7 - i] = (generation >> i * 8) & 0xff;
	}
}

void decryptNcaHeader(byte_t header[nx::nca::kHeaderSize], const byte_t* key[2])
{
	byte_t tweak[crypto::aes::kAesBlockSize];

	// decrypt main header
	byte_t raw_hdr[nx::nca::kSectorSize];
	nx::NcaHeader hdr;
	crypto::aes::AesXtsMakeTweak(tweak, 1);
	crypto::aes::AesXtsDecryptSector(header + sectorToOffset(1), nx::nca::kSectorSize, key[0], key[1], tweak, raw_hdr);
	hdr.importBinary(raw_hdr, nx::nca::kSectorSize);

	bool useNca2SectorIndex = hdr.getFormatVersion() == nx::NcaHeader::NCA2_FORMAT;

	// decrypt whole header
	for (size_t i = 0; i < nx::nca::kHeaderSectorNum; i++)
	{
		crypto::aes::AesXtsMakeTweak(tweak, (i > 1 && useNca2SectorIndex)? 0 : i);
		crypto::aes::AesXtsDecryptSector(header + sectorToOffset(i), nx::nca::kSectorSize, key[0], key[1], tweak, header + sectorToOffset(i));
	}
}

bool testNcaHeaderKey(const byte_t* header_src, const byte_t* key[2])
{
	bool validKey = false;
	byte_t header_dec[nx::nca::kSectorSize];
	byte_t tweak[crypto::aes::kAesBlockSize];

	// try key
	crypto::aes::AesXtsMakeTweak(tweak, 1);
	crypto::aes::AesXtsDecryptSector(header_src + sectorToOffset(1), nx::nca::kSectorSize, key[0], key[1], tweak, header_dec);
	if (memcmp(header_dec, nx::nca::kNca2Sig.c_str(), 4) == 0 || memcmp(header_dec, nx::nca::kNca3Sig.c_str(), 4) == 0)
	{
		validKey = true;
	}

	return validKey;
}

KeysetType getKeysetFromNcaHeader(const byte_t* header_src)
{
	for (int i = 0; i < 2; i++)
	{
		if (testNcaHeaderKey(header_src, kNcaHeaderKey[i]) == true)
		{
			return (KeysetType)i;
		}
	}
	
	throw fnd::Exception("Failed to determine NCA header key");
}

void printHeader(const byte_t* header)
{
	nx::NcaHeader hdr;
	hdr.importBinary(header + sectorToOffset(1), nx::nca::kSectorSize);

	printf("[NCA Header]\n");
	printf("  Format Type:     %s\n", kFormatVersionStr[hdr.getFormatVersion()].c_str());
	printf("  Dist. Type:      %s\n", kDistributionTypeStr[hdr.getDistributionType()].c_str());
	printf("  Content Type:    %s\n", kContentTypeStr[hdr.getContentType()].c_str());
	printf("  Key Generation:  %d\n", hdr.getKeyGeneration());
	printf("  Kaek Index:      %s (%d)\n", kKaekIndexStr[hdr.getKaekIndex()].c_str(), hdr.getKaekIndex());
	printf("  Size:            0x%" PRIx64 "\n", hdr.getContentSize());
	printf("  ProgID:          0x%016" PRIx64 "\n", hdr.getProgramId());
	printf("  Content Index:   %" PRIu32 "\n", hdr.getContentIndex());
	uint32_t ver = hdr.getSdkAddonVersion();
	printf("  SdkAddon Ver.:   v%d.%d.%d (v%" PRIu32 ")\n", (ver>>24 & 0xff),(ver>>16 & 0xff),(ver>>8 & 0xff), ver);
	printf("  RightsId:        ");
	fnd::SimpleTextOutput::hexDump(hdr.getRightsId(), 0x10);
	printf("\n");
	printf("  Encrypted Key Area:\n");
	crypto::aes::sAes128Key zero_key;
	memset(zero_key.key, 0, sizeof(zero_key));
	for (size_t i = 0; i < hdr.getEncAesKeys().getSize(); i++)
	{
		if (hdr.getEncAesKeys()[i] != zero_key)
		{
			printf("    %2lu: ", i);
			fnd::SimpleTextOutput::hexDump(hdr.getEncAesKeys()[i].key, crypto::aes::kAes128KeySize);
			printf("\n");
		}
	}
	
	printf("  Sections:\n");
	for (size_t i = 0; i < hdr.getPartitions().getSize(); i++)
	{
		const nx::NcaHeader::sPartition& partition = hdr.getPartitions()[i];
		printf("    %lu:\n", i);
		//printf("      Start Blk: %" PRId32 "\n", partition.start_blk);
		//printf("      End Blk:   %" PRId32 "\n", partition.end_blk);
		printf("      Index:       %d\n", partition.index);
		printf("      Offset:      0x%" PRIx64 "\n", partition.offset);
		printf("      Size:        0x%" PRIx64 "\n", partition.size);
		

		size_t sector_index = 2 + partition.index;
		
		crypto::sha::sSha256Hash ncaFsHeaderHash;
		crypto::sha::Sha256(header + sectorToOffset(sector_index), nx::nca::kSectorSize, ncaFsHeaderHash.bytes);
		if (partition.hash.compare(ncaFsHeaderHash) == false)
		{
			throw fnd::Exception("ncatool", "NcaFsHeader has bad sha256 hash");
		}

		const nx::sNcaFsHeader* fsHdr = (const nx::sNcaFsHeader*)(header + sectorToOffset(sector_index));
		printf("      FsHeader:\n");
		printf("        Version:     0x%d\n", fsHdr->version.get());
		printf("        Format Type: %s\n", kFormatTypeStr[fsHdr->format_type].c_str());
		printf("        Hash Type:   %s\n", kHashTypeStr[fsHdr->hash_type].c_str());
		printf("        Enc. Type:   %s\n", kEncryptionTypeStr[fsHdr->encryption_type].c_str());
		if (fsHdr->format_type == nx::nca::FORMAT_ROMFS)
		{

		}
		else if (fsHdr->format_type == nx::nca::FORMAT_PFS0)
		{
			const nx::sPfsSuperBlock* pfs0 = (const nx::sPfsSuperBlock*)(header + sectorToOffset(sector_index) + sizeof(nx::sNcaFsHeader));
			printf("      PFS0 SuperBlock:\n");
			printf("        Master Hash:       \n");
			printf("        HashBlockSize:     0x%x\n", pfs0->hash_block_size.get());
			printf("        Unknown:           0x%x\n", pfs0->unk_0x02.get());
			printf("        HashDataOffset:    0x%" PRIx64 "\n", pfs0->hash_data.offset.get());
			printf("        HashDataSize:      0x%" PRIx64 "\n", pfs0->hash_data.size.get());
			printf("        HashTargetOffset:  0x%" PRIx64 "\n", pfs0->hash_target.offset.get());
			printf("        HashTargetSize:    0x%" PRIx64 "\n", pfs0->hash_target.size.get());
		
		}
		
	}
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("usage: ncatool <nca file>\n");
		return 1;
	}

	try
	{
		fnd::MemoryBlob nca;
		fnd::io::readFile(argv[1], 0x0, nx::nca::kHeaderSize, nca);

		KeysetType keyset = getKeysetFromNcaHeader(nca.getBytes());

		decryptNcaHeader(nca.getBytes(), kNcaHeaderKey[keyset]);

		printHeader(nca.getBytes());
	} catch (const fnd::Exception& e)
	{
		printf("%s\n",e.what());
	}

	return 0;
}