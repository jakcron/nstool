#include "CnmtProcess.h"

#include <pietendo/hac/ContentMetaUtil.h>

nstool::CnmtProcess::CnmtProcess() :
	mModuleName("nstool::CnmtProcess"),
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

const pie::hac::ContentMeta& nstool::CnmtProcess::getContentMeta() const
{
	return mCnmt;
}

void nstool::CnmtProcess::importCnmt()
{
	if (mFile == nullptr)
	{
		throw tc::Exception(mModuleName, "No file reader set.");
	}
	if (mFile->canRead() == false || mFile->canSeek() == false)
	{
		throw tc::NotSupportedException(mModuleName, "Input stream requires read/seek permissions.");
	}
	
	// check if file_size is greater than 20MB, don't import.
	size_t cnmt_file_size = tc::io::IOUtil::castInt64ToSize(mFile->length());
	if (cnmt_file_size > (0x100000 * 20))
	{
		throw tc::Exception(mModuleName, "File too large.");
	}

	// read cnmt
	tc::ByteData scratch = tc::ByteData(cnmt_file_size);
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read(scratch.data(), scratch.size());

	// parse cnmt
	mCnmt.fromBytes(scratch.data(), scratch.size());
}

void nstool::CnmtProcess::displayCnmt()
{
	const pie::hac::sContentMetaHeader* cnmt_hdr = (const pie::hac::sContentMetaHeader*)mCnmt.getBytes().data();
	fmt::print("[ContentMeta]\n");
	fmt::print("  TitleId:               0x{:016x}\n", mCnmt.getTitleId());
	fmt::print("  Version:               {:s} (v{:d})\n", pie::hac::ContentMetaUtil::getVersionAsString(mCnmt.getTitleVersion()), mCnmt.getTitleVersion());
	fmt::print("  Type:                  {:s} ({:d})\n", pie::hac::ContentMetaUtil::getContentMetaTypeAsString(mCnmt.getContentMetaType()), (uint32_t)mCnmt.getContentMetaType());
	fmt::print("  Attributes:            0x{:x}", *((byte_t*)&cnmt_hdr->attributes));
	if (mCnmt.getAttribute().size())
	{
		std::vector<std::string> attribute_list;

		for (auto itr = mCnmt.getAttribute().begin(); itr != mCnmt.getAttribute().end(); itr++)
		{
			attribute_list.push_back(pie::hac::ContentMetaUtil::getContentMetaAttributeFlagAsString(pie::hac::cnmt::ContentMetaAttributeFlag(*itr)));
		}

		fmt::print(" [");
		for (auto itr = attribute_list.begin(); itr != attribute_list.end(); itr++)
		{
			fmt::print("{:s}",*itr);
			if ((itr + 1) != attribute_list.end())
			{
				fmt::print(", ");
			}
		}
		fmt::print("]");
	}
	fmt::print("\n");

	fmt::print("  StorageId:             {:s} ({:d})\n", pie::hac::ContentMetaUtil::getStorageIdAsString(mCnmt.getStorageId()), (uint32_t)mCnmt.getStorageId());
	fmt::print("  ContentInstallType:    {:s} ({:d})\n", pie::hac::ContentMetaUtil::getContentInstallTypeAsString(mCnmt.getContentInstallType()),(uint32_t)mCnmt.getContentInstallType());
	fmt::print("  RequiredDownloadSystemVersion: {:s} (v{:d})\n", pie::hac::ContentMetaUtil::getVersionAsString(mCnmt.getRequiredDownloadSystemVersion()), mCnmt.getRequiredDownloadSystemVersion());
	switch(mCnmt.getContentMetaType())
	{
		case (pie::hac::cnmt::ContentMetaType_Application):
			fmt::print("  ApplicationExtendedHeader:\n");
			fmt::print("    RequiredApplicationVersion: {:s} (v{:d})\n", pie::hac::ContentMetaUtil::getVersionAsString(mCnmt.getApplicationMetaExtendedHeader().getRequiredApplicationVersion()), mCnmt.getApplicationMetaExtendedHeader().getRequiredApplicationVersion());
			fmt::print("    RequiredSystemVersion:      {:s} (v{:d})\n", pie::hac::ContentMetaUtil::getVersionAsString(mCnmt.getApplicationMetaExtendedHeader().getRequiredSystemVersion()), mCnmt.getApplicationMetaExtendedHeader().getRequiredSystemVersion());
			fmt::print("    PatchId:                    0x{:016x}\n", mCnmt.getApplicationMetaExtendedHeader().getPatchId());
			break;
		case (pie::hac::cnmt::ContentMetaType_Patch):
			fmt::print("  PatchMetaExtendedHeader:\n");
			fmt::print("    RequiredSystemVersion: {:s} (v{:d})\n", pie::hac::ContentMetaUtil::getVersionAsString(mCnmt.getPatchMetaExtendedHeader().getRequiredSystemVersion()), mCnmt.getPatchMetaExtendedHeader().getRequiredSystemVersion());
			fmt::print("    ApplicationId:         0x{:016x}\n", mCnmt.getPatchMetaExtendedHeader().getApplicationId());
			break;
		case (pie::hac::cnmt::ContentMetaType_AddOnContent):
			fmt::print("  AddOnContentMetaExtendedHeader:\n");
			fmt::print("    RequiredApplicationVersion: {:s} (v{:d})\n", pie::hac::ContentMetaUtil::getVersionAsString(mCnmt.getAddOnContentMetaExtendedHeader().getRequiredApplicationVersion()), mCnmt.getAddOnContentMetaExtendedHeader().getRequiredApplicationVersion());
			fmt::print("    ApplicationId:         0x{:016x}\n", mCnmt.getAddOnContentMetaExtendedHeader().getApplicationId());
			break;
		case (pie::hac::cnmt::ContentMetaType_Delta):
			fmt::print("  DeltaMetaExtendedHeader:\n");
			fmt::print("    ApplicationId:         0x{:016x}\n", mCnmt.getDeltaMetaExtendedHeader().getApplicationId());
			break;
		default:
			break;
	}
	if (mCnmt.getContentInfo().size() > 0)
	{
		fmt::print("  ContentInfo:\n");
		for (size_t i = 0; i < mCnmt.getContentInfo().size(); i++)
		{
			const pie::hac::ContentInfo& info = mCnmt.getContentInfo()[i];
			fmt::print("    {:d}\n", i);
			fmt::print("      Type:         {:s} ({:d})\n", pie::hac::ContentMetaUtil::getContentTypeAsString(info.getContentType()), (uint32_t)info.getContentType());
			fmt::print("      Id:           {:s}\n", tc::cli::FormatUtil::formatBytesAsString(info.getContentId().data(), info.getContentId().size(), false, ""));
			fmt::print("      Size:         0x{:x}\n", info.getContentSize());
			fmt::print("      Hash:         {:s}\n", tc::cli::FormatUtil::formatBytesAsString(info.getContentHash().data(), info.getContentHash().size(), false, ""));
		}
	}
	if (mCnmt.getContentMetaInfo().size() > 0)
	{
		fmt::print("  ContentMetaInfo:\n");
		displayContentMetaInfoList(mCnmt.getContentMetaInfo(), "    ");
	}

	// print extended data
	if (mCnmt.getContentMetaType() == pie::hac::cnmt::ContentMetaType_Patch && mCnmt.getPatchMetaExtendedHeader().getExtendedDataSize() != 0)
	{
		// this is stubbed as the raw output is for development purposes
		//fmt::print("  PatchMetaExtendedData:\n");
		//tc::cli::FormatUtil::formatBytesAsHxdHexString(mCnmt.getPatchMetaExtendedData().data(), mCnmt.getPatchMetaExtendedData().size());
	}
	else if (mCnmt.getContentMetaType() == pie::hac::cnmt::ContentMetaType_Delta && mCnmt.getDeltaMetaExtendedHeader().getExtendedDataSize() != 0)
	{
		// this is stubbed as the raw output is for development purposes
		//fmt::print("  DeltaMetaExtendedData:\n");
		//tc::cli::FormatUtil::formatBytesAsHxdHexString(mCnmt.getDeltaMetaExtendedData().data(), mCnmt.getDeltaMetaExtendedData().size());
	}
	else if (mCnmt.getContentMetaType() == pie::hac::cnmt::ContentMetaType_SystemUpdate && mCnmt.getSystemUpdateMetaExtendedHeader().getExtendedDataSize() != 0)
	{
		fmt::print("  SystemUpdateMetaExtendedData:\n");
		fmt::print("    FormatVersion:         {:d}\n", mCnmt.getSystemUpdateMetaExtendedData().getFormatVersion());
		fmt::print("    FirmwareVariation:\n");
		auto variation_info = mCnmt.getSystemUpdateMetaExtendedData().getFirmwareVariationInfo();
		for (size_t i = 0; i < mCnmt.getSystemUpdateMetaExtendedData().getFirmwareVariationInfo().size(); i++)
		{
			fmt::print("      {:d}\n", i);
			fmt::print("        FirmwareVariationId:  0x{:x}\n", variation_info[i].variation_id);
			if (mCnmt.getSystemUpdateMetaExtendedData().getFormatVersion() == 2)
			{
				fmt::print("        ReferToBase:          {}\n", variation_info[i].meta.empty());
				if (variation_info[i].meta.empty() == false)
				{
					fmt::print("        ContentMeta:\n");
					displayContentMetaInfoList(variation_info[i].meta, "          ");
				}
			}
		}
	}

	fmt::print("  Digest:   {:s}\n", tc::cli::FormatUtil::formatBytesAsString(mCnmt.getDigest().data(), mCnmt.getDigest().size(), false, ""));
}

