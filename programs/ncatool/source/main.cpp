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


void initNcaCtr(u8 ctr[crypto::aes::kAesBlockSize], u32 generation)
{
	memset(ctr, 0, crypto::aes::kAesBlockSize);
	for (size_t i = 0; i < 4; i++)
	{
		ctr[7 - i] = (generation >> i * 8) & 0xff;
	}
}

void hexDump(const u8* data, size_t len)
{
	for (size_t i = 0; i < len; i++)
	{
		printf("%02X", data[i]);
	}
}

void xorData(const u8* a, const u8* b, u8* out, size_t len)
{
	for (size_t i = 0; i < len; i++)
	{
		out[i] = a[i] ^ b[i];
	}
}

void decryptNcaSectorXts(const fnd::MemoryBlob& nca, u8 out[kNcaSectorSize], size_t sector, const u8* key1, const u8* key2)
{
	u8 tweak[crypto::aes::kAesBlockSize];
	crypto::aes::AesXtsMakeTweak(tweak, sector);
	crypto::aes::AesXtsDecryptSector(nca.getBytes() + sector*kNcaSectorSize, kNcaSectorSize, key1, key2, tweak, out);
}

void decryptNcaSectorCtr(const fnd::MemoryBlob& nca, u8 out[kNcaSectorSize], size_t sector, const u8* key)
{
	u8 ctr[crypto::aes::kAesBlockSize];
	initNcaCtr(ctr, 0);
	crypto::aes::AesIncrementCounter(ctr, (sector*kNcaSectorSize)/crypto::aes::kAesBlockSize, ctr);
	crypto::aes::AesCtr(nca.getBytes() + sector*kNcaSectorSize, kNcaSectorSize, key, ctr, out);
}

void dumpNcaSector(u8 out[kNcaSectorSize])
{
	for (size_t j = 0; j < kNcaSectorSize / crypto::aes::kAesBlockSize; j++)
	{
		hexDump(out + j * crypto::aes::kAesBlockSize, crypto::aes::kAesBlockSize);
		printf("\n");
	}
}

void dumpHxdStyleSector(u8* out, size_t len)
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
	"UNKNOWN_2",
	"AesCtr"
};

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

		u8 sector[kNcaSectorSize];

		// nca test
		if (argc == 2 || argc == 3)
		{
			decryptNcaSectorXts(nca, sector, 1, crypto::aes::nx::dev::nca_header_key[0], crypto::aes::nx::dev::nca_header_key[1]);

			nx::NcaHeader hdr;
			hdr.importBinary(sector, kNcaSectorSize);

			printf("[NCA Header]\n");
			printf("  Dist. Type:      %s\n", kDistributionTypeStr[hdr.getDistributionType()].c_str());
			printf("  Type:            %s\n", kContentTypeStr[hdr.getContentType()].c_str());
			printf("  Enc. Type:       %s\n", kEncryptionTypeStr[hdr.getEncryptionType()].c_str());
			printf("  KeyIndex:        %d\n", hdr.getKeyIndex());
			printf("  Size:            0x%" PRIx64 "\n", hdr.getNcaSize());
			printf("  ProgID:          0x%016" PRIx64 "\n", hdr.getProgramId());
			printf("  Content. Idx:    %" PRIu32 "\n", hdr.getContentIndex());
			printf("  SdkAddon Ver.:   v%" PRIu32 "\n", hdr.getSdkAddonVersion());
			printf("  Sections:\n");
			for (size_t i = 0; i < hdr.getSections().getSize(); i++)
			{
				const nx::NcaHeader::sSection& section = hdr.getSections()[i];
				printf("    %lu:\n", i);
				//printf("      Start Blk: %" PRId32 "\n", section.start_blk);
				//printf("      End Blk:   %" PRId32 "\n", section.end_blk);
				printf("      Offset:      0x%" PRIx64 "\n", section.offset);
				printf("      Size:        0x%" PRIx64 "\n", section.size);
				printf("      Enc. Type:   %s\n", kEncryptionTypeStr[section.enc_type].c_str());
				printf("      Hash:        ");
				hexDump(section.hash.bytes, crypto::sha::kSha256HashLen);
				printf("\n");
			}
			printf("  Encrypted Body Keys:\n");
			for (size_t i = 0; i < hdr.getEncAesKeys().getSize(); i++)
			{
				printf("    %lu: ", i);
				hexDump(hdr.getEncAesKeys()[i].key, crypto::aes::kAes128KeySize);
				printf("\n");
			}

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
			u8 sect[kNcaSectorSize];;
			for (size_t i = 0; i < 6; i++)
			{
				decryptNcaSectorXts(nca, sect, i, crypto::aes::nx::dev::nca_header_key[0], crypto::aes::nx::dev::nca_header_key[1]);
				dumpNcaSector(sect);
			}
		}
		
	} catch (const fnd::Exception& e)
	{
		printf("%s\n",e.what());
	}

	return 0;
}