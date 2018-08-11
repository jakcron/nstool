#include <fnd/SimpleTextOutput.h>
#include "OffsetAdjustedIFile.h"
#include "CnmtProcess.h"

CnmtProcess::CnmtProcess() :
	mFile(nullptr),
	mOwnIFile(false),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
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
	fnd::Vec<byte_t> scratch;

	if (mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	scratch.alloc(mFile->size());
	mFile->read(scratch.data(), 0, scratch.size());

	mCnmt.fromBytes(scratch.data(), scratch.size());

	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayCmnt();
}

void CnmtProcess::setInputFile(fnd::IFile* file, bool ownIFile)
{
	mFile = file;
	mOwnIFile = ownIFile;
}

void CnmtProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void CnmtProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

const nn::hac::ContentMetaBinary& CnmtProcess::getContentMetaBinary() const
{
	return mCnmt;
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
	printf("    IncludesExFatDriver: %s\n", getBoolStr(_HAS_BIT(mCnmt.getAttributes(), nn::hac::cnmt::ATTRIBUTE_INCLUDES_EX_FAT_DRIVER)));
	printf("    Rebootless:          %s\n", getBoolStr(_HAS_BIT(mCnmt.getAttributes(), nn::hac::cnmt::ATTRIBUTE_REBOOTLESS)));
	printf("  RequiredDownloadSystemVersion: v%" PRId32 " (%d.%d.%d.%d)\n", (uint32_t)mCnmt.getRequiredDownloadSystemVersion(), _SPLIT_VER(mCnmt.getRequiredDownloadSystemVersion()));
	switch(mCnmt.getType())
	{
		case (nn::hac::cnmt::METATYPE_APPLICATION):
			printf("  ApplicationExtendedHeader:\n");
			printf("    RequiredSystemVersion: v%" PRId32 " (%d.%d.%d.%d)\n", (uint32_t)mCnmt.getApplicationMetaExtendedHeader().required_system_version, _SPLIT_VER(mCnmt.getApplicationMetaExtendedHeader().required_system_version));
			printf("    PatchId:               0x%016" PRIx64 "\n", (uint64_t)mCnmt.getApplicationMetaExtendedHeader().patch_id);
			break;
		case (nn::hac::cnmt::METATYPE_PATCH):
			printf("  PatchMetaExtendedHeader:\n");
			printf("    RequiredSystemVersion: v%" PRId32 " (%d.%d.%d.%d))\n", (uint32_t)mCnmt.getPatchMetaExtendedHeader().required_system_version, _SPLIT_VER(mCnmt.getPatchMetaExtendedHeader().required_system_version));
			printf("    ApplicationId:         0x%016" PRIx64 "\n", (uint64_t)mCnmt.getPatchMetaExtendedHeader().application_id);
			break;
		case (nn::hac::cnmt::METATYPE_ADD_ON_CONTENT):
			printf("  AddOnContentMetaExtendedHeader:\n");
			printf("    RequiredSystemVersion: v%" PRId32 " (%d.%d.%d.%d)\n", (uint32_t)mCnmt.getAddOnContentMetaExtendedHeader().required_system_version, _SPLIT_VER(mCnmt.getAddOnContentMetaExtendedHeader().required_system_version));
			printf("    ApplicationId:         0x%016" PRIx64 "\n", (uint64_t)mCnmt.getAddOnContentMetaExtendedHeader().application_id);
			break;
		case (nn::hac::cnmt::METATYPE_DELTA):
			printf("  DeltaMetaExtendedHeader:\n");
			printf("    ApplicationId:         0x%016" PRIx64 "\n", (uint64_t)mCnmt.getDeltaMetaExtendedHeader().application_id);
			break;
		default:
			break;
	}
	if (mCnmt.getContentInfo().size() > 0)
	{
		printf("  ContentInfo:\n");
		for (size_t i = 0; i < mCnmt.getContentInfo().size(); i++)
		{
			const nn::hac::ContentMetaBinary::ContentInfo& info = mCnmt.getContentInfo()[i];
			printf("    %d\n", (int)i);
			printf("      Type:         %s (%d)\n", getContentTypeStr(info.type), info.type);
			printf("      Id:           ");
			_HEXDUMP_L(info.nca_id, nn::hac::cnmt::kContentIdLen);
			printf("\n");
			printf("      Size:         0x%" PRIx64 "\n", (uint64_t)info.size);
			printf("      Hash:         ");
			_HEXDUMP_L(info.hash.bytes, sizeof(info.hash));
			printf("\n");
		}
	}
	if (mCnmt.getContentMetaInfo().size() > 0)
	{
		printf("  ContentMetaInfo:\n");
		for (size_t i = 0; i < mCnmt.getContentMetaInfo().size(); i++)
		{
			const nn::hac::ContentMetaBinary::ContentMetaInfo& info = mCnmt.getContentMetaInfo()[i];
			printf("    %d\n", (int)i);
			printf("      Id:           0x%016" PRIx64 "\n", (uint64_t)info.id);
			printf("      Version:      v%" PRId32 " (%d.%d.%d.%d)\n", (uint32_t)info.version, _SPLIT_VER(info.version));
			printf("      Type:         %s (%d)\n", getContentMetaTypeStr(info.type), info.type);
			printf("      Attributes:   %x\n", mCnmt.getAttributes());
			printf("        IncludesExFatDriver: %s\n", getBoolStr(_HAS_BIT(mCnmt.getAttributes(), nn::hac::cnmt::ATTRIBUTE_INCLUDES_EX_FAT_DRIVER)));
			printf("        Rebootless:          %s\n", getBoolStr(_HAS_BIT(mCnmt.getAttributes(), nn::hac::cnmt::ATTRIBUTE_REBOOTLESS)));
		}
	}
	printf("  Digest:   ");
	_HEXDUMP_L(mCnmt.getDigest().data, nn::hac::cnmt::kDigestLen);
	printf("\n");

#undef _HEXDUMP_L
#undef _HEXDUMP_U
#undef _SPLIT_VER
}

