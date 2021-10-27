#include "NcaProcess.h"
#include "MetaProcess.h"
#include "util.h"

#include <tc/crypto/detail/BlockUtilImpl.h>

#include <nn/hac/ContentArchiveUtil.h>
#include <nn/hac/AesKeygen.h>
#include <nn/hac/HierarchicalSha256Stream.h>
#include <nn/hac/HierarchicalIntegrityStream.h>
#include <nn/hac/PartitionFsMetaGenerator.h>
#include <nn/hac/RomFsMetaGenerator.h>
#include <nn/hac/CombinedFsMetaGenerator.h>


nstool::NcaProcess::NcaProcess() :
	mModuleName("nstool::NcaProcess"),
	mFile(),
	mCliOutputMode(true, false, false, false),
	mVerify(false),
	mFileSystem(),
	mFsProcess()
{
}

void nstool::NcaProcess::process()
{
	// import header
	importHeader();

	// determine keys
	generateNcaBodyEncryptionKeys();

	// import/generate fs header data
	generatePartitionConfiguration();

	// validate signatures
	if (mVerify)
		validateNcaSignatures();

	// display header
	if (mCliOutputMode.show_basic_info)
		displayHeader();

	// process partition
	processPartitions();
}

void nstool::NcaProcess::setInputFile(const std::shared_ptr<tc::io::IStream>& file)
{
	mFile = file;
}

void nstool::NcaProcess::setKeyCfg(const KeyBag& keycfg)
{
	mKeyCfg = keycfg;
}

void nstool::NcaProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void nstool::NcaProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void nstool::NcaProcess::setShowFsTree(bool show_fs_tree)
{
	mFsProcess.setShowFsTree(show_fs_tree);
}

void nstool::NcaProcess::setFsRootLabel(const std::string& root_label)
{
	mFsProcess.setFsRootLabel(root_label);
}

void nstool::NcaProcess::setExtractJobs(const std::vector<nstool::ExtractJob>& extract_jobs)
{
	mFsProcess.setExtractJobs(extract_jobs);
}

const std::shared_ptr<tc::io::IStorage>& nstool::NcaProcess::getFileSystem() const
{
	return mFileSystem;
}

void nstool::NcaProcess::importHeader()
{
	if (mFile == nullptr)
	{
		throw tc::Exception(mModuleName, "No file reader set.");
	}
	if (mFile->canRead() == false || mFile->canSeek() == false)
	{
		throw tc::NotSupportedException(mModuleName, "Input stream requires read/seek permissions.");
	}

	// read header block
	if (mFile->length() < tc::io::IOUtil::castSizeToInt64(sizeof(nn::hac::sContentArchiveHeaderBlock)))
	{
		throw tc::Exception(mModuleName, "Corrupt NCA: File too small.");
	}
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read((byte_t*)(&mHdrBlock), sizeof(nn::hac::sContentArchiveHeaderBlock));

	// decrypt header block
	if (mKeyCfg.nca_header_key.isNull())
	{
		throw tc::Exception(mModuleName, "Failed to decrypt NCA header. (nca_header_key could not be loaded)");
	}
	nn::hac::ContentArchiveUtil::decryptContentArchiveHeader((byte_t*)&mHdrBlock, (byte_t*)&mHdrBlock, mKeyCfg.nca_header_key.get());

	// generate header hash
	tc::crypto::GenerateSha256Hash(mHdrHash.data(), (byte_t*)&mHdrBlock.header, sizeof(nn::hac::sContentArchiveHeader));

	// proccess main header
	mHdr.fromBytes((byte_t*)&mHdrBlock.header, sizeof(nn::hac::sContentArchiveHeader));
}

