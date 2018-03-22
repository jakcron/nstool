#include <cstdio>
#include <crypto/aes.h>
#include <fnd/io.h>
#include <fnd/MemoryBlob.h>
#include <nx/NXCrypto.h>
#include <nx/NcaHeader.h>
#include <inttypes.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

const size_t kNcaSectorSize = nx::NcaHeader::kBlockSize;

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
	"BKTR"
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

enum EncryptionType
{
	CRYPT_AUTO,
	CRYPT_NONE,
	CRYPT_AESXTS,
	CRYPT_AESCTR,
	CRYPT_BKTR
};

static const byte_t kNcaMagic[2][4] = {{'N','C','A','2'}, {'N','C','A','3'}};

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

#pragma pack(push,1)
struct sNcaFsHeader
{
	le_uint16_t version; // usually 0x0002
	byte_t format_type; // RomFs(0x00), PartitionFs(0x01)
	byte_t hash_type; // HashTypeAuto(0x00), HashTypeHierarchicalSha256(0x02), HashTypeHierarchicalIntegrity(0x03).RomFs uses (0x03) this is forced, PartitionFs uses (0x02).
	byte_t encryption_type; // EncryptionTypeAuto(0x00), EncryptionTypeNone(0x01), EncryptionTypeAesCtr(0x03)
	byte_t reserved[3];
};
#pragma pack(pop)

inline size_t sectorToOffset(size_t sector_index) { return sector_index * kNcaSectorSize; }

void initNcaCtr(byte_t ctr[crypto::aes::kAesBlockSize], uint32_t generation)
{
	memset(ctr, 0, crypto::aes::kAesBlockSize);
	for (size_t i = 0; i < 4; i++)
	{
		ctr[7 - i] = (generation >> i * 8) & 0xff;
	}
}

void hexDump(const byte_t* data, size_t len)
{
	for (size_t i = 0; i < len; i++)
	{
		printf("%02X", data[i]);
	}
}

void xorData(const byte_t* a, const byte_t* b, byte_t* out, size_t len)
{
	for (size_t i = 0; i < len; i++)
	{
		out[i] = a[i] ^ b[i];
	}
}

void decryptNcaHeader(byte_t header[0xc00], const byte_t* key[2])
{
	byte_t tweak[crypto::aes::kAesBlockSize];

	// decrypt main header
	byte_t raw_hdr[kNcaSectorSize];
	nx::NcaHeader hdr;
	crypto::aes::AesXtsMakeTweak(tweak, 1);
	crypto::aes::AesXtsDecryptSector(header + sectorToOffset(1), kNcaSectorSize, key[0], key[1], tweak, raw_hdr);
	hdr.importBinary(raw_hdr, kNcaSectorSize);

	// decrypt whole header
	for (size_t i = 0; i < 6; i++)
	{
		crypto::aes::AesXtsMakeTweak(tweak, (i > 1 && hdr.getFormatVersion() == nx::NcaHeader::NCA2_FORMAT)? 0 : i);
		crypto::aes::AesXtsDecryptSector(header + sectorToOffset(i), kNcaSectorSize, key[0], key[1], tweak, header + sectorToOffset(i));
	}
}

void decryptNcaSectorXts(const fnd::MemoryBlob& nca, byte_t out[kNcaSectorSize], size_t sector, const byte_t* key[2])
{
	byte_t tweak[crypto::aes::kAesBlockSize];
	crypto::aes::AesXtsMakeTweak(tweak, sector);
	crypto::aes::AesXtsDecryptSector(nca.getBytes() + sectorToOffset(sector), kNcaSectorSize, key[0], key[1], tweak, out);
}

void decryptNcaSectorCtr(const fnd::MemoryBlob& nca, byte_t out[kNcaSectorSize], size_t sector, const byte_t* key)
{
	byte_t ctr[crypto::aes::kAesBlockSize];
	initNcaCtr(ctr, 0);
	crypto::aes::AesIncrementCounter(ctr, (sector*kNcaSectorSize)/crypto::aes::kAesBlockSize, ctr);
	crypto::aes::AesCtr(nca.getBytes() + sector*kNcaSectorSize, kNcaSectorSize, key, ctr, out);
}

