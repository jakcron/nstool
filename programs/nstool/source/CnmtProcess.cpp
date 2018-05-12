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
		""
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

inline const std::string& getContentMetaTypeStr(byte_t index)
{
	return (index < 0x80) ? kContentMetaTypeStr[0][index] : kContentMetaTypeStr[1][index-0x80];
}

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

inline const char* getBoolStr(bool isTrue)
{
	return isTrue? "TRUE" : "FALSE";
}

void CnmtProcess::displayCmnt()
{
#define _SPLIT_VER(ver) ( (ver>>24) & 0xff), ( (ver>>16) & 0xff), ( (ver>>8) & 0xff), (ver & 0xff)

	printf("[ContentMeta]\n");
	printf("  TitleId:               0x%" PRIx64 "\n", mCnmt.getTitleId());
	uint32_t ver = mCnmt.getTitleVersion();
	printf("  Version:               v%d.%d.%d-%d (v%" PRId32 ")\n", _SPLIT_VER(ver), ver);
	printf("  Type:                  %s\n", getContentMetaTypeStr(mCnmt.getType()).c_str());
	printf("  Attributes:            %x\n", mCnmt.getAttributes());
	printf("    IncludesExFatDriver: %s\n", getBoolStr(_HAS_BIT(mCnmt.getAttributes(), nx::cnmt::ATTRIBUTE_INCLUDES_EX_FAT_DRIVER)));
	printf("    Rebootless:          %s\n", getBoolStr(_HAS_BIT(mCnmt.getAttributes(), nx::cnmt::ATTRIBUTE_REBOOTLESS)));
	ver = mCnmt.getRequiredDownloadSystemVersion();
	printf("  RequiredDownloadSystemVersion: v%d.%d.%d-%d (v%" PRId32 ")\n", _SPLIT_VER(ver), ver);
	switch(mCnmt.getType())
	{
		case (nx::cnmt::METATYPE_APPLICATION):
			printf("  ApplicationExtendedHeader:\n");
			printf("    RequiredSystemVersion: %" PRId32 "\n", mCnmt.getApplicationMetaExtendedHeader().required_system_version);
			printf("    PatchId:               0x%016" PRIx64 "\n", mCnmt.getApplicationMetaExtendedHeader().patch_id);
			break;
		case (nx::cnmt::METATYPE_PATCH):
			printf("  PatchMetaExtendedHeader:\n");
			printf("    RequiredSystemVersion: %" PRId32 "\n", mCnmt.getPatchMetaExtendedHeader().required_system_version);
			printf("    ApplicationId:         0x%016" PRIx64 "\n", mCnmt.getPatchMetaExtendedHeader().application_id);
			break;
		case (nx::cnmt::METATYPE_ADD_ON_CONTENT):
			printf("  AddOnContentMetaExtendedHeader:\n");
			printf("    RequiredSystemVersion: %" PRId32 "\n", mCnmt.getAddOnContentMetaExtendedHeader().required_system_version);
			printf("    ApplicationId:         0x%016" PRIx64 "\n", mCnmt.getAddOnContentMetaExtendedHeader().application_id);
			break;
		case (nx::cnmt::METATYPE_DELTA):
			printf("  DeltaMetaExtendedHeader:\n");
			printf("    ApplicationId:         0x%016" PRIx64 "\n", mCnmt.getDeltaMetaExtendedHeader().application_id);
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
			printf("    %d\n", i);
			printf("      Type:         %s\n", kContentTypeStr[info.type].c_str());
			printf("      Id:           ");
			fnd::SimpleTextOutput::hexDump(info.nca_id, nx::cnmt::kContentIdLen);
			printf("      Size:         0x%" PRIx64 "\n", info.size);
			printf("      Hash:         ");
			fnd::SimpleTextOutput::hexDump(info.hash.bytes, sizeof(info.hash));
		}
	}
	if (mCnmt.getContentMetaInfo().getSize() > 0)
	{
		printf("  ContentMetaInfo:\n");
		for (size_t i = 0; i < mCnmt.getContentMetaInfo().getSize(); i++)
		{
			const nx::ContentMetaBinary::ContentMetaInfo& info = mCnmt.getContentMetaInfo()[i];
			printf("    %d\n", i);
			printf("      Id:           0x%" PRIx64 "\n", info.id);
			ver = info.version;
			printf("      Version:      v%d.%d.%d-%d (v%" PRId32 ")\n", _SPLIT_VER(ver), ver);
			printf("      Type:         %s\n", getContentMetaTypeStr(info.type).c_str());
			printf("      Attributes:   %x\n", mCnmt.getAttributes());
			printf("        IncludesExFatDriver: %s\n", getBoolStr(_HAS_BIT(mCnmt.getAttributes(), nx::cnmt::ATTRIBUTE_INCLUDES_EX_FAT_DRIVER)));
			printf("        Rebootless:          %s\n", getBoolStr(_HAS_BIT(mCnmt.getAttributes(), nx::cnmt::ATTRIBUTE_REBOOTLESS)));
		}
	}
	printf("  Digest:   ");
	fnd::SimpleTextOutput::hexDump(mCnmt.getDigest().data, nx::cnmt::kDigestLen);

#undef _SPLIT_VER
}

CnmtProcess::CnmtProcess() :
	mReader(nullptr),
	mCliOutputType(OUTPUT_NORMAL),
	mVerify(false)
{
}

CnmtProcess::~CnmtProcess()
{
	if (mReader != nullptr)
	{
		delete mReader;
	}
}

void CnmtProcess::process()
{
	fnd::MemoryBlob scratch;

	if (mReader == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	scratch.alloc(mReader->size());
	mReader->read(scratch.getBytes(), 0, scratch.getSize());

	mCnmt.importBinary(scratch.getBytes(), scratch.getSize());

	if (mCliOutputType >= OUTPUT_NORMAL)
	{
		displayCmnt();
	}
}

void CnmtProcess::setInputFile(fnd::IFile* file, size_t offset, size_t size)
{
	mReader = new OffsetAdjustedIFile(file, offset, size);
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
