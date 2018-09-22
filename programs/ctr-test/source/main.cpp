/*
(c) 2018 Jakcron
*/
#include <cstdio>

#include <fnd/types.h>
#include <fnd/Vec.h>
#include <fnd/SimpleFile.h>
#include <nn/ctr/CodeBinaryHeader.h>
#include <nn/ctr/NcchHeader.h>

void dump_code_header(const nn::ctr::CodeBinaryHeader& codeHdr)
{
	printf("Code Bin Size:   0x%08x\n", codeHdr.getCodeBinSize());
	printf("Stack Size:      0x%08x\n", codeHdr.getStackSize());
	printf("BSS Size:        0x%08x\n", codeHdr.getBssSize());
	printf("Text Segment:\n");
	printf(" Addr:           0x%08x\n", codeHdr.getTextSegment().getAddress());
	printf(" Size:           0x%08x\n", codeHdr.getTextSegment().getSize());
	printf(" PageNum:        0x%08x\n", codeHdr.getTextSegment().getPageNum());
	printf("RO Data Segment:\n");
	printf(" Addr:           0x%08x\n", codeHdr.getRODataSegment().getAddress());
	printf(" Size:           0x%08x\n", codeHdr.getRODataSegment().getSize());
	printf(" PageNum:        0x%08x\n", codeHdr.getRODataSegment().getPageNum());
	printf("RW Data Segment:\n");
	printf(" Addr:           0x%08x\n", codeHdr.getDataSegment().getAddress());
	printf(" Size:           0x%08x\n", codeHdr.getDataSegment().getSize());
	printf(" PageNum:        0x%08x\n", codeHdr.getDataSegment().getPageNum());
}

const char* dump_ncch_header_ncch_type(uint16_t type)
{
	const char* ret_string = nullptr;

	switch (type)
	{
		case (nn::ctr::ncch::TYPE_CFA):
			ret_string = "CTR File Archive (CFA)";
			break;
		case (nn::ctr::ncch::TYPE_CXI):
			ret_string = "CTR Excutable Image (CXI)";
			break;
		default:
			ret_string = "Unknown";
	}

	return ret_string;
}

const char* dump_ncch_header_crypto_flag(uint8_t type)
{
	const char* ret_string = nullptr;

	switch (type)
	{
		case (nn::ctr::ncch::CRYPTO_DEFAULT):
			ret_string = "[CTR|SNAKE] 0x2C";
			break;
		case (nn::ctr::ncch::CRYPTO_25):
			ret_string = "[CTR|SNAKE] 0x2C/0x25";
			break;
		case (nn::ctr::ncch::CRYPTO_18):
			ret_string = "[SNAKE] 0x2C/0x18";
			break;
		case (nn::ctr::ncch::CRYPTO_1B):
			ret_string = "[SNAKE] 0x2C/0x1B";
			break;
		default:
			ret_string = "Unknown";
	}

	return ret_string;
}

const char* dump_ncch_header_platform_type(uint8_t type)
{
	const char* ret_string = nullptr;

	switch (type)
	{
		case (nn::ctr::ncch::PLATFORM_CTR):
			ret_string = "CTR";
			break;
		case (nn::ctr::ncch::PLATFORM_SNAKE):
			ret_string = "SNAKE";
			break;
		default:
			ret_string = "Unknown";
	}

	return ret_string;
}

const char* dump_ncch_header_form_type(uint8_t type)
{
	const char* ret_string = nullptr;

	switch (type)
	{
		case (nn::ctr::ncch::SIMPLE_CONTENT):
			ret_string = "Simple Content";
			break;
		case (nn::ctr::ncch::EXECUTABLE_WITHOUT_ROMFS):
			ret_string = "Executable Without RomFS";
			break;
		case (nn::ctr::ncch::EXECUTABLE):
			ret_string = "Executable";
			break;
		default:
			ret_string = "Unknown";
	}

	return ret_string;
}