void dumpNcaSector(byte_t out[kNcaSectorSize])
{
	for (size_t j = 0; j < kNcaSectorSize / crypto::aes::kAesBlockSize; j++)
	{
		hexDump(out + j * crypto::aes::kAesBlockSize, crypto::aes::kAesBlockSize);
		printf("\n");
	}
}

void dumpHxdStyleSector(byte_t* out, size_t len)
{
	// iterate over 0x10 blocks
	for (size_t i = 0; i < (len / crypto::aes::kAesBlockSize); i++)
	{
		// for block i print each byte
		for (size_t j = 0; j < crypto::aes::kAesBlockSize; j++)
		{
			printf("%02X ", out[i*crypto::aes::kAesBlockSize + j]);
		}
		printf(" ");
		for (size_t j = 0; j < crypto::aes::kAesBlockSize; j++)
		{
			printf("%c", isalnum(out[i*crypto::aes::kAesBlockSize + j]) ? out[i*crypto::aes::kAesBlockSize + j] : '.');
		}
		printf("\n");
	}

	/*
	for (size_t i = 0; i < len % crypto::aes::kAesBlockSize; i++)
	{
		printf("%02X ", out[(len / crypto::aes::kAesBlockSize)*crypto::aes::kAesBlockSize + i]);
	}
	for (size_t i = 0; i < crypto::aes::kAesBlockSize - (len % crypto::aes::kAesBlockSize); i++)
	{
		printf("   ");
	}
	for (size_t i = 0; i < len % crypto::aes::kAesBlockSize; i++)
	{
		printf("%c", out[(len / crypto::aes::kAesBlockSize)*crypto::aes::kAesBlockSize + i]);
	}
	*/
}


