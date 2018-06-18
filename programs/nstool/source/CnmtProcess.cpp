#include <fnd/SimpleTextOutput.h>
#include "OffsetAdjustedIFile.h"
#include "CnmtProcess.h"

const std::string kContentTypeStr[7] =
{
	"Meta",
	"Program",
	"Data",
	"Control",
	"HtmlDocument",
	"LegalInformation",
	"DeltaFragment"
};

const std::string kContentMetaTypeStr[2][0x80] =
{
	{
		"",
		"SystemProgram",
		"SystemData",
		"SystemUpdate",
		"BootImagePackage",
		"BootImagePackageSafe"
	},
	{
		"Application",
		"Patch",
		"AddOnContent",
		"Delta"
	}
};

const std::string kUpdateTypeStr[3] =
{
	"ApplyAsDelta",
	"Overwrite",
	"Create"
};

const std::string kContentMetaAttrStr[3] =
{
	"IncludesExFatDriver",
	"Rebootless"
};


std::string kUnknownStr = "Unknown";

inline const char* getBoolStr(bool isTrue)
{
	return isTrue? "TRUE" : "FALSE";
}

inline const char* getContentTypeStr(byte_t i)
{
	return i < 7 ? kContentTypeStr[i].c_str() : kUnknownStr.c_str();
}

inline const char* getContentMetaTypeStr(byte_t i)
{
	return (i < 0x80) ? kContentMetaTypeStr[0][i].c_str() : kContentMetaTypeStr[1][i - 0x80].c_str();
}

