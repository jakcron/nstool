#include <iostream>
#include <iomanip>
#include <fnd/SimpleTextOutput.h>
#include <fnd/OffsetAdjustedIFile.h>
#include "CnmtProcess.h"

CnmtProcess::CnmtProcess() :
	mFile(),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false)
{
}

void CnmtProcess::process()
{
	importCnmt();

	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayCnmt();
}

void CnmtProcess::setInputFile(const fnd::SharedPtr<fnd::IFile>& file)
{
	mFile = file;
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

void CnmtProcess::importCnmt()
{
	fnd::Vec<byte_t> scratch;

	if (*mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	scratch.alloc((*mFile)->size());
	(*mFile)->read(scratch.data(), 0, scratch.size());

	mCnmt.fromBytes(scratch.data(), scratch.size());
}

void CnmtProcess::displayCnmt()
{
#define _SPLIT_VER(ver) (uint32_t)((ver>>26) & 0x3f) << "." << (uint32_t)((ver>>20) & 0x3f) << "." << (uint32_t)((ver>>16) & 0xf) << "." << (uint32_t)(ver & 0xffff)

	std::cout << "[ContentMeta]" << std::endl;
	std::cout << "  TitleId:               0x" << std::hex << std::setw(16) << std::setfill('0') << mCnmt.getTitleId() << std::endl;
	std::cout << "  Version:               v" << std::dec << mCnmt.getTitleVersion() << " (" << _SPLIT_VER(mCnmt.getTitleVersion()) << ")"<< std::endl;
	std::cout << "  Type:                  " << getContentMetaTypeStr(mCnmt.getType()) << " (" << std::dec << mCnmt.getType() << ")" << std::endl;
	std::cout << "  Attributes:            0x" << std::hex << (uint32_t)mCnmt.getAttributes() << std::endl;
	std::cout << "    IncludesExFatDriver: " << getBoolStr(_HAS_BIT(mCnmt.getAttributes(), nn::hac::cnmt::ATTRIBUTE_INCLUDES_EX_FAT_DRIVER)) << std::endl;
	std::cout << "    Rebootless:          " << getBoolStr(_HAS_BIT(mCnmt.getAttributes(), nn::hac::cnmt::ATTRIBUTE_REBOOTLESS)) << std::endl;
	std::cout << "  RequiredDownloadSystemVersion: v" << mCnmt.getRequiredDownloadSystemVersion() << " (" << _SPLIT_VER(mCnmt.getRequiredDownloadSystemVersion()) << ")"<< std::endl;
	switch(mCnmt.getType())
	{
		case (nn::hac::cnmt::METATYPE_APPLICATION):
			std::cout << "  ApplicationExtendedHeader:" << std::endl;
			std::cout << "    RequiredSystemVersion: v" << std::dec << mCnmt.getApplicationMetaExtendedHeader().required_system_version << " (" << _SPLIT_VER(mCnmt.getApplicationMetaExtendedHeader().required_system_version) << ")"<< std::endl;
			std::cout << "    PatchId:               0x" << std::hex << std::setw(16) << std::setfill('0') << mCnmt.getApplicationMetaExtendedHeader().patch_id << std::endl;
			break;
		case (nn::hac::cnmt::METATYPE_PATCH):
			std::cout << "  PatchMetaExtendedHeader:" << std::endl;
			std::cout << "    RequiredSystemVersion: v" << std::dec << mCnmt.getPatchMetaExtendedHeader().required_system_version << " (" << _SPLIT_VER(mCnmt.getPatchMetaExtendedHeader().required_system_version) << ")"<< std::endl;
			std::cout << "    ApplicationId:         0x" << std::hex << std::setw(16) << std::setfill('0') << mCnmt.getPatchMetaExtendedHeader().application_id << std::endl;
			break;
		case (nn::hac::cnmt::METATYPE_ADD_ON_CONTENT):
			std::cout << "  AddOnContentMetaExtendedHeader:" << std::endl;
			std::cout << "    RequiredApplicationVersion: v" << std::dec << mCnmt.getAddOnContentMetaExtendedHeader().required_application_version << " (" << _SPLIT_VER(mCnmt.getAddOnContentMetaExtendedHeader().required_application_version) << ")" << std::endl;
			std::cout << "    ApplicationId:         0x" << std::hex << std::setw(16) << std::setfill('0') << mCnmt.getAddOnContentMetaExtendedHeader().application_id << std::endl;
			break;
		case (nn::hac::cnmt::METATYPE_DELTA):
			std::cout << "  DeltaMetaExtendedHeader:" << std::endl;
			std::cout << "    ApplicationId:         0x" << std::hex << std::setw(16) << std::setfill('0') << mCnmt.getDeltaMetaExtendedHeader().application_id << std::endl;
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
			std::cout << "    " << std::dec << i << std::endl;
			std::cout << "      Type:         " << getContentTypeStr(info.type) << " (" << std::dec << info.type << ")" << std::endl;
			std::cout << "      Id:           " << fnd::SimpleTextOutput::arrayToString(info.nca_id, nn::hac::cnmt::kContentIdLen, false, "") << std::endl;
			std::cout << "      Size:         0x" << std::hex << info.size << std::endl;
			std::cout << "      Hash:         " << fnd::SimpleTextOutput::arrayToString(info.hash.bytes, sizeof(info.hash), false, "") << std::endl;
		}
	}
	if (mCnmt.getContentMetaInfo().size() > 0)
	{
		std::cout << "  ContentMetaInfo:" << std::endl;
		for (size_t i = 0; i < mCnmt.getContentMetaInfo().size(); i++)
		{
			const nn::hac::ContentMetaBinary::ContentMetaInfo& info = mCnmt.getContentMetaInfo()[i];
			std::cout << "    " << std::dec << i << std::endl;
			std::cout << "      Id:           0x" << std::hex << std::setw(16) << std::setfill('0') << info.id << std::endl;
			std::cout << "      Version:      v" << std::dec << info.version << " (" << _SPLIT_VER(info.version) << ")"<< std::endl;
			std::cout << "      Type:         " << getContentMetaTypeStr(info.type) << " (" << std::dec << info.type << ")" << std::endl; 
			std::cout << "      Attributes:   0x" << std::hex << (uint32_t)info.attributes << std::endl;
			std::cout << "        IncludesExFatDriver: " << getBoolStr(_HAS_BIT(info.attributes, nn::hac::cnmt::ATTRIBUTE_INCLUDES_EX_FAT_DRIVER)) << std::endl;
			std::cout << "        Rebootless:          " << getBoolStr(_HAS_BIT(info.attributes, nn::hac::cnmt::ATTRIBUTE_REBOOTLESS)) << std::endl;
		}
	}

	std::cout << "  Digest:   " << fnd::SimpleTextOutput::arrayToString(mCnmt.getDigest().data, nn::hac::cnmt::kDigestLen, false, "") << std::endl;

#undef _SPLIT_VER
}

const char* CnmtProcess::getBoolStr(bool state) const
{
	return state? "TRUE" : "FALSE";
}

const char* CnmtProcess::getContentTypeStr(nn::hac::cnmt::ContentType type) const
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

const char* CnmtProcess::getContentMetaTypeStr(nn::hac::cnmt::ContentMetaType type) const
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

const char* CnmtProcess::getUpdateTypeStr(nn::hac::cnmt::UpdateType type) const
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

const char* CnmtProcess::getContentMetaAttrStr(nn::hac::cnmt::ContentMetaAttribute attr) const
{
	const char* str = nullptr;

	switch (attr)
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