const char* CnmtProcess::getBoolStr(bool state) const
{
	return state? "TRUE" : "FALSE";
}

const char* CnmtProcess::getContentTypeStr(byte_t type) const
{
	const char* str = nullptr;

	switch (type)
	{
	case (nn::hac::cnmt::TYPE_META):
		str = "Meta";
		break;
	case (nn::hac::cnmt::TYPE_PROGRAM):
		str = "Program";
		break;
	case (nn::hac::cnmt::TYPE_DATA):
		str = "Data";
		break;
	case (nn::hac::cnmt::TYPE_CONTROL):
		str = "Control";
		break;
	case (nn::hac::cnmt::TYPE_HTML_DOCUMENT):
		str = "HtmlDocument";
		break;
	case (nn::hac::cnmt::TYPE_LEGAL_INFORMATION):
		str = "LegalInformation";
		break;
	case (nn::hac::cnmt::TYPE_DELTA_FRAGMENT):
		str = "DeltaFragment";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

const char* CnmtProcess::getContentMetaTypeStr(byte_t type) const
{
	const char* str = nullptr;

	switch (type)
	{
	case (nn::hac::cnmt::METATYPE_SYSTEM_PROGRAM):
		str = "SystemProgram";
		break;
	case (nn::hac::cnmt::METATYPE_SYSTEM_DATA):
		str = "SystemData";
		break;
	case (nn::hac::cnmt::METATYPE_SYSTEM_UPDATE):
		str = "SystemUpdate";
		break;
	case (nn::hac::cnmt::METATYPE_BOOT_IMAGE_PACKAGE):
		str = "BootImagePackage";
		break;
	case (nn::hac::cnmt::METATYPE_BOOT_IMAGE_PACKAGE_SAFE):
		str = "BootImagePackageSafe";
		break;
	case (nn::hac::cnmt::METATYPE_APPLICATION):
		str = "Application";
		break;
	case (nn::hac::cnmt::METATYPE_PATCH):
		str = "Patch";
		break;
	case (nn::hac::cnmt::METATYPE_ADD_ON_CONTENT):
		str = "AddOnContent";
		break;
	case (nn::hac::cnmt::METATYPE_DELTA):
		str = "Delta";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

const char* CnmtProcess::getUpdateTypeStr(byte_t type) const
{
	const char* str = nullptr;

	switch (type)
	{
	case (nn::hac::cnmt::UPDATETYPE_APPLY_AS_DELTA):
		str = "ApplyAsDelta";
		break;
	case (nn::hac::cnmt::UPDATETYPE_OVERWRITE):
		str = "Overwrite";
		break;
	case (nn::hac::cnmt::UPDATETYPE_CREATE):
		str = "Create";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

const char* CnmtProcess::getContentMetaAttrStr(byte_t type) const
{
	const char* str = nullptr;

	switch (type)
	{
	case (nn::hac::cnmt::ATTRIBUTE_INCLUDES_EX_FAT_DRIVER):
		str = "IncludesExFatDriver";
		break;
	case (nn::hac::cnmt::ATTRIBUTE_REBOOTLESS):
		str = "Rebootless";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}
