#include <cstdio>
#include <vector>
#include <string>
#include <crypto/aes.h>
#include <crypto/rsa.h>
#include <fnd/io.h>
#include <fnd/MemoryBlob.h>
#include <fnd/SimpleTextOutput.h>
#include <nx/NXCrypto.h>
#include <nx/xci.h>
#include <nx/PfsHeader.h>

/*
struct sXciKeyData
{
	crypto::aes::sAes128Key xci_header_encryption_key;
	crypto::aes::sAes128Key initial_data_key;
	crypto::rsa::sRsa2048Key xci_header_signer_key;
	crypto::rsa::sRsa2048Key card_key_area_oeap_key;
};

void getTitleKeyFromInitialData(const byte_t* initialData, crypto::aes::sAes128Key& titleKey)
{
	const sInitialData* data = (const sInitialData*)initialData;
	crypto::aes::sAes128Key ccmKey;
	crypto::aes::AesEcbDecrypt(data->key_source, 16, key_data.initial_data_key.key, ccmKey.key);
	crypto::aes::AesCcmDecrypt(data->title_key_enc, 16, ccmKey.key, data->ccm_nonce, data->ccm_mac, titleKey.key);
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
		case (nx::xci::ROM_SIZE_1GB) :
			str = "1GB";
			break;
		case (nx::xci::ROM_SIZE_2GB) :
			str = "2GB";
			break;
		case (nx::xci::ROM_SIZE_4GB) :
			str = "4GB";
			break;
		case (nx::xci::ROM_SIZE_8GB) :
			str = "8GB";
			break;
		case (nx::xci::ROM_SIZE_16GB) :
			str = "16GB";
			break;
		case (nx::xci::ROM_SIZE_32GB) :
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
		case (nx::xci::CLOCK_RATE_25) :
			str = "20 MHz";
			break;
		case (nx::xci::CLOCK_RATE_50) :
			str = "50 MHz";
			break;
		
	}
	return str;
}

void printXciHeader(const nx::sXciHeader& hdr, bool is_decrypted)
{
	crypto::aes::sAesIvCtr iv;
	for (size_t i = 0; i < sizeof(iv); i++)
	{
		iv.iv[15-i] = hdr.encryption_iv[i];
	}

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
	printf("    AutoBoot:           %s\n", getBoolStr(_HAS_BIT(hdr.flags, nx::xci::FLAG_AUTOBOOT)));
	printf("    HistoryErase:       %s\n", getBoolStr(_HAS_BIT(hdr.flags, nx::xci::FLAG_HISTORY_ERASE)));
	printf("    RepairTool:         %s\n", getBoolStr(_HAS_BIT(hdr.flags, nx::xci::FLAG_REPAIR_TOOL)));
	printf("  PackageId:            0x%" PRIx64 "\n", hdr.package_id.get());
	printf("  ValidDataEndPage:     0x%x (0x%" PRIx64 ")\n", hdr.valid_data_end_page.get(), blockToAddr(hdr.valid_data_end_page.get()));
	printf("  AesIv:                ");
	fnd::SimpleTextOutput::hexDump(iv.iv, sizeof(iv));
	printf("  PartitionFs:\n");
	printf("    Offset:             0x%" PRIx64 "\n", hdr.partition_fs_header_address.get());
	printf("    Size:               0x%" PRIx64 "\n", hdr.partition_fs_header_size.get());
	printf("    Hash:               ");
	fnd::SimpleTextOutput::hexDump(hdr.partition_fs_header_hash, 0x20);
	printf("  InitialData:\n");
	printf("    Hash:               ");
	fnd::SimpleTextOutput::hexDump(hdr.initial_data_hash, 0x20);
	printf("  SelSec:               0x%x\n", hdr.sel_sec.get());
	printf("  SelT1Key:             0x%x\n", hdr.sel_t1_key.get());
	printf("  SelKey:               0x%x\n", hdr.sel_key.get());
	printf("  LimArea:              0x%x\n", hdr.lim_area.get());
	
	if (is_decrypted == true)
	{
		printf("  FwVersion:            v%d.%d\n", hdr.fw_version[nx::xci::FWVER_MAJOR].get(), hdr.fw_version[nx::xci::FWVER_MINOR].get());
		printf("  AccCtrl1:             0x%x\n", hdr.acc_ctrl_1.get());
		printf("    CardClockRate:      %s\n", getCardClockRate(hdr.acc_ctrl_1.get()));
		printf("  Wait1TimeRead:        0x%x\n", hdr.wait_1_time_read.get());
		printf("  Wait2TimeRead:        0x%x\n", hdr.wait_2_time_read.get());
		printf("  Wait1TimeWrite:       0x%x\n", hdr.wait_1_time_write.get());
		printf("  Wait2TimeWrite:       0x%x\n", hdr.wait_2_time_write.get());
		printf("  FwMode:               0x%x\n", hdr.fw_mode.get());
		printf("  UppVersion:           %d\n", hdr.upp_version.get());
		printf("  UppHash:              ");
		fnd::SimpleTextOutput::hexDump(hdr.upp_hash, 8);
		printf("  UppId:                %016" PRIx64 "\n", hdr.upp_id.get());

	}
}

void printXciPartitionFs(const nx::PfsHeader& pfs, const std::string& partition_name)
{
	printf("[PartitionFS]\n");
	printf("  Type:        %s\n", pfs.getFsType() == pfs.TYPE_PFS0 ? "PFS0" : "HFS0");
	
	if (partition_name.empty())
	{
		printf("  FileSystem:  (%d files)\n", pfs.getFileList().getSize());
	}
	else
	{
		printf("  %s/\n", partition_name.c_str());
	}
	for (size_t i = 0; i < pfs.getFileList().getSize(); i++)
	{
		printf("    %s", pfs.getFileList()[i].name.c_str());
		if (pfs.getFsType() == pfs.TYPE_PFS0)
			printf(" (offset=0x%" PRIx64 ", size=0x%" PRIx64 ")\n", pfs.getFileList()[i].offset, pfs.getFileList()[i].size);
		else
			printf(" (offset=0x%" PRIx64 ", size=0x%" PRIx64 ", hash_protected_size=0x%" PRIx64 ")\n", pfs.getFileList()[i].offset, pfs.getFileList()[i].size, pfs.getFileList()[i].hash_protected_size);
		
	}
}

void decryptXciHeader(const byte_t* src, byte_t* dst)
{
	const byte_t* src_iv = ((const nx::sXciHeader*)src)->encryption_iv;
	byte_t iv[crypto::aes::kAesBlockSize];

	for (size_t i = 0; i < crypto::aes::kAesBlockSize; i++)
	{
		iv[i] = src_iv[15 - i];
	}

	// copy plain
	memcpy(dst, src, nx::xci::kHeaderEncOffset);

	// decrypt encrypted data
	crypto::aes::AesCbcDecrypt(src + nx::xci::kHeaderEncOffset, nx::xci::kHeaderEncSize, crypto::aes::nx::prod::xci_header_key, iv, dst + nx::xci::kHeaderEncOffset);
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("usage: %s <xci file>\n", argv[0]);
		return 1;
	}

	
	fnd::MemoryBlob tmp;
	fnd::io::readFile(argv[1], 0x100, sizeof(nx::sXciHeader), tmp);
	decryptXciHeader(tmp.getBytes(), tmp.getBytes());

	nx::sXciHeader hdr;
	memcpy((void*)&hdr, tmp.getBytes(), sizeof(nx::sXciHeader));
	printXciHeader(hdr, true);

	crypto::sha::sSha256Hash testHash;

	// read root PFS
	fnd::io::readFile(argv[1], hdr.partition_fs_header_address.get(), hdr.partition_fs_header_size.get(), tmp);
	crypto::sha::Sha256(tmp.getBytes(), tmp.getSize(), testHash.bytes);
	if (testHash.compare(hdr.partition_fs_header_hash) == false)
	{
		throw fnd::Exception("xcitool", "Bad root partition hash");
	}
	nx::PfsHeader rootPfs;
	rootPfs.importBinary(tmp.getBytes(), tmp.getSize());
	printXciPartitionFs(rootPfs, "xci:");

	// read sub PFS
	for (size_t i = 0; i < rootPfs.getFileList().getSize(); i++)
	{
		fnd::io::readFile(argv[1], hdr.partition_fs_header_address.get() + rootPfs.getFileList()[i].offset, rootPfs.getFileList()[i].hash_protected_size, tmp);
		crypto::sha::Sha256(tmp.getBytes(), tmp.getSize(), testHash.bytes);
		if (testHash.compare(rootPfs.getFileList()[i].hash) == false)
		{
			throw fnd::Exception("xcitool", "Bad partition hash");
		}
		nx::PfsHeader pfs;
		pfs.importBinary(tmp.getBytes(), tmp.getSize());
		printXciPartitionFs(pfs, "xci:/" + rootPfs.getFileList()[i].name);
	}

	return 0;
}