const char* dump_ncch_header_content_type(uint8_t type)
{
	const char* ret_string = nullptr;

	switch (type)
	{
		case (nn::ctr::ncch::APPLICATION):
			ret_string = "Application";
			break;
		case (nn::ctr::ncch::SYSTEM_UPDATE):
			ret_string = "System Update";
			break;
		case (nn::ctr::ncch::MANUAL):
			ret_string = "Manual";
			break;
		case (nn::ctr::ncch::CHILD):
			ret_string = "Child";
			break;
		case (nn::ctr::ncch::TRIAL):
			ret_string = "Trial";
			break;
		case (nn::ctr::ncch::EXTENDED_SYSTEM_UPDATE):
			ret_string = "Extended System Update";
			break;
		default:
			ret_string = "Unknown";
	}

	return ret_string;
}

const char* dump_ncch_header_other_flag_bit(uint8_t bit)
{
	const char* ret_string = nullptr;

	switch (_BIT(bit))
	{
		case (nn::ctr::ncch::FIXED_AES_KEY):
			ret_string = "FIXED_AES_KEY";
			break;
		case (nn::ctr::ncch::NO_MOUNT_ROMFS):
			ret_string = "NO_MOUNT_ROMFS";
			break;
		case (nn::ctr::ncch::NO_AES):
			ret_string = "NO_AES";
			break;
		case (nn::ctr::ncch::SEED_KEY):
			ret_string = "SEED_KEY";
			break;
		case (nn::ctr::ncch::MANUAL_DISCLOSURE):
			ret_string = "MANUAL_DISCLOSURE";
			break;
		default:
			ret_string = nullptr;
	}

	return ret_string;
}