bool testNcaHeaderKey(const byte_t* header_src, const byte_t* key[2])
{
	bool validKey = false;
	byte_t header_dec[kNcaSectorSize];
	byte_t tweak[crypto::aes::kAesBlockSize];

	// try key
	crypto::aes::AesXtsMakeTweak(tweak, 1);
	crypto::aes::AesXtsDecryptSector(header_src + sectorToOffset(1), kNcaSectorSize, key[0], key[1], tweak, header_dec);
	if (memcmp(header_dec, kNcaMagic[0], 4) == 0 || memcmp(header_dec, kNcaMagic[1], 4) == 0)
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
		fnd::io::readFile(argv[1], nca);

		KeysetType keyset = getKeysetFromNcaHeader(nca.getBytes());

		decryptNcaHeader(nca.getBytes(), kNcaHeaderKey[keyset]);
		//dumpHxdStyleSector(nca.getBytes(), 0xc00);

		// nca test
		if (argc == 2 || argc == 3)
		{
			//decryptNcaSectorXts(nca, sector, 1, crypto::aes::nx::dev::nca_header_key[0], crypto::aes::nx::dev::nca_header_key[1]);

			nx::NcaHeader hdr;
			hdr.importBinary(nca.getBytes() + sectorToOffset(1), kNcaSectorSize);

			printf("[NCA Header]\n");
			printf("  Format Type:     %s\n", kFormatVersionStr[hdr.getFormatVersion()].c_str());
			printf("  Dist. Type:      %s\n", kDistributionTypeStr[hdr.getDistributionType()].c_str());
			printf("  Type:            %s\n", kContentTypeStr[hdr.getContentType()].c_str());
			printf("  Crypto Type:     %d\n", hdr.getCryptoType());
			printf("  Kaek Index:      %s (%d)\n", kKaekIndexStr[hdr.getKaekIndex()].c_str(), hdr.getKaekIndex());
			printf("  Size:            0x%" PRIx64 "\n", hdr.getNcaSize());
			printf("  ProgID:          0x%016" PRIx64 "\n", hdr.getProgramId());
			printf("  Content. Idx:    %" PRIu32 "\n", hdr.getContentIndex());
			uint32_t ver = hdr.getSdkAddonVersion();
			printf("  SdkAddon Ver.:   v%d.%d.%d.%d (v%" PRIu32 ")\n", (ver>>24 & 0xff),(ver>>16 & 0xff),(ver>>8 & 0xff),(ver>>0 & 0xff), ver);
			printf("  Encrypted Key Area:\n");
			for (size_t i = 0; i < hdr.getEncAesKeys().getSize(); i++)
			{
				printf("    %lu: ", i);
				hexDump(hdr.getEncAesKeys()[i].key, crypto::aes::kAes128KeySize);
				printf("\n");
				/*
				byte_t key[crypto::aes::kAes128KeySize];
				crypto::aes::AesEcbDecrypt(hdr.getEncAesKeys()[i].key, crypto::aes::kAes128KeySize, crypto::aes::nx::dev::key_area_encryption_key_0, key);
				printf("    dec: ", i);
				hexDump(key, crypto::aes::kAes128KeySize);
				printf("\n");
				*/
			}
			
			printf("  Sections:\n");
			for (size_t i = 0; i < hdr.getSections().getSize(); i++)
			{
				const nx::NcaHeader::sSection& section = hdr.getSections()[i];
				printf("    %lu:\n", i);
				//printf("      Start Blk: %" PRId32 "\n", section.start_blk);
				//printf("      End Blk:   %" PRId32 "\n", section.end_blk);
				printf("      Offset:      0x%" PRIx64 "\n", section.offset);
				printf("      Size:        0x%" PRIx64 "\n", section.size);
				

				size_t sector_index = 1 + (hdr.getSections().getSize() - i);

				byte_t hash[crypto::sha::kSha256HashLen];
				crypto::sha::Sha256(nca.getBytes() + sectorToOffset(sector_index), kNcaSectorSize, hash);
				if (section.hash.compare(hash) == false)
				{
					throw fnd::Exception("ncatool", "NcaFsHeader has bad sha256 hash");
				}

				const sNcaFsHeader* fsHdr = (const sNcaFsHeader*)(nca.getBytes() + sectorToOffset(sector_index));
				printf("      FsHeader:\n");
				printf("        Version:     0x%d\n", fsHdr->version.get());
				printf("        Format Type: %s\n", kFormatTypeStr[fsHdr->format_type].c_str());
				printf("        Hash Type:   %s\n", kHashTypeStr[fsHdr->hash_type].c_str());
				printf("        Enc. Type:   %s\n", kEncryptionTypeStr[fsHdr->encryption_type].c_str());
				/*
				printf("      Hash:        ");
				hexDump(section.hash.bytes, crypto::sha::kSha256HashLen);
				printf("\n");
				byte_t hash[crypto::sha::kSha256HashLen];
				crypto::sha::Sha256(nca.getBytes() + sectorToOffset(sector_index), kNcaSectorSize, hash);
				printf("      Hash:        ");
				hexDump(hash, crypto::sha::kSha256HashLen);
				printf("\n");
				*/
				//dumpHxdStyleSector(nca.getBytes() + sectorToOffset(sector_index), 0x10);

			}
			

			

#ifdef USE_OLD_CODE
			if (argc == 3)
			{
#ifdef _WIN32
				_mkdir(argv[2]);
#else
				mkdir(argv[2], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

				for (size_t i = 0; i < hdr.getSections().getSize(); i++)
				{
					const nx::NcaHeader::sSection& section = hdr.getSections()[i];
#ifdef _WIN32
					fnd::io::writeFile(std::string(argv[2]) + "\\" + std::to_string(i) + ".bin" , nca.getBytes() + section.offset, section.size);
#else
					fnd::io::writeFile(std::string(argv[2]) + "/" + std::to_string(i) + ".bin", nca.getBytes() + section.offset, section.size);
#endif
				}
			}
		}
		if (argc == 4)
		{
			printf("decrypt test\n");
			byte_t sect[kNcaSectorSize];;
			for (size_t i = 0; i < 6; i++)
			{
				decryptNcaSectorXts(nca, sect, i, crypto::aes::nx::dev::nca_header_key[0], crypto::aes::nx::dev::nca_header_key[1]);
				dumpNcaSector(sect);
			}
		}
#endif
		}
	} catch (const fnd::Exception& e)
	{
		printf("%s\n",e.what());
	}

	return 0;
}