void nstool::NcaProcess::generateNcaBodyEncryptionKeys()
{
	// create zeros key
	KeyBag::aes128_key_t zero_aesctr_key;
	memset(zero_aesctr_key.data(), 0, zero_aesctr_key.size());
	
	// get key data from header
	byte_t masterkey_rev = nn::hac::AesKeygen::getMasterKeyRevisionFromKeyGeneration(mHdr.getKeyGeneration());
	byte_t keak_index = mHdr.getKeyAreaEncryptionKeyIndex();

	// process key area
	sKeys::sKeyAreaKey kak;
	for (size_t i = 0; i < mHdr.getKeyArea().size(); i++)
	{
		if (mHdr.getKeyArea()[i] != zero_aesctr_key)
		{
			kak.index = (byte_t)i;
			kak.enc = mHdr.getKeyArea()[i];
			kak.decrypted = false;
			// key[0-3]
			if (i < 4 && mKeyCfg.nca_key_area_encryption_key[keak_index].find(masterkey_rev) != mKeyCfg.nca_key_area_encryption_key[keak_index].end())
			{
				kak.decrypted = true;
				nn::hac::AesKeygen::generateKey(kak.dec.data(), kak.enc.data(), mKeyCfg.nca_key_area_encryption_key[keak_index][masterkey_rev].data());
			}
			// key[KEY_AESCTR_HW]
			else if (i == nn::hac::nca::KEY_AESCTR_HW && mKeyCfg.nca_key_area_encryption_key_hw[keak_index].find(masterkey_rev) != mKeyCfg.nca_key_area_encryption_key_hw[keak_index].end())
			{
				kak.decrypted = true;
				nn::hac::AesKeygen::generateKey(kak.dec.data(), kak.enc.data(), mKeyCfg.nca_key_area_encryption_key_hw[keak_index][masterkey_rev].data());
			}
			else
			{
				kak.decrypted = false;
			}
			mContentKey.kak_list.push_back(kak);
		}
	}

	// clear content key
	mContentKey.aes_ctr = tc::Optional<nn::hac::detail::aes128_key_t>();

	// if this has a rights id, the key needs to be sourced from a ticket
	if (mHdr.hasRightsId() == true)
	{
		KeyBag::aes128_key_t tmp_key;
		if (mKeyCfg.external_content_keys.find(mHdr.getRightsId()) != mKeyCfg.external_content_keys.end())
		{
			mContentKey.aes_ctr = mKeyCfg.external_content_keys[mHdr.getRightsId()];
		}
		else if (mKeyCfg.fallback_content_key.isSet())
		{
			mContentKey.aes_ctr = mKeyCfg.fallback_content_key.get();
		}
		else if (mKeyCfg.fallback_enc_content_key.isSet())
		{
			tmp_key = mKeyCfg.fallback_enc_content_key.get();
			if (mKeyCfg.etik_common_key.find(masterkey_rev) != mKeyCfg.etik_common_key.end())
			{
				nn::hac::AesKeygen::generateKey(tmp_key.data(), tmp_key.data(), mKeyCfg.etik_common_key[masterkey_rev].data());
				mContentKey.aes_ctr = tmp_key;
			}
		}
	}
	// otherwise used decrypt key area
	else
	{
		for (size_t i = 0; i < mContentKey.kak_list.size(); i++)
		{
			if (mContentKey.kak_list[i].index == nn::hac::nca::KEY_AESCTR && mContentKey.kak_list[i].decrypted)
			{
				mContentKey.aes_ctr = mContentKey.kak_list[i].dec;
			}
		}
	}

	// if the keys weren't generated, check if the keys were supplied by the user
	if (mContentKey.aes_ctr.isNull())
	{
		if (mKeyCfg.fallback_content_key.isSet())
		{
			mContentKey.aes_ctr = mKeyCfg.fallback_content_key.get();
		}
	}
	
	if (mCliOutputMode.show_keydata)
	{
		if (mContentKey.aes_ctr.isSet())
		{
			fmt::print("[NCA Content Key]\n");
			fmt::print("  AES-CTR Key: {:s}\n", tc::cli::FormatUtil::formatBytesAsString(mContentKey.aes_ctr.get().data(), mContentKey.aes_ctr.get().size(), true, ":"));
		}
	}
}

