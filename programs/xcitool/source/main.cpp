#include <cstdio>
#include <vector>
#include <string>
#include <crypto/aes.h>
#include <crypto/rsa.h>
#include <fnd/io.h>
#include <fnd/MemoryBlob.h>
#include <nx/NXCrypto.h>

enum RomSize
{
	ROM_SIZE_1GB = 0xFA,
	ROM_SIZE_2GB = 0xF8,
	ROM_SIZE_4GB = 0xF0,
	ROM_SIZE_8GB = 0xE0,
	ROM_SIZE_16GB = 0xE1,
	ROM_SIZE_32GB = 0xE2
};

enum XciHeaderFlags
{
	XCI_FLAG_AUTOBOOT,
	XCI_FLAG_HISTORY_ERASE
};

enum CardClockRate
{
	CLOCK_RATE_25 = 10551312,
	CLOCK_RATE_50 = 10551313,
};

static const size_t kXciPageSize = 0x200;
static const size_t kXciHeaderEncOffset = 0x90;
static const size_t kXciHeaderEncSize = 0x70;

#pragma pack (push, 1)
struct sXciHeader
{
	char signature[4]; // 0x00 // "HEAD"
	le_uint32_t rom_area_start_page; // 0x04
	le_uint32_t backup_area_start_page; // 0x08
	byte_t key_flag; // 0x0C // bit0-3 = KekIndex, bit4-7 = TitleKeyDecIndex
	byte_t rom_size; // 0x0D // this is an enum
	byte_t card_header_version; // 0x0E // CardHeaderVersion
	byte_t flags; // 0x0F
	le_uint64_t package_id; // 0x10 // stylised as 0x{0:x2}{1:x2}{2:x2}{3:x2}_{4:x2}{5:x2}{6:x2}{7:x2}
	le_uint32_t valid_data_end_page; // 0x18
	byte_t reserved_01[4]; // 0x1C
	byte_t encryption_iv[16]; // 0x20
	le_uint64_t partition_fs_header_address; // 0x30
	le_uint64_t partition_fs_header_size; // 0x38
	byte_t partition_fs_header_hash[0x20]; // 0x40
	byte_t initial_data_hash[0x20]; // 0x60
	le_uint32_t sel_sec; // 0x80
	le_uint32_t sel_t1_key; // 0x84 // SelT1Key
	le_uint32_t sel_key; // 0x88 // SelKey
	le_uint32_t lim_area; // 0x8C
	// START ENCRYPTION
	le_uint32_t fw_version[2]; // 0x90 // [0]=minor, [1]=major
	le_uint32_t acc_ctrl_1; // 0x98
	le_uint32_t wait_1_time_read; // 0x9C // Wait1TimeRead
	le_uint32_t wait_2_time_read; // 0xA0 // Wait2TimeRead
	le_uint32_t wait_1_time_write; // 0xA4 // Wait1TimeWrite
	le_uint32_t wait_2_time_write; // 0xA8 // Wait2TimeWrite
	le_uint32_t fw_mode; // 0xAC
	le_uint32_t cup_version; // 0xB0
	byte_t reserved_03[0x4]; // 0xB4
	byte_t upp_hash[8]; // 0xB8 // stylised as 0x{0:x2}{1:x2}{2:x2}{3:x2}_{4:x2}{5:x2}{6:x2}{7:x2}
	le_uint64_t cup_id; // 0xC0 // cup programID?
	byte_t reserved_04[0x38];
	// END ENCRYPTION
};

struct sInitialData
{
	byte_t key_source[16]; // { package_id[8], zeros[8]}
	byte_t title_key_enc[16];
	byte_t ccm_mac[16];
	byte_t ccm_nonce[12];
}; // sizeof() = 512 (1 page)

struct sKeyDataArea
{
	sInitialData initial_data; // AES128-CCM encrypted {titlekey[16]}
	byte_t encrypted_00[0x200*6]; // AES128-CTR encrypted {titlekey[16]}
	byte_t encrypted_00_aesctr_data[0x100]; // RSA2048-OAEP-SHA256 encrypted AES-CTR data used for encrypted_00 {key[16],iv[16]}
	byte_t reserved_01[0x100];
}; // sizeof() = 512*8 (8 pages)

#pragma pack (pop)

struct sXciKeyData
{
	crypto::aes::sAes128Key xci_header_encryption_key;
	crypto::aes::sAes128Key initial_data_key;
	crypto::rsa::sRsa2048Key xci_header_signer_key;
	crypto::rsa::sRsa2048Key card_key_area_oeap_key;
};

/*
void getTitleKeyFromInitialData(const byte_t* initialData, crypto::aes::sAes128Key& titleKey)
{
	const sInitialData* data = (const sInitialData*)initialData;
	crypto::aes::sAes128Key ccmKey;
	crypto::aes::AesEcbDecrypt(data->key_source, 16, key_data.initial_data_key.key, ccmKey.key);
	//crypto::aes::AesCcmDecrypt(data->title_key_enc, 16, ccmKey.key, data->ccm_nonce, data->ccm_mac, titleKey.key);
}
*/

inline uint64_t blockToAddr(uint32_t block)
{
	return ((uint64_t)block) << 9;
}

inline const char* getBoolStr(bool isTrue)
{
	return isTrue? "TRUE" : "FALSE";
}

inline const char* getRomSizeStr(byte_t rom_size)
{
	const char* str = "unknown";
	switch (rom_size)
	{
		case (ROM_SIZE_1GB) :
			str = "1GB";
			break;
		case (ROM_SIZE_2GB) :
			str = "2GB";
			break;
		case (ROM_SIZE_4GB) :
			str = "4GB";
			break;
		case (ROM_SIZE_8GB) :
			str = "8GB";
			break;
		case (ROM_SIZE_16GB) :
			str = "16GB";
			break;
		case (ROM_SIZE_32GB) :
			str = "32GB";
			break;
	}
	return str;
}