void dump_ncch_header(const nn::ctr::NcchHeader& ncchHdr)
{
	printf("NcchSize:        0x%08x\n", ncchHdr.getNcchBinarySize());
	printf("TitleID:         %04x-%04x-%06x-%02x\n", ncchHdr.getTitleId().getDeviceGroup(), ncchHdr.getTitleId().getCategory(), ncchHdr.getTitleId().getUniqueId(), ncchHdr.getTitleId().getVariation());
	printf("CompanyCode:     %s\n", ncchHdr.getCompanyCode().c_str());
	printf("NcchType:        %04x (%s)\n", ncchHdr.getNcchType(), dump_ncch_header_ncch_type(ncchHdr.getNcchType()));
	printf("SeedChecksum:    %08x\n", ncchHdr.getSeedChecksum());
	printf("ProgramID:       %04x-%04x-%06x-%02x\n", ncchHdr.getProgramId().getDeviceGroup(), ncchHdr.getProgramId().getCategory(), ncchHdr.getProgramId().getUniqueId(), ncchHdr.getProgramId().getVariation());
	printf("Product Code:    %s\n", ncchHdr.getProductCode().c_str());
	printf("Flags:\n");
	printf(" NewCryptoFlag:  %02x (%s)\n", ncchHdr.getNewCryptoFlag(), dump_ncch_header_crypto_flag(ncchHdr.getNewCryptoFlag()));
	printf(" Platform:       %02x (%s)\n", ncchHdr.getPlatform(), dump_ncch_header_platform_type(ncchHdr.getPlatform()));
	printf(" FormType:       %02x (%s)\n", ncchHdr.getFormType(), dump_ncch_header_form_type(ncchHdr.getFormType()));
	printf(" ContentType:    %02x (%s)\n", ncchHdr.getContentType(), dump_ncch_header_content_type(ncchHdr.getContentType()));
	printf(" BlockSize:      %02x\n", ncchHdr.getBlockSize());
	
	if (ncchHdr.getOtherFlagList().size() > 0)
		printf(" OtherFlag:\n");
	for (size_t i = 0; i < ncchHdr.getOtherFlagList().size(); i++)
	{
		nn::ctr::ncch::OtherFlag flag = ncchHdr.getOtherFlagList()[i];
		if (dump_ncch_header_other_flag_bit(flag) != nullptr)
		{
			printf("  %s\n", dump_ncch_header_other_flag_bit(flag));
		}
		else
		{
			printf("  [bit%d]", flag);
		}
	}
	printf("Exheader:\n");
	printf(" Size:           0x%08x\n", ncchHdr.getExtendedHeaderLayout().getSize());
	printf(" Hash:           ");
	for (int i = 0; i < 32; i++)
	{
		printf("%02x", ncchHdr.getExtendedHeaderLayout().getHash().bytes[i]);
	}
	printf("\n");
	printf("PlainRegion:\n");
	printf(" Offset:         0x%08x\n", ncchHdr.getPlainRegionLayout().getOffset());
	printf(" Size:           0x%08x\n", ncchHdr.getPlainRegionLayout().getSize());
	printf("Logo:\n");
	printf(" Offset:         0x%08x\n", ncchHdr.getLogoLayout().getOffset());
	printf(" Size:           0x%08x\n", ncchHdr.getLogoLayout().getSize());
	printf(" Hash:           ");
	for (int i = 0; i < 32; i++)
	{
		printf("%02x", ncchHdr.getLogoLayout().getHash().bytes[i]);
	}
	printf("\n");
	printf("Exefs:\n");
	printf(" Offset:         0x%08x\n", ncchHdr.getExefsLayout().getOffset());
	printf(" Size:           0x%08x\n", ncchHdr.getExefsLayout().getSize());
	printf(" ProtSize:       0x%08x\n", ncchHdr.getExefsLayout().getHashedSize());
	printf(" Hash:           ");
	for (int i = 0; i < 32; i++)
	{
		printf("%02x", ncchHdr.getExefsLayout().getHash().bytes[i]);
	}
	printf("\n");
	printf("Romfs:\n");
	printf(" Offset:         0x%08x\n", ncchHdr.getRomfsLayout().getOffset());
	printf(" Size:           0x%08x\n", ncchHdr.getRomfsLayout().getSize());
	printf(" ProtSize:       0x%08x\n", ncchHdr.getRomfsLayout().getHashedSize());
	printf(" Hash:           ");
	for (int i = 0; i < 32; i++)
	{
		printf("%02x", ncchHdr.getRomfsLayout().getHash().bytes[i]);
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	fnd::SimpleFile fileObj;
	fnd::Vec<byte_t> fileData;

	if (argc < 2)
	{
		printf("usage: %s <file>\n", argv[0]);
		return 1;
	}

	// read file
	try
	{
		fileObj.open(argv[1], fnd::SimpleFile::Read);
		fileData.alloc(_MAX(fileObj.size(), 0x1000));
		fileObj.read(fileData.data(), fileData.size());
	}
	catch (const fnd::Exception &e)
	{
		printf("[ERROR] %s\n", e.error());
		return 2;
	}

	// attempt to read file
	try
	{
		nn::ctr::CodeBinaryHeader hdr, test;
		hdr.fromBytes(fileData.data(), fileData.size());
		printf("[NOTICE] .code file detected\n");
		dump_code_header(hdr);

		hdr.toBytes();
		test.fromBytes(hdr.getBytes().data(), hdr.getBytes().size());
		printf("Testing re-serialisation\n");
		dump_code_header(test);
	}
	catch (const fnd::Exception &e)
	{
		printf("[NOTICE] not a .code file (%s)\n", e.what());
	}

	try
	{
		nn::ctr::NcchHeader hdr, test;
		hdr.fromBytes(fileData.data() + 0x100, fileData.size() >= 0x200 ? 0x100 : 0);
		printf("[NOTICE] NCCH file detected\n");
		dump_ncch_header(hdr);

		hdr.toBytes();
		test.fromBytes(hdr.getBytes().data(), hdr.getBytes().size());
		printf("Testing re-serialisation\n");
		dump_ncch_header(test);
	}
	catch (const fnd::Exception &e)
	{
		printf("[NOTICE] not a NCCH file (%s)\n", e.what());
	}

	return 0;
}