void nstool::NcaProcess::generatePartitionConfiguration()
{
	for (size_t i = 0; i < mHdr.getPartitionEntryList().size(); i++)
	{
		// get reference to relevant structures
		const nn::hac::ContentArchiveHeader::sPartitionEntry& partition = mHdr.getPartitionEntryList()[i];
		nn::hac::sContentArchiveFsHeader& fs_header = mHdrBlock.fs_header[partition.header_index];

		// output structure
		sPartitionInfo& info = mPartitions[partition.header_index];

		// validate header hash
		nn::hac::detail::sha256_hash_t fs_header_hash;
		tc::crypto::GenerateSha256Hash(fs_header_hash.data(), (const byte_t*)&mHdrBlock.fs_header[partition.header_index], sizeof(nn::hac::sContentArchiveFsHeader));
		if (fs_header_hash != partition.fs_header_hash)
		{
			throw tc::Exception(mModuleName, fmt::format("NCA FS Header [{:d}] Hash: FAIL", partition.header_index));
		}

		if (fs_header.version.unwrap() != nn::hac::nca::kDefaultFsHeaderVersion)
		{
			throw tc::Exception(mModuleName, fmt::format("NCA FS Header [{:d}] Version({:d}): UNSUPPORTED", partition.header_index, fs_header.version.unwrap()));
		}

		// detect compacted partition (sparse layer)
		if (fs_header.sparse_info.generation.unwrap() != 0)
		{
			// TODO
		}

		// setup AES-CTR 
		nn::hac::ContentArchiveUtil::getNcaPartitionAesCtr(&fs_header, info.aes_ctr.data());

		// save partition configinfo
		info.offset = partition.offset;
		info.size = partition.size;
		info.format_type = (nn::hac::nca::FormatType)fs_header.format_type;
		info.hash_type = (nn::hac::nca::HashType)fs_header.hash_type;
		info.enc_type = (nn::hac::nca::EncryptionType)fs_header.encryption_type;
		if (info.hash_type == nn::hac::nca::HashType::HierarchicalSha256)
		{
			info.hierarchicalsha256_hdr.fromBytes(fs_header.hash_info.data(), fs_header.hash_info.size());
			/*
			nn::hac::HierarchicalSha256Header hdr;

			// import raw data
			hdr.fromBytes(fs_header.hash_info.data(), fs_header.hash_info.size());
			for (size_t i = 0; i < hdr.getLayerInfo().size(); i++)
			{
				nn::hac::HierarchicalValidatedStream::StreamInfo::LayerInfo layer;
				layer.offset = hdr.getLayerInfo()[i].offset;
				layer.size = hdr.getLayerInfo()[i].size;
				layer.block_size = hdr.getHashBlockSize();
				if (i + 1 == hdr.getLayerInfo().size())
				{
					info.hashed_stream_info.data_layer_info = layer;
				}
				else
				{
					info.hashed_stream_info.hash_layer_info.push_back(layer);
				}
			}
			info.hashed_stream_info.master_hash_data = tc::ByteData(hdr.getMasterHash().data(), hdr.getMasterHash().size());
			info.hashed_stream_info.align_partial_block_to_blocksize = false;
			*/
		}	
		else if (info.hash_type == nn::hac::nca::HashType::HierarchicalIntegrity)
		{
			info.hierarchicalintegrity_hdr.fromBytes(fs_header.hash_info.data(), fs_header.hash_info.size());
			/*
			// info.hash_tree_meta.importData(fs_header.hash_info, nn::hac::nca::kHashInfoLen, LayeredIntegrityMetadata::HASH_TYPE_INTEGRITY);
			nn::hac::HierarchicalIntegrityHeader hdr;

			hdr.fromBytes(fs_header.hash_info.data(), fs_header.hash_info.size());
			for (size_t i = 0; i < hdr.getLayerInfo().size(); i++)
			{
				nn::hac::HierarchicalValidatedStream::StreamInfo::LayerInfo layer;
				layer.offset = hdr.getLayerInfo()[i].offset;
				layer.size = hdr.getLayerInfo()[i].size;
				layer.block_size = (1 << hdr.getLayerInfo()[i].block_size);
				if (i + 1 == hdr.getLayerInfo().size())
				{
					info.hashed_stream_info.data_layer_info = layer;
				}
				else
				{
					info.hashed_stream_info.hash_layer_info.push_back(layer);
				}
			}
			info.hashed_stream_info.master_hash_data = tc::ByteData(hdr.getMasterHashList().size() * sizeof(nn::hac::detail::sha256_hash_t));
			for (size_t i = 0; i < hdr.getMasterHashList().size(); i++)
			{
				((nn::hac::detail::sha256_hash_t*)info.hashed_stream_info.master_hash_data.data())[i] = hdr.getMasterHashList()[i];
			}
			info.hashed_stream_info.align_partial_block_to_blocksize = false;
			*/
		}

		// create reader
		try 
		{
			// create reader based on encryption type0
			if (info.enc_type == nn::hac::nca::EncryptionType::None)
			{
				info.reader = std::make_shared<tc::io::SubStream>(tc::io::SubStream(mFile, info.offset, info.size));
			}
			else if (info.enc_type == nn::hac::nca::EncryptionType::AesCtr)
			{
				if (mContentKey.aes_ctr.isNull())
					throw tc::Exception(mModuleName, "AES-CTR Key was not determined");
				//info.reader = new fnd::OffsetAdjustedIFile(new fnd::AesCtrWrappedIFile(mFile, mContentKey.aes_ctr.get(), info.aes_ctr), info.offset, info.size);
				info.reader = std::make_shared<tc::crypto::Aes128CtrEncryptedStream>(tc::crypto::Aes128CtrEncryptedStream(mFile, mContentKey.aes_ctr.get(), info.aes_ctr));
				info.reader = std::make_shared<tc::io::SubStream>(tc::io::SubStream(info.reader, info.offset, info.size));

			}
			else if (info.enc_type == nn::hac::nca::EncryptionType::AesXts || info.enc_type == nn::hac::nca::EncryptionType::AesCtrEx)
			{
				throw tc::Exception(mModuleName, fmt::format("EncryptionType({:s}): UNSUPPORTED", nn::hac::ContentArchiveUtil::getEncryptionTypeAsString(info.enc_type)));
			}
			else
			{
				throw tc::Exception(mModuleName, fmt::format("EncryptionType({:s}): UNKNOWN", nn::hac::ContentArchiveUtil::getEncryptionTypeAsString(info.enc_type)));
			}

			// filter out unrecognised hash types, and hash based readers
			switch (info.hash_type)
			{
			case (nn::hac::nca::HashType::None):
				break;
			case (nn::hac::nca::HashType::HierarchicalSha256):
				info.reader = std::make_shared<nn::hac::HierarchicalSha256Stream>(nn::hac::HierarchicalSha256Stream(info.reader, info.hierarchicalsha256_hdr));
				break;
			case (nn::hac::nca::HashType::HierarchicalIntegrity):
				info.reader = std::make_shared<nn::hac::HierarchicalIntegrityStream>(nn::hac::HierarchicalIntegrityStream(info.reader, info.hierarchicalintegrity_hdr));
				break;
			default:
				throw tc::Exception(mModuleName, fmt::format("HashType({:s}): UNKNOWN", nn::hac::ContentArchiveUtil::getHashTypeAsString(info.hash_type)));
			}

			// filter out unrecognised format types
			switch (info.format_type)
			{
			case (nn::hac::nca::FormatType::PartitionFs):
				info.fs_meta = nn::hac::PartitionFsMetaGenerator(info.reader);
				info.fs_reader = std::make_shared<tc::io::VirtualFileSystem>(tc::io::VirtualFileSystem(info.fs_meta));
				break;
			case (nn::hac::nca::FormatType::RomFs):
				info.fs_meta = nn::hac::RomFsMetaGenerator(info.reader);
				info.fs_reader = std::make_shared<tc::io::VirtualFileSystem>(tc::io::VirtualFileSystem(info.fs_meta));
				break;
			default:
				throw tc::Exception(mModuleName, fmt::format("FormatType({:s}): UNKNOWN", nn::hac::ContentArchiveUtil::getFormatTypeAsString(info.format_type)));
			}
		}
		catch (const tc::Exception& e)
		{
			info.fail_reason = std::string(e.error());
		}
	}
}

