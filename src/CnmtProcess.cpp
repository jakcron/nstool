#include "CnmtProcess.h"

#include <iostream>
#include <iomanip>

#include <fnd/SimpleTextOutput.h>
#include <fnd/OffsetAdjustedIFile.h>

#include <nn/hac/ContentMetaUtil.h>

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

const nn::hac::ContentMeta& CnmtProcess::getContentMeta() const
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
		for (size_t i = 0; i < mCnmt.getContentMetaInfo().size(); i++)
		{
			const nn::hac::ContentMetaInfo& info = mCnmt.getContentMetaInfo()[i];
			std::cout << "    " << std::dec << i << std::endl;
			std::cout << "      Id:           0x" << std::hex << std::setw(16) << std::setfill('0') << info.getTitleId() << std::endl;
			std::cout << "      Version:      " << nn::hac::ContentMetaUtil::getVersionAsString(info.getTitleVersion()) << " (v" << std::dec << info.getTitleVersion() << ")"<< std::endl;
			std::cout << "      Type:         " << nn::hac::ContentMetaUtil::getContentMetaTypeAsString(info.getContentMetaType()) << " (" << std::dec << (uint32_t)info.getContentMetaType() << ")" << std::endl; 
			std::cout << "      Attributes:   0x" << std::hex << info.getAttribute().to_ullong();
			if (info.getAttribute().any())
			{
				std::vector<std::string> attribute_list;

				for (size_t flag = 0; flag < info.getAttribute().size(); flag++)
				{
					
					if (info.getAttribute().test(flag))
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
		}
	}

	std::cout << "  Digest:   " << fnd::SimpleTextOutput::arrayToString(mCnmt.getDigest().data(), mCnmt.getDigest().size(), false, "") << std::endl;
}