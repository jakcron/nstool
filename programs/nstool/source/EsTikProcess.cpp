#include <fnd/SimpleTextOutput.h>
#include "OffsetAdjustedIFile.h"
#include "EsTikProcess.h"

EsTikProcess::EsTikProcess() :
	mFile(nullptr),
	mOwnIFile(false),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false)
{
}

EsTikProcess::~EsTikProcess()
{
	if (mOwnIFile)
	{
		delete mFile;
	}
}

void EsTikProcess::process()
{
	fnd::Vec<byte_t> scratch;

	if (mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	scratch.alloc(mFile->size());
	mFile->read(scratch.data(), 0, scratch.size());

	mTik.fromBytes(scratch.data(), scratch.size());

	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayTicket();
}

void EsTikProcess::setInputFile(fnd::IFile* file, bool ownIFile)
{
	mFile = file;
	mOwnIFile = ownIFile;
}

void EsTikProcess::setKeyset(const sKeyset* keyset)
{
	mKeyset = keyset;
}

void EsTikProcess::setCliOutputMode(CliOutputMode mode)
{
	mCliOutputMode = mode;
}

void EsTikProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void EsTikProcess::displayTicket()
{
#define _SPLIT_VER(ver) ( (ver>>10) & 0x3f), ( (ver>>4) & 0x3f), ( (ver>>0) & 0xf)
#define _HEXDUMP_U(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02X", var[a__a__A]); } while(0)
#define _HEXDUMP_L(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02x", var[a__a__A]); } while(0)

	const es::TicketBody_V2& body = mTik.getBody();	

	printf("[ES Ticket]\n");
	printf("  SignType:       0x%" PRIx32 " (%s)\n", mTik.getSignature().getSignType(), mTik.getSignature().isLittleEndian()? "LittleEndian" : "BigEndian");
	printf("  Issuer:         %s\n", body.getIssuer().c_str());
	printf("  Title Key:\n");
	printf("    EncMode:      %s\n", getTitleKeyPersonalisationStr(body.getTitleKeyEncType()));
	printf("    CommonKeyId:  %02X\n", body.getCommonKeyId());
	printf("    EncData:\n");
	size_t size = body.getTitleKeyEncType() == es::ticket::RSA2048 ? crypto::rsa::kRsa2048Size : crypto::aes::kAes128KeySize;
	fnd::SimpleTextOutput::hexDump(body.getEncTitleKey(), size, 0x10, 6);

	/*
	if (body.getTitleKeyEncType() == es::ticket::AES128_CBC && body.getCommonKeyId() == 0)
	{
		byte_t iv[crypto::aes::kAesBlockSize];
		byte_t key[crypto::aes::kAes128KeySize];
		memcpy(iv, body.getRightsId(), crypto::aes::kAesBlockSize);
		crypto::aes::AesCbcDecrypt(body.getEncTitleKey(), crypto::aes::kAes128KeySize, eticket_common_key, iv, key);
		size = crypto::aes::kAes128KeySize;
		printf("    TitleKey:\n");
		fnd::SimpleTextOutput::hexDump(key, size, 0x10, 6);
	}
	*/
	printf("  Version:        v%d.%d.%d (%d)\n", _SPLIT_VER(body.getTicketVersion()), body.getTicketVersion());
	printf("  License Type:   %s\n", getLicenseTypeStr(body.getLicenseType()));
	
	if (body.getPropertyFlags().size() > 0)
	{
		printf("  Flags:\n");
		for (size_t i = 0; i < body.getPropertyFlags().size(); i++)
		{
			printf("    %s\n", getPropertyFlagStr(body.getPropertyFlags()[i]));
		}
	}
	
	printf("  Reserved Region:\n");
	fnd::SimpleTextOutput::hexDump(body.getReservedRegion(), 8, 0x10, 4);
	printf("  TicketId:       0x%016" PRIx64 "\n", body.getTicketId());
	printf("  DeviceId:       0x%016" PRIx64 "\n", body.getDeviceId());
	printf("  RightsId:       ");
	fnd::SimpleTextOutput::hexDump(body.getRightsId(), 16);

	printf("  SectionTotalSize:     0x%x\n", body.getSectionTotalSize());
	printf("  SectionHeaderOffset:  0x%x\n", body.getSectionHeaderOffset());
	printf("  SectionNum:           0x%x\n", body.getSectionNum());
	printf("  SectionEntrySize:     0x%x\n", body.getSectionEntrySize());

	
#undef _HEXDUMP_L
#undef _HEXDUMP_U
#undef _SPLIT_VER
}

const char* EsTikProcess::getTitleKeyPersonalisationStr(byte_t flag) const
{
	const char* str = nullptr;
	switch(flag)
	{
	case (es::ticket::AES128_CBC):
		str = "Generic (AESCBC)";
		break;
	case (es::ticket::RSA2048):
		str = "Personalised (RSA2048)";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}

const char* EsTikProcess::getLicenseTypeStr(byte_t flag) const
{
	const char* str = nullptr;
	switch(flag)
	{
	case (es::ticket::LICENSE_PERMANENT):
		str = "Permanent";
		break;
	case (es::ticket::LICENSE_DEMO):
		str = "Demo";
		break;
	case (es::ticket::LICENSE_TRIAL):
		str = "Trial";
		break;
	case (es::ticket::LICENSE_RENTAL):
		str = "Rental";
		break;
	case (es::ticket::LICENSE_SUBSCRIPTION):
		str = "Subscription";
		break;
	case (es::ticket::LICENSE_SERVICE):
		str = "Service";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}

const char* EsTikProcess::getPropertyFlagStr(byte_t flag) const
{
	const char* str = nullptr;
	switch(flag)
	{
	case (es::ticket::FLAG_PRE_INSTALL):
		str = "PreInstall";
		break;
	case (es::ticket::FLAG_SHARED_TITLE):
		str = "SharedTitle";
		break;
	case (es::ticket::FLAG_ALLOW_ALL_CONTENT):
		str = "AllContent";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}