void nstool::NcaProcess::validateNcaSignatures()
{
	// validate signature[0]
	if (mKeyCfg.nca_header_sign0_key.find(mHdr.getSignatureKeyGeneration()) != mKeyCfg.nca_header_sign0_key.end())
	{
		if (tc::crypto::VerifyRsa2048PssSha256(mHdrBlock.signature_main.data(), mHdrHash.data(), mKeyCfg.nca_header_sign0_key[mHdr.getSignatureKeyGeneration()]) == false)
		{
			fmt::print("[WARNING] NCA Header Main Signature: FAIL\n");
		}
	}
	else
	{
		fmt::print("[WARNING] NCA Header Main Signature: FAIL (could not load header key)\n");
	}
	

	// validate signature[1]
	if (mHdr.getContentType() == nn::hac::nca::ContentType::Program)
	{
		try {
			if (mPartitions[nn::hac::nca::PARTITION_CODE].format_type == nn::hac::nca::FormatType::PartitionFs)
			{
				if (mPartitions[nn::hac::nca::PARTITION_CODE].fs_reader != nullptr)
				{
					std::shared_ptr<tc::io::IStream> npdm_file;
					try {
						mPartitions[nn::hac::nca::PARTITION_CODE].fs_reader->openFile(tc::io::Path(kNpdmExefsPath), tc::io::FileMode::Open, tc::io::FileAccess::Read, npdm_file);
					}
					catch (tc::io::FileNotFoundException&) {
						throw tc::Exception(fmt::format("\"{:s}\" not present in ExeFs", kNpdmExefsPath));
					}

					MetaProcess npdm;
					npdm.setInputFile(npdm_file);
					npdm.setKeyCfg(mKeyCfg);
					npdm.setVerifyMode(true);
					npdm.setCliOutputMode(CliOutputMode(false, false, false, false));
					npdm.process();

					if (tc::crypto::VerifyRsa2048PssSha256(mHdrBlock.signature_acid.data(), mHdrHash.data(), npdm.getMeta().getAccessControlInfoDesc().getContentArchiveHeaderSignature2Key()) == false)
					{
						throw tc::Exception("Bad signature");
					}
				}
				else
				{
					throw tc::Exception("ExeFs was not mounted");
				}
			}
			else
			{
				throw tc::Exception("No ExeFs partition");
			}
		}
		catch (tc::Exception& e) {
			fmt::print("[WARNING] NCA Header ACID Signature: FAIL ({:s})\n", e.error());
		}
	}
}

