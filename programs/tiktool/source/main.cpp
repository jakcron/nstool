#include <cstdio>
#include <inttypes.h>
#include <crypto/aes.h>
#include <fnd/io.h>
#include <fnd/MemoryBlob.h>
#include <fnd/SimpleTextOutput.h>
#include <es/ETicketBody_V2.h>

const std::string kTitleKeyPersonalisation[2] = 
{ 
	"Generic (AESCBC)", 
	"Personalised (AESCBC/RSA2048)"
};

const std::string kLicenseType[6] =
{
	"Permanent",
	"Demo",
	"Trial",
	"Rental",
	"Subscription",
	"Service"
};

const std::string kBooleanStr[2] =
{
	"FALSE",
	"TRUE"
};

const byte_t eticket_common_key[16] = { 0x55, 0xA3, 0xF8, 0x72, 0xBD, 0xC8, 0x0C, 0x55, 0x5A, 0x65, 0x43, 0x81, 0x13, 0x9E, 0x15, 0x3B }; // lol this 3ds dev common key


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("usage: tiktool <file>\n");
		return 1;
	}

	try
	{
		fnd::MemoryBlob file;
		fnd::io::readFile(argv[1], file);

		// import
		es::ETicketBody_V2 body;
		body.importBinary(file.getBytes() + 0x140, file.getSize() - 0x140);
		
		printf("[ETICKET]\n");
		printf("  Issuer:         %s\n", body.getIssuer().c_str());
		printf("  Title Key:\n");
		printf("    EncMode:      %s\n", kTitleKeyPersonalisation[body.getTitleKeyEncType()].c_str());
		printf("    CommonKeyId:  %02X\n", body.getCommonKeyId());
		printf("    EncData:\n");
		size_t size = body.getTitleKeyEncType() == es::ETicketBody_V2::RSA2048 ? crypto::rsa::kRsa2048Size : crypto::aes::kAes128KeySize;
		fnd::SimpleTextOutput::hexDump(body.getEncTitleKey(), size, 0x10, 6);

		if (body.getTitleKeyEncType() == es::ETicketBody_V2::AES128_CBC && body.getCommonKeyId() == 0)
		{
			byte_t iv[crypto::aes::kAesBlockSize];
			byte_t key[crypto::aes::kAes128KeySize];
			memcpy(iv, body.getRightsId(), crypto::aes::kAesBlockSize);
			crypto::aes::AesCbcDecrypt(body.getEncTitleKey(), crypto::aes::kAes128KeySize, eticket_common_key, iv, key);
			size = crypto::aes::kAes128KeySize;
			printf("    TitleKey:\n");
			fnd::SimpleTextOutput::hexDump(key, size, 0x10, 6);
		}
		printf("  Version:        v%d\n", body.getTicketVersion());
		printf("  License Type:   %s\n", kLicenseType[body.getLicenseType()].c_str());
		printf("  Flags:\n");
		printf("    PreInstall:   %s\n", kBooleanStr[body.isPreInstall()].c_str());
		printf("    SharedTitle:  %s\n", kBooleanStr[body.isSharedTitle()].c_str());
		printf("    AllContent:   %s\n", kBooleanStr[body.allowAllContent()].c_str());
		printf("  Reserved Region:\n");
		fnd::SimpleTextOutput::hexDump(body.getReservedRegion(), 8, 0x10, 4);
		printf("  TicketId:       0x%016" PRIx64 "\n", body.getTicketId());
		printf("  DeviceId:       0x%016" PRIx64 "\n", body.getDeviceId());
		printf("  RightsId:       ");
		fnd::SimpleTextOutput::hexDump(body.getRightsId(), 16);

	} catch (const fnd::Exception& e)
	{
		printf("%s\n", e.what());
	}

	return 0;
}