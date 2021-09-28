#include "CnmtProcess.h"

#include <iostream>
#include <iomanip>

#include <nn/hac/ContentMetaUtil.h>

nstool::CnmtProcess::CnmtProcess() :
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false)
{
}

void nstool::CnmtProcess::process()
{
	importCnmt();

	if (mCliOutputMode.show_basic_info)
		displayCnmt();
}

void nstool::CnmtProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::CnmtProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void nstool::CnmtProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

const nn::hac::ContentMeta& nstool::CnmtProcess::getContentMeta() const
{
	return mCnmt;
}

void nstool::CnmtProcess::importCnmt()
{
	tc::ByteData scratch;

	if (*mFile == nullptr)
	{
		throw tc::Exception(kModuleName, "No file reader set.");
	}

	scratch.alloc((*mFile)->size());
	(*mFile)->read(scratch.data(), 0, scratch.size());

	mCnmt.fromBytes(scratch.data(), scratch.size());
}

void nstool::CnmtProcess::displayCnmt()
{
	std::cout << "[ContentMeta]" << std::endl;
	std::cout << "  TitleId:               0x" << std::hex << std::setw(16) << std::setfill('0') << mCnmt.getTitleId() << std::endl;
	std::cout << "  Version:               " << nn::hac::ContentMetaUtil::getVersionAsString(mCnmt.getTitleVersion()) << " (v" << std::dec << mCnmt.getTitleVersion() << ")"<< std::endl;
	std::cout << "  Type:                  " << nn::hac::ContentMetaUtil::getContentMetaTypeAsString(mCnmt.getContentMetaType()) << " (" << std::dec << (uint32_t)mCnmt.getContentMetaType() << ")" << std::endl;
	std::cout << "  Attributes:            0x" << std::hex << mCnmt.getAttribute().to_ullong();
	if (mCnmt.getAttribute().any())
	{
		std::vector<std::string> attribute_list;

		for (size_t flag = 0; flag < mCnmt.getAttribute().size(); flag++)
		{
			
			if (mCnmt.getAttribute().test(flag))
			{
				attribute_list.push_back(nn::hac::ContentMetaUtil::getContentMetaAttributeFlagAsString(nn::hac::cnmt::ContentMetaAttributeFlag(flag)));
			}
		}

		std::cout << " [";
		for (auto itr = attribute_list.begin(); itr != attribute_list.end(); itr++)
		{
			std::cout << *itr;
			if ((itr + 1) != attribute_list.end())
			{
				std::cout << ", ";
			}
		}
		std::cout << "]";
	}
	std::cout << std::endl;

	std::cout << "  StorageId:             " << nn::hac::ContentMetaUtil::getStorageIdAsString(mCnmt.getStorageId()) << " (" << std::dec << (uint32_t)mCnmt.getStorageId() << ")" << std::endl;
	std::cout << "  ContentInstallType:    " << nn::hac::ContentMetaUtil::getContentInstallTypeAsString(mCnmt.getContentInstallType()) << " (" << std::dec << (uint32_t)mCnmt.getContentInstallType() << ")" << std::endl;
	std::cout << "  RequiredDownloadSystemVersion: " << nn::hac::ContentMetaUtil::getVersionAsString(mCnmt.getRequiredDownloadSystemVersion()) << " (v" << mCnmt.getRequiredDownloadSystemVersion() << ")"<< std::endl;
	switch(mCnmt.getContentMetaType())
	{
		case (nn::hac::cnmt::ContentMetaType::Application):
			std::cout << "  ApplicationExtendedHeader:" << std::endl;
			std::cout << "    RequiredApplicationVersion: " << nn::hac::ContentMetaUtil::getVersionAsString(mCnmt.getApplicationMetaExtendedHeader().getRequiredApplicationVersion()) << " (v" << std::dec << mCnmt.getApplicationMetaExtendedHeader().getRequiredApplicationVersion() << ")"<< std::endl;
			std::cout << "    RequiredSystemVersion:      " << nn::hac::ContentMetaUtil::getVersionAsString(mCnmt.getApplicationMetaExtendedHeader().getRequiredSystemVersion()) << " (v" << std::dec << mCnmt.getApplicationMetaExtendedHeader().getRequiredSystemVersion() << ")"<< std::endl;
			std::cout << "    PatchId:                    0x" << std::hex << std::setw(16) << std::setfill('0') << mCnmt.getApplicationMetaExtendedHeader().getPatchId() << std::endl;
			break;
		case (nn::hac::cnmt::ContentMetaType::Patch):
			std::cout << "  PatchMetaExtendedHeader:" << std::endl;
			std::cout << "    RequiredSystemVersion: " << nn::hac::ContentMetaUtil::getVersionAsString(mCnmt.getPatchMetaExtendedHeader().getRequiredSystemVersion()) <<  " (v" << std::dec << mCnmt.getPatchMetaExtendedHeader().getRequiredSystemVersion() << ")"<< std::endl;
			std::cout << "    ApplicationId:         0x" << std::hex << std::setw(16) << std::setfill('0') << mCnmt.getPatchMetaExtendedHeader().getApplicationId() << std::endl;
			break;
		case (nn::hac::cnmt::ContentMetaType::AddOnContent):
			std::cout << "  AddOnContentMetaExtendedHeader:" << std::endl;
			std::cout << "    RequiredApplicationVersion: " << nn::hac::ContentMetaUtil::getVersionAsString(mCnmt.getAddOnContentMetaExtendedHeader().getRequiredApplicationVersion()) << " (v" << std::dec << mCnmt.getAddOnContentMetaExtendedHeader().getRequiredApplicationVersion() << ")" << std::endl;
			std::cout << "    ApplicationId:         0x" << std::hex << std::setw(16) << std::setfill('0') << mCnmt.getAddOnContentMetaExtendedHeader().getApplicationId() << std::endl;
			break;
		case (nn::hac::cnmt::ContentMetaType::Delta):
			std::cout << "  DeltaMetaExtendedHeader:" << std::endl;
			std::cout << "    ApplicationId:         0x" << std::hex << std::setw(16) << std::setfill('0') << mCnmt.getDeltaMetaExtendedHeader().getApplicationId() << std::endl;
			break;
		default:
			break;
	}
	if (mCnmt.getContentInfo().size() > 0)
	{
		printf("  ContentInfo:\n");
		for (size_t i = 0; i < mCnmt.getContentInfo().size(); i++)
		{
			const nn::hac::ContentInfo& info = mCnmt.getContentInfo()[i];
			std::cout << "    " << std::dec << i << std::endl;
			std::cout << "      Type:         " << nn::hac::ContentMetaUtil::getContentTypeAsString(info.getContentType()) << " (" << std::dec << (uint32_t)info.getContentType() << ")" << std::endl;
			std::cout << "      Id:           " << fnd::SimpleTextOutput::arrayToString(info.getContentId().data(), info.getContentId().size(), false, "") << std::endl;
			std::cout << "      Size:         0x" << std::hex << info.getContentSize() << std::endl;
			std::cout << "      Hash:         " << fnd::SimpleTextOutput::arrayToString(info.getContentHash().bytes, sizeof(info.getContentHash()), false, "") << std::endl;
		}
	}
	if (mCnmt.getContentMetaInfo().size() > 0)
	{
		std::cout << "  ContentMetaInfo:" << std::endl;
		displayContentMetaInfoList(mCnmt.getContentMetaInfo(), "    ");
	}

	// print extended data
	if (mCnmt.getContentMetaType() == nn::hac::cnmt::ContentMetaType::Patch && mCnmt.getPatchMetaExtendedHeader().getExtendedDataSize() != 0)
	{
		// this is stubbed as the raw output is for development purposes
		//std::cout << "  PatchMetaExtendedData:" << std::endl;
		//fnd::SimpleTextOutput::hxdStyleDump(mCnmt.getPatchMetaExtendedData().data(), mCnmt.getPatchMetaExtendedData().size());
	}
	else if (mCnmt.getContentMetaType() == nn::hac::cnmt::ContentMetaType::Delta && mCnmt.getDeltaMetaExtendedHeader().getExtendedDataSize() != 0)
	{
		// this is stubbed as the raw output is for development purposes
		//std::cout << "  DeltaMetaExtendedData:" << std::endl;
		//fnd::SimpleTextOutput::hxdStyleDump(mCnmt.getDeltaMetaExtendedData().data(), mCnmt.getDeltaMetaExtendedData().size());
	}
	else if (mCnmt.getContentMetaType() == nn::hac::cnmt::ContentMetaType::SystemUpdate && mCnmt.getSystemUpdateMetaExtendedHeader().getExtendedDataSize() != 0)
	{
		std::cout << "  SystemUpdateMetaExtendedData:" << std::endl;
		std::cout << "    FormatVersion:         " << std::dec << mCnmt.getSystemUpdateMetaExtendedData().getFormatVersion() << std::endl;
		std::cout << "    FirmwareVariation:" << std::endl;
		auto variation_info = mCnmt.getSystemUpdateMetaExtendedData().getFirmwareVariationInfo();
		for (size_t i = 0; i < mCnmt.getSystemUpdateMetaExtendedData().getFirmwareVariationInfo().size(); i++)
		{
			std::cout << "      " << std::dec << i << std::endl;
			std::cout << "        FirmwareVariationId:  0x" << std::hex << variation_info[i].variation_id << std::endl;
			if (mCnmt.getSystemUpdateMetaExtendedData().getFormatVersion() == 2)
			{
				std::cout << "        ReferToBase:          " << std::boolalpha << variation_info[i].meta.empty() << std::endl;
				if (variation_info[i].meta.empty() == false)
				{
					std::cout << "        ContentMeta:" << std::endl;
					displayContentMetaInfoList(variation_info[i].meta, "          ");
				}
			}
		}
	} 

	

	std::cout << "  Digest:   " << fnd::SimpleTextOutput::arrayToString(mCnmt.getDigest().data(), mCnmt.getDigest().size(), false, "") << std::endl;
}