void nstool::NcaProcess::displayHeader()
{
	fmt::print("[NCA Header]\n");
	fmt::print("  Format Type:     {:s}\n", nn::hac::ContentArchiveUtil::getFormatHeaderVersionAsString((nn::hac::nca::HeaderFormatVersion)mHdr.getFormatVersion()));
	fmt::print("  Dist. Type:      {:s}\n", nn::hac::ContentArchiveUtil::getDistributionTypeAsString(mHdr.getDistributionType()));
	fmt::print("  Content Type:    {:s}\n", nn::hac::ContentArchiveUtil::getContentTypeAsString(mHdr.getContentType()));
	fmt::print("  Key Generation:  {:d}\n", mHdr.getKeyGeneration());
	fmt::print("  Sig. Generation: {:d}\n", mHdr.getSignatureKeyGeneration());
	fmt::print("  Kaek Index:      {:s} ({:d})\n", nn::hac::ContentArchiveUtil::getKeyAreaEncryptionKeyIndexAsString((nn::hac::nca::KeyAreaEncryptionKeyIndex)mHdr.getKeyAreaEncryptionKeyIndex()), mHdr.getKeyAreaEncryptionKeyIndex());
	fmt::print("  Size:            0x{:x}\n", mHdr.getContentSize());
	fmt::print("  ProgID:          0x{:016x}\n", mHdr.getProgramId());
	fmt::print("  Content Index:   {:d}\n", mHdr.getContentIndex());
	fmt::print("  SdkAddon Ver.:   {:s} (v{:d})\n", nn::hac::ContentArchiveUtil::getSdkAddonVersionAsString(mHdr.getSdkAddonVersion()), mHdr.getSdkAddonVersion());
	if (mHdr.hasRightsId())
	{
		fmt::print("  RightsId:        {:s}\n", tc::cli::FormatUtil::formatBytesAsString(mHdr.getRightsId().data(), mHdr.getRightsId().size(), true, ""));
	}
	
	if (mContentKey.kak_list.size() > 0 && mCliOutputMode.show_keydata)
	{
		fmt::print("  Key Area:\n");
		fmt::print("    <--------------------------------------------------------------------------------------------------------->\n");
		fmt::print("    | IDX | ENCRYPTED KEY                                   | DECRYPTED KEY                                   |\n");
		fmt::print("    |-----|-------------------------------------------------|-------------------------------------------------|\n");
		for (size_t i = 0; i < mContentKey.kak_list.size(); i++)
		{
			fmt::print("    | {:3d} | {:s} | ", mContentKey.kak_list[i].index, tc::cli::FormatUtil::formatBytesAsString(mContentKey.kak_list[i].enc.data(), mContentKey.kak_list[i].enc.size(), true, ":"));
						
			
			if (mContentKey.kak_list[i].decrypted)
				fmt::print("{:s}", tc::cli::FormatUtil::formatBytesAsString(mContentKey.kak_list[i].dec.data(), mContentKey.kak_list[i].dec.size(), true, ":"));
			else
				fmt::print("<unable to decrypt>                            ");
			
			fmt::print(" |\n");
		}
		fmt::print("    <--------------------------------------------------------------------------------------------------------->\n");
	}

	if (mCliOutputMode.show_layout)
	{
		fmt::print("  Partitions:\n");
		for (size_t i = 0; i < mHdr.getPartitionEntryList().size(); i++)
		{
			uint32_t index = mHdr.getPartitionEntryList()[i].header_index;
			sPartitionInfo& info = mPartitions[index];
			if (info.size == 0) continue;

			fmt::print("    {:d}:\n", index);
			fmt::print("      Offset:      0x{:x}\n", info.offset);
			fmt::print("      Size:        0x{:x}\n", info.size);
			fmt::print("      Format Type: {:s}\n", nn::hac::ContentArchiveUtil::getFormatTypeAsString(info.format_type));
			fmt::print("      Hash Type:   {:s}\n", nn::hac::ContentArchiveUtil::getHashTypeAsString(info.hash_type));
			fmt::print("      Enc. Type:   {:s}\n", nn::hac::ContentArchiveUtil::getEncryptionTypeAsString(info.enc_type));
			if (info.enc_type == nn::hac::nca::EncryptionType::AesCtr)
			{
				nn::hac::detail::aes_iv_t aes_ctr;
				memcpy(aes_ctr.data(), info.aes_ctr.data(), aes_ctr.size());
				tc::crypto::detail::incr_counter<16>(aes_ctr.data(), info.offset>>4);
				fmt::print("      AesCtr Counter:\n");
				fmt::print("        {:s}\n", tc::cli::FormatUtil::formatBytesAsString(aes_ctr.data(), aes_ctr.size(), true, ":"));
			}
			if (info.hash_type == nn::hac::nca::HashType::HierarchicalIntegrity)
			{
				auto hash_hdr = info.hierarchicalintegrity_hdr;
				fmt::print("      HierarchicalIntegrity Header:\n");
				for (size_t j = 0; j < hash_hdr.getLayerInfo().size(); j++)
				{
					if (j+1 == hash_hdr.getLayerInfo().size())
					{
						fmt::print("        Data Layer:\n");
					}
					else
					{
						fmt::print("        Hash Layer {:d}:\n", j);
					}
					fmt::print("          Offset:          0x{:x}\n", hash_hdr.getLayerInfo()[j].offset);
					fmt::print("          Size:            0x{:x}\n", hash_hdr.getLayerInfo()[j].size);
					fmt::print("          BlockSize:       0x{:x}\n", hash_hdr.getLayerInfo()[j].block_size);
				}
				for (size_t j = 0; j < hash_hdr.getMasterHashList().size(); j++)
				{
					fmt::print("        Master Hash {:d}:\n", j);
					fmt::print("          {:s}\n", tc::cli::FormatUtil::formatBytesAsString(hash_hdr.getMasterHashList()[j].data(), 0x10, true, ":"));
					fmt::print("          {:s}\n", tc::cli::FormatUtil::formatBytesAsString(hash_hdr.getMasterHashList()[j].data()+0x10, 0x10, true, ":"));
				}
			}
			else if (info.hash_type == nn::hac::nca::HashType::HierarchicalSha256)
			{
				auto hash_hdr = info.hierarchicalsha256_hdr;
				fmt::print("      HierarchicalSha256 Header:\n");
				fmt::print("        Master Hash:\n");
				fmt::print("          {:s}\n", tc::cli::FormatUtil::formatBytesAsString(hash_hdr.getMasterHash().data(), 0x10, true, ":"));
				fmt::print("          {:s}\n", tc::cli::FormatUtil::formatBytesAsString(hash_hdr.getMasterHash().data()+0x10, 0x10, true, ":"));
				fmt::print("        HashBlockSize:     0x{:x}\n", hash_hdr.getHashBlockSize());
				for (size_t j = 0; j < hash_hdr.getLayerInfo().size(); j++)
				{
					if (j+1 == hash_hdr.getLayerInfo().size())
					{
						fmt::print("        Data Layer:\n");
					}
					else
					{
						fmt::print("        Hash Layer {:d}:\n", j);
					}
					fmt::print("          Offset:          0x{:x}\n", hash_hdr.getLayerInfo()[j].offset);
					fmt::print("          Size:            0x{:x}\n", hash_hdr.getLayerInfo()[j].size);
				}
			}
		}
	}
}


