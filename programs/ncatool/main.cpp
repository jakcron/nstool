#include <cstdio>
#include <crypto/aes.h>
#include <fnd/io.h>
#include <fnd/memory_blob.h>
#include <nx/NXCrypto.h>
#include <nx/NcaHeader.h>
#include <inttypes.h>

const size_t kNcaSectorSize = nx::NcaHeader::kDefaultBlockSize;

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
		if (argc == 2)
		{
			decryptNcaSectorXts(nca, sector, 1, crypto::aes::nx::nca_header_key[0], crypto::aes::nx::nca_header_key[1]);

			nx::NcaHeader hdr;
			hdr.importBinary(sector);

			printf("[NCA Header]\n");
			printf("  Size:       0x%" PRIx64 "\n", hdr.getNcaSize());
			printf("  ProgID:     0x%016" PRIx64 "\n", hdr.getProgramId());
			printf("  Unk0:       0x%" PRIx32 "\n", hdr.getUnk());
			printf("  Sections:\n");
			for (size_t i = 0; i < hdr.getSections().getSize(); i++)
			{
				const nx::NcaHeader::sSection& section = hdr.getSections()[i];
				printf("    %lu:\n", i);
				//printf("      Start Blk: %" PRId32 "\n", section.start_blk);
				//printf("      End Blk:   %" PRId32 "\n", section.end_blk);
				printf("      Offset:    0x%" PRIx64 "\n", section.offset);
				printf("      Size:      0x%" PRIx64 "\n", section.size);
				printf("      KeyType:   0x%02x\n", section.key_type);
				printf("      Hash:      ");
				hexDump(section.hash.bytes, crypto::sha::kSha256HashLen);
				printf("\n");
			}
			printf("  AES Keys:\n");
			for (size_t i = 0; i < hdr.getAesKeys().getSize(); i++)
			{
				printf("    %lu: ", i);
				hexDump(hdr.getAesKeys()[i].key, crypto::aes::kAes128KeySize);
				printf("\n");
			}


		}
	} catch (const fnd::Exception& e)
	{
		printf("[%s%sERROR] %s\n", e.module(), strlen(e.module()) > 0 ? " " : "", e.what());
	}

	return 0;
}