void nstool::CnmtProcess::displayContentMetaInfo(const nn::hac::ContentMetaInfo& content_meta_info, const std::string& prefix)
{
	std::cout << prefix << "Id:           0x" << std::hex << std::setw(16) << std::setfill('0') << content_meta_info.getTitleId() << std::endl;
	std::cout << prefix << "Version:      " << nn::hac::ContentMetaUtil::getVersionAsString(content_meta_info.getTitleVersion()) << " (v" << std::dec << content_meta_info.getTitleVersion() << ")"<< std::endl;
	std::cout << prefix << "Type:         " << nn::hac::ContentMetaUtil::getContentMetaTypeAsString(content_meta_info.getContentMetaType()) << " (" << std::dec << (uint32_t)content_meta_info.getContentMetaType() << ")" << std::endl; 
	std::cout << prefix << "Attributes:   0x" << std::hex << content_meta_info.getAttribute().to_ullong();
	if (content_meta_info.getAttribute().any())
	{
		std::vector<std::string> attribute_list;

		for (size_t flag = 0; flag < content_meta_info.getAttribute().size(); flag++)
		{
			
			if (content_meta_info.getAttribute().test(flag))
			{
				attribute_list.push_back(nn::hac::ContentMetaUtil::getContentMetaAttributeFlagAsString(nn::hac::cnmt::ContentMetaAttributeFlag(flag)));
			}
		}

		std::cout << " [";
		for (auto itr = attribute_list.begin(); itr != attribute_list.end(); itr++)
		{
			std::cout << *itr;
			if ((itr + 1) != attribute_list.end())
			{
				std::cout << ", ";
			}
		}
		std::cout << "]";
	}
}

void nstool::CnmtProcess::displayContentMetaInfoList(const std::vector<nn::hac::ContentMetaInfo>& content_meta_info_list, const std::string& prefix)
{
	for (size_t i = 0; i < content_meta_info_list.size(); i++)
		{
			const nn::hac::ContentMetaInfo& info = mCnmt.getContentMetaInfo()[i];
			std::cout << prefix << std::dec << i << std::endl;
			displayContentMetaInfo(info, prefix + "  ");
			std::cout << std::endl;
		}
}