void nstool::NcaProcess::processPartitions()
{
	std::vector<nn::hac::CombinedFsMetaGenerator::MountPointInfo> mount_points;

	for (size_t i = 0; i < mHdr.getPartitionEntryList().size(); i++)
	{
		uint32_t index = mHdr.getPartitionEntryList()[i].header_index;
		struct sPartitionInfo& partition = mPartitions[index];

		// if the reader is null, skip
		if (partition.fs_reader == nullptr)
		{
			fmt::print("[WARNING] NCA Partition {:d} not readable.", index);
			if (partition.fail_reason.empty() == false)
			{
				fmt::print(" ({:s})", partition.fail_reason);
			}
			fmt::print("\n");
			continue;
		}

		std::string mount_point_name;
		/*
		if (mHdr.getContentType() == nn::hac::nca::ContentType::Program)
		{
			mount_point_name = nn::hac::ContentArchiveUtil::getProgramContentParititionIndexAsString((nn::hac::nca::ProgramContentPartitionIndex)index);
		}
		else
		*/
		{
			mount_point_name = fmt::format("{:d}", index);
		}

		mount_points.push_back( { mount_point_name, partition.fs_meta } );
	}

	tc::io::VirtualFileSystem::FileSystemMeta fs_meta = nn::hac::CombinedFsMetaGenerator(mount_points);

	std::shared_ptr<tc::io::IStorage> nca_fs = std::make_shared<tc::io::VirtualFileSystem>(tc::io::VirtualFileSystem(fs_meta));

	mFsProcess.setInputFileSystem(nca_fs);
	mFsProcess.setFsFormatName("ContentArchive");
	mFsProcess.setFsRootLabel(getContentTypeForMountStr(mHdr.getContentType()));
	mFsProcess.process();
}

std::string nstool::NcaProcess::getContentTypeForMountStr(nn::hac::nca::ContentType cont_type) const
{
	std::string str;

	switch (cont_type)
	{
		case (nn::hac::nca::ContentType::Program):
			str = "program";
			break;
		case (nn::hac::nca::ContentType::Meta):
			str = "meta";
			break;
		case (nn::hac::nca::ContentType::Control):
			str = "control";
			break;
		case (nn::hac::nca::ContentType::Manual):
			str = "manual";
			break;
		case (nn::hac::nca::ContentType::Data):
			str = "data";
			break;
		case (nn::hac::nca::ContentType::PublicData):
			str = "publicdata";
			break;
		default:
			str = "";
			break;
	}

	return str;
}