void CnmtProcess::displayCmnt()
{
#define _SPLIT_VER(ver) ( (ver>>26) & 0x3f), ( (ver>>20) & 0x3f), ( (ver>>16) & 0xf), (ver & 0xffff)
#define _HEXDUMP_U(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02X", var[a__a__A]); } while(0)
#define _HEXDUMP_L(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02x", var[a__a__A]); } while(0)

	printf("[ContentMeta]\n");
	printf("  TitleId:               0x%016" PRIx64 "\n", (uint64_t)mCnmt.getTitleId());
	printf("  Version:               v%" PRId32 " (%d.%d.%d.%d)\n", (uint32_t)mCnmt.getTitleVersion(), _SPLIT_VER(mCnmt.getTitleVersion()));
	printf("  Type:                  %s (%d)\n", getContentMetaTypeStr(mCnmt.getType()), mCnmt.getType());
	printf("  Attributes:            %x\n", mCnmt.getAttributes());
	printf("    IncludesExFatDriver: %s\n", getBoolStr(_HAS_BIT(mCnmt.getAttributes(), nx::cnmt::ATTRIBUTE_INCLUDES_EX_FAT_DRIVER)));
	printf("    Rebootless:          %s\n", getBoolStr(_HAS_BIT(mCnmt.getAttributes(), nx::cnmt::ATTRIBUTE_REBOOTLESS)));
	printf("  RequiredDownloadSystemVersion: v%" PRId32 " (%d.%d.%d.%d)\n", (uint32_t)mCnmt.getRequiredDownloadSystemVersion(), _SPLIT_VER(mCnmt.getRequiredDownloadSystemVersion()));
	switch(mCnmt.getType())
	{
		case (nx::cnmt::METATYPE_APPLICATION):
			printf("  ApplicationExtendedHeader:\n");
			printf("    RequiredSystemVersion: v%" PRId32 " (%d.%d.%d.%d)\n", (uint32_t)mCnmt.getApplicationMetaExtendedHeader().required_system_version, _SPLIT_VER(mCnmt.getApplicationMetaExtendedHeader().required_system_version));
			printf("    PatchId:               0x%016" PRIx64 "\n", (uint64_t)mCnmt.getApplicationMetaExtendedHeader().patch_id);
			break;
		case (nx::cnmt::METATYPE_PATCH):
			printf("  PatchMetaExtendedHeader:\n");
			printf("    RequiredSystemVersion: v%" PRId32 " (%d.%d.%d.%d))\n", (uint32_t)mCnmt.getPatchMetaExtendedHeader().required_system_version, _SPLIT_VER(mCnmt.getPatchMetaExtendedHeader().required_system_version));
			printf("    ApplicationId:         0x%016" PRIx64 "\n", (uint64_t)mCnmt.getPatchMetaExtendedHeader().application_id);
			break;
		case (nx::cnmt::METATYPE_ADD_ON_CONTENT):
			printf("  AddOnContentMetaExtendedHeader:\n");
			printf("    RequiredSystemVersion: v%" PRId32 " (%d.%d.%d.%d)\n", (uint32_t)mCnmt.getAddOnContentMetaExtendedHeader().required_system_version, _SPLIT_VER(mCnmt.getAddOnContentMetaExtendedHeader().required_system_version));
			printf("    ApplicationId:         0x%016" PRIx64 "\n", (uint64_t)mCnmt.getAddOnContentMetaExtendedHeader().application_id);
			break;
		case (nx::cnmt::METATYPE_DELTA):
			printf("  DeltaMetaExtendedHeader:\n");
			printf("    ApplicationId:         0x%016" PRIx64 "\n", (uint64_t)mCnmt.getDeltaMetaExtendedHeader().application_id);
			break;
		default:
			break;
	}
	if (mCnmt.getContentInfo().getSize() > 0)
	{
		printf("  ContentInfo:\n");
		for (size_t i = 0; i < mCnmt.getContentInfo().getSize(); i++)
		{
			const nx::ContentMetaBinary::ContentInfo& info = mCnmt.getContentInfo()[i];
			printf("    %d\n", (int)i);
			printf("      Type:         %s (%d)\n", getContentTypeStr(info.type), info.type);
			printf("      Id:           ");
			_HEXDUMP_L(info.nca_id, nx::cnmt::kContentIdLen);
			printf("\n");
			printf("      Size:         0x%" PRIx64 "\n", (uint64_t)info.size);
			printf("      Hash:         ");
			_HEXDUMP_L(info.hash.bytes, sizeof(info.hash));
			printf("\n");
		}
	}
	if (mCnmt.getContentMetaInfo().getSize() > 0)
	{
		printf("  ContentMetaInfo:\n");
		for (size_t i = 0; i < mCnmt.getContentMetaInfo().getSize(); i++)
		{
			const nx::ContentMetaBinary::ContentMetaInfo& info = mCnmt.getContentMetaInfo()[i];
			printf("    %d\n", (int)i);
			printf("      Id:           0x%016" PRIx64 "\n", (uint64_t)info.id);
			printf("      Version:      v%" PRId32 " (%d.%d.%d.%d)\n", (uint32_t)info.version, _SPLIT_VER(info.version));
			printf("      Type:         %s (%d)\n", getContentMetaTypeStr(info.type), info.type);
			printf("      Attributes:   %x\n", mCnmt.getAttributes());
			printf("        IncludesExFatDriver: %s\n", getBoolStr(_HAS_BIT(mCnmt.getAttributes(), nx::cnmt::ATTRIBUTE_INCLUDES_EX_FAT_DRIVER)));
			printf("        Rebootless:          %s\n", getBoolStr(_HAS_BIT(mCnmt.getAttributes(), nx::cnmt::ATTRIBUTE_REBOOTLESS)));
		}
	}
	printf("  Digest:   ");
	_HEXDUMP_L(mCnmt.getDigest().data, nx::cnmt::kDigestLen);
	printf("\n");

#undef _HEXDUMP_L
#undef _HEXDUMP_U
#undef _SPLIT_VER
}

CnmtProcess::CnmtProcess() :
	mFile(nullptr),
	mOwnIFile(false),
	mCliOutputType(OUTPUT_NORMAL),
	mVerify(false)
{
}

CnmtProcess::~CnmtProcess()
{
	if (mOwnIFile)
	{
		delete mFile;
	}
}

void CnmtProcess::process()
{
	fnd::MemoryBlob scratch;

	if (mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	scratch.alloc(mFile->size());
	mFile->read(scratch.getBytes(), 0, scratch.getSize());

	mCnmt.importBinary(scratch.getBytes(), scratch.getSize());

	if (mCliOutputType >= OUTPUT_NORMAL)
	{
		displayCmnt();
	}
}

void CnmtProcess::setInputFile(fnd::IFile* file, bool ownIFile)
{
	mFile = file;
	mOwnIFile = ownIFile;
}

void CnmtProcess::setCliOutputMode(CliOutputType type)
{
	mCliOutputType = type;
}

void CnmtProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

const nx::ContentMetaBinary& CnmtProcess::getContentMetaBinary() const
{
	return mCnmt;
}