void nstool::CnmtProcess::displayContentMetaInfo(const pie::hac::ContentMetaInfo& content_meta_info, const std::string& prefix)
{
	const pie::hac::sContentMetaInfo* content_meta_info_raw = (const pie::hac::sContentMetaInfo*)content_meta_info.getBytes().data();
	fmt::print("{:s}Id:           0x{:016x}\n", prefix, content_meta_info.getTitleId());
	fmt::print("{:s}Version:      {:s} (v{:d})\n", prefix, pie::hac::ContentMetaUtil::getVersionAsString(content_meta_info.getTitleVersion()), content_meta_info.getTitleVersion());
	fmt::print("{:s}Type:         {:s} ({:d})\n", prefix, pie::hac::ContentMetaUtil::getContentMetaTypeAsString(content_meta_info.getContentMetaType()), (uint32_t)content_meta_info.getContentMetaType());
	fmt::print("{:s}Attributes:   0x{:x}", prefix, *((byte_t*)&content_meta_info_raw->attributes) );
	if (content_meta_info.getAttribute().size())
	{
		std::vector<std::string> attribute_list;

		for (auto itr = content_meta_info.getAttribute().begin(); itr != content_meta_info.getAttribute().end(); itr++)
		{
			attribute_list.push_back(pie::hac::ContentMetaUtil::getContentMetaAttributeFlagAsString(pie::hac::cnmt::ContentMetaAttributeFlag(*itr)));
		}

		fmt::print(" [");
		for (auto itr = attribute_list.begin(); itr != attribute_list.end(); itr++)
		{
			fmt::print("{:s}",*itr);
			if ((itr + 1) != attribute_list.end())
			{
				fmt::print(", ");
			}
		}
		fmt::print("]");
	}
	fmt::print("\n");
}

void nstool::CnmtProcess::displayContentMetaInfoList(const std::vector<pie::hac::ContentMetaInfo>& content_meta_info_list, const std::string& prefix)
{
	for (size_t i = 0; i < content_meta_info_list.size(); i++)
	{
		const pie::hac::ContentMetaInfo& info = mCnmt.getContentMetaInfo()[i];
		fmt::print("{:s}{:d}\n", i);
		displayContentMetaInfo(info, prefix + "  ");
	}
}