inline const char* getCardClockRate(uint32_t acc_ctrl_1)
{
	const char* str = "unknown";
	switch (acc_ctrl_1)
	{
		case (CLOCK_RATE_25) :
			str = "20 MHz";
			break;
		case (CLOCK_RATE_50) :
			str = "50 MHz";
			break;
		
	}
	return str;
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

void printXciHeader(const sXciHeader& hdr, bool is_decrypted)
{
	be_uint64_t *aes_iv, *hash;

	printf("[XCI HEADER]\n");
	printf("  Magic:                HEAD\n");
	printf("  RomAreaStartPage:     0x%0x (0x%" PRIx64 ")\n", hdr.rom_area_start_page.get(), blockToAddr(hdr.rom_area_start_page.get()));
	printf("  BackupAreaStartPage:  0x%0x\n", hdr.backup_area_start_page.get());
	printf("  KeyFlag:              0x%x\n", hdr.key_flag);
	printf("    KekIndex:           %d\n", hdr.key_flag & 7);
	printf("    TitleKeyDecIndex:   %d\n", (hdr.key_flag >> 4) & 7);
	printf("  RomSize:              0x%x (%s)\n", hdr.rom_size, getRomSizeStr(hdr.rom_size));
	printf("  CardHeaderVersion:    %d\n", hdr.card_header_version);
	printf("  Flags:                0x%x\n", hdr.flags);
	printf("    AutoBoot:           %s\n", getBoolStr(_HAS_BIT(hdr.flags, XCI_FLAG_AUTOBOOT)));
	printf("    HistoryErase:       %s\n", getBoolStr(_HAS_BIT(hdr.flags, XCI_FLAG_HISTORY_ERASE)));
	printf("  PackageId:            0x%" PRIx64 "\n", hdr.package_id.get());
	printf("  ValidDataEndPage:     0x%x (0x%" PRIx64 ")\n", hdr.valid_data_end_page.get(), blockToAddr(hdr.valid_data_end_page.get()));
	aes_iv = (be_uint64_t*)hdr.encryption_iv;
	printf("  AesIv:                %016" PRIX64 "%016" PRIX64"\n", aes_iv[0].get(), aes_iv[1].get());
	printf("  PartitionFs:\n");
	printf("    Offset:             0x%" PRIx64 "\n", hdr.partition_fs_header_address.get());
	printf("    Size:               0x%" PRIx64 "\n", hdr.partition_fs_header_size.get());
	hash = (be_uint64_t*)hdr.partition_fs_header_hash;
	printf("    Hash:               %016" PRIX64 "%016" PRIX64 "%016" PRIX64 "%016" PRIX64"\n", hash[0].get(),hash[1].get(),hash[2].get(),hash[3].get());
	printf("  InitialData:\n");
	hash = (be_uint64_t*)hdr.initial_data_hash;
	printf("    Hash:               %016" PRIX64 "%016" PRIX64 "%016" PRIX64 "%016" PRIX64"\n", hash[0].get(),hash[1].get(),hash[2].get(),hash[3].get());
	printf("  SelSec:               0x%x\n", hdr.sel_sec.get());
	printf("  SelT1Key:             0x%x\n", hdr.sel_t1_key.get());
	printf("  SelKey:               0x%x\n", hdr.sel_key.get());
	printf("  LimArea:              0x%x\n", hdr.lim_area.get());
	
	if (is_decrypted == true)
	{
		printf("  FwVersion:            v%d.%d\n", hdr.fw_version[1].get(), hdr.fw_version[0].get());
		printf("  AccCtrl1:             0x%x\n", hdr.acc_ctrl_1.get());
		printf("    CardClockRate:      %s\n", getCardClockRate(hdr.acc_ctrl_1.get()));
		printf("  Wait1TimeRead:        0x%x\n", hdr.wait_1_time_read.get());
		printf("  Wait2TimeRead:        0x%x\n", hdr.wait_2_time_read.get());
		printf("  Wait1TimeWrite:       0x%x\n", hdr.wait_1_time_write.get());
		printf("  Wait2TimeWrite:       0x%x\n", hdr.wait_2_time_write.get());
		printf("  FwMode:               0x%x\n", hdr.fw_mode.get());
		printf("  CupVersion:           %d\n", hdr.cup_version.get());
		hash = (be_uint64_t*)hdr.upp_hash;
		printf("  UppHash:              %016" PRIX64 "\n", hash[0].get());
		printf("  CupId:                %016" PRIx64 "\n", hdr.cup_id.get());

	}
}

void decryptXciHeader(const byte_t* src, byte_t* dst)
{
	const byte_t* src_iv = ((const sXciHeader*)src)->encryption_iv;
	byte_t iv[crypto::aes::kAesBlockSize];

	for (size_t i = 0; i < crypto::aes::kAesBlockSize; i++)
	{
		iv[i] = src_iv[15 - i];
	}

	// copy plain
	memcpy(dst, src, kXciHeaderEncOffset);

	// decrypt encrypted
	crypto::aes::AesCbcDecrypt(src + kXciHeaderEncOffset, kXciHeaderEncSize, crypto::aes::nx::prod::xci_header_key, iv, dst + kXciHeaderEncOffset);
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("usage: %s <xci file>\n", argv[0]);
		return 1;
	}

	
	fnd::MemoryBlob xciFile;
	fnd::io::readFile(argv[1], 0x100, 0x100, xciFile);

	sXciHeader* hdr = (sXciHeader*)xciFile.getBytes();
	decryptXciHeader(xciFile.getBytes(), xciFile.getBytes());
	printXciHeader(*hdr, true);

	return 0;
}