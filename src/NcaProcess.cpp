#include "NcaProcess.h"
#include "MetaProcess.h"
#include "util.h"

#include <pietendo/hac/ContentArchiveUtil.h>
#include <pietendo/hac/AesKeygen.h>
#include <pietendo/hac/HierarchicalSha256Stream.h>
#include <pietendo/hac/HierarchicalIntegrityStream.h>
#include <pietendo/hac/BKTREncryptedStream.h>
#include <pietendo/hac/PartitionFsSnapshotGenerator.h>
#include <pietendo/hac/RomFsSnapshotGenerator.h>
#include <pietendo/hac/CombinedFsSnapshotGenerator.h>

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

void nstool::NcaProcess::setBaseNcaPath(const tc::Optional<tc::io::Path>& nca_path)
{
	mBaseNcaPath = nca_path;
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

const std::shared_ptr<tc::io::IFileSystem>& nstool::NcaProcess::getFileSystem() const
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
	if (mFile->length() < tc::io::IOUtil::castSizeToInt64(sizeof(pie::hac::sContentArchiveHeaderBlock)))
	{
		throw tc::Exception(mModuleName, "Corrupt NCA: File too small.");
	}
	mFile->seek(0, tc::io::SeekOrigin::Begin);
	mFile->read((byte_t*)(&mHdrBlock), sizeof(pie::hac::sContentArchiveHeaderBlock));

	// decrypt header block
	if (mKeyCfg.nca_header_key.isNull())
	{
		throw tc::Exception(mModuleName, "Failed to decrypt NCA header. (nca_header_key could not be loaded)");
	}
	pie::hac::ContentArchiveUtil::decryptContentArchiveHeader((byte_t*)&mHdrBlock, (byte_t*)&mHdrBlock, mKeyCfg.nca_header_key.get());

	// generate header hash
	tc::crypto::GenerateSha2256Hash(mHdrHash.data(), (byte_t*)&mHdrBlock.header, sizeof(pie::hac::sContentArchiveHeader));

	// proccess main header
	mHdr.fromBytes((byte_t*)&mHdrBlock.header, sizeof(pie::hac::sContentArchiveHeader));
}

void nstool::NcaProcess::generateNcaBodyEncryptionKeys()
{
	// create zeros key
	KeyBag::aes128_key_t zero_aesctr_key;
	memset(zero_aesctr_key.data(), 0, zero_aesctr_key.size());
	
	// get key data from header
	byte_t masterkey_rev = pie::hac::AesKeygen::getMasterKeyRevisionFromKeyGeneration(mHdr.getKeyGeneration());
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
				pie::hac::AesKeygen::generateKey(kak.dec.data(), kak.enc.data(), mKeyCfg.nca_key_area_encryption_key[keak_index][masterkey_rev].data());
			}
			// key[KeyBankIndex_AesCtrHw]
			else if (i == pie::hac::nca::KeyBankIndex_AesCtrHw && mKeyCfg.nca_key_area_encryption_key_hw[keak_index].find(masterkey_rev) != mKeyCfg.nca_key_area_encryption_key_hw[keak_index].end())
			{
				kak.decrypted = true;
				pie::hac::AesKeygen::generateKey(kak.dec.data(), kak.enc.data(), mKeyCfg.nca_key_area_encryption_key_hw[keak_index][masterkey_rev].data());
			}
			else
			{
				kak.decrypted = false;
			}
			mContentKey.kak_list.push_back(kak);
		}
	}

	// clear content key
	mContentKey.aes_ctr = tc::Optional<pie::hac::detail::aes128_key_t>();

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
		else if (mKeyCfg.external_enc_content_keys.find(mHdr.getRightsId()) != mKeyCfg.external_enc_content_keys.end())
		{
			tmp_key = mKeyCfg.external_enc_content_keys[mHdr.getRightsId()];
			if (mKeyCfg.etik_common_key.find(masterkey_rev) != mKeyCfg.etik_common_key.end())
			{
				pie::hac::AesKeygen::generateKey(tmp_key.data(), tmp_key.data(), mKeyCfg.etik_common_key[masterkey_rev].data());
				mContentKey.aes_ctr = tmp_key;
			}
		}
		else if (mKeyCfg.fallback_enc_content_key.isSet())
		{
			tmp_key = mKeyCfg.fallback_enc_content_key.get();
			if (mKeyCfg.etik_common_key.find(masterkey_rev) != mKeyCfg.etik_common_key.end())
			{
				pie::hac::AesKeygen::generateKey(tmp_key.data(), tmp_key.data(), mKeyCfg.etik_common_key[masterkey_rev].data());
				mContentKey.aes_ctr = tmp_key;
			}
		}
	}
	// otherwise used decrypt key area
	else
	{
		for (size_t i = 0; i < mContentKey.kak_list.size(); i++)
		{
			if (mContentKey.kak_list[i].index == pie::hac::nca::KeyBankIndex_AesCtr && mContentKey.kak_list[i].decrypted)
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
			fmt::print("  AES-CTR Key: {:s}\n", tc::cli::FormatUtil::formatBytesAsString(mContentKey.aes_ctr.get().data(), mContentKey.aes_ctr.get().size(), true, ""));
		}
	}
}

nstool::NcaProcess nstool::NcaProcess::readBaseNCA()
{
	// open base nca stream
	if (mBaseNcaPath.isNull())
	{
		throw tc::Exception(mModuleName, "Base NCA not supplied. Necessary for update NCA.");
	}
	std::shared_ptr<tc::io::IStream> base_stream = std::make_shared<tc::io::FileStream>(tc::io::FileStream(mBaseNcaPath.get(), tc::io::FileMode::Open, tc::io::FileAccess::Read));

	// process base nca with output suppressed
	NcaProcess obj;
	nstool::CliOutputMode cliOutput;
	cliOutput.show_basic_info = false;
	cliOutput.show_extended_info = false;
	cliOutput.show_keydata = false;
	cliOutput.show_layout = false;
	obj.setCliOutputMode(cliOutput);
	obj.setVerifyMode(true);
	obj.setKeyCfg(mKeyCfg);
	obj.setInputFile(base_stream);
	obj.process();

	// return processed base nca
	return obj;
}

void nstool::NcaProcess::generatePartitionConfiguration()
{
	for (size_t i = 0; i < mHdr.getPartitionEntryList().size(); i++)
	{
		// get reference to relevant structures
		const pie::hac::ContentArchiveHeader::sPartitionEntry& partition = mHdr.getPartitionEntryList()[i];
		pie::hac::sContentArchiveFsHeader& fs_header = mHdrBlock.fs_header[partition.header_index];

		// output structure
		sPartitionInfo& info = mPartitions[partition.header_index];

		// validate header hash
		pie::hac::detail::sha256_hash_t fs_header_hash;
		tc::crypto::GenerateSha2256Hash(fs_header_hash.data(), (const byte_t*)&mHdrBlock.fs_header[partition.header_index], sizeof(pie::hac::sContentArchiveFsHeader));
		if (fs_header_hash != partition.fs_header_hash)
		{
			throw tc::Exception(mModuleName, fmt::format("NCA FS Header [{:d}] Hash: FAIL", partition.header_index));
		}

		if (fs_header.version.unwrap() != pie::hac::nca::kDefaultFsHeaderVersion)
		{
			throw tc::Exception(mModuleName, fmt::format("NCA FS Header [{:d}] Version({:d}): UNSUPPORTED", partition.header_index, fs_header.version.unwrap()));
		}

		// setup AES-CTR 
		pie::hac::ContentArchiveUtil::getNcaPartitionAesCtr(&fs_header, info.aes_ctr.data());

		// save partition configinfo
		info.offset = partition.offset;
		info.size = partition.size;
		info.format_type = (pie::hac::nca::FormatType)fs_header.format_type;
		info.hash_type = (pie::hac::nca::HashType)fs_header.hash_type;
		info.enc_type = (pie::hac::nca::EncryptionType)fs_header.encryption_type;
		info.metadata_hash_type = (pie::hac::nca::MetaDataHashType)fs_header.meta_data_hash_type;

		if (info.hash_type == pie::hac::nca::HashType_HierarchicalSha256)
		{
			info.hierarchicalsha256_hdr.fromBytes(fs_header.hash_info.data(), fs_header.hash_info.size());
		}	
		else if (info.hash_type == pie::hac::nca::HashType_HierarchicalIntegrity)
		{
			info.hierarchicalintegrity_hdr.fromBytes(fs_header.hash_info.data(), fs_header.hash_info.size());
		}

		// create reader
		try 
		{
			// handle partition encryption and partition compaction (sparse layer)
			if (fs_header.sparse_info.generation.unwrap() != 0)
			{
				throw tc::Exception("SparseStorage: Not currently supported.");
			}
			else
			{
				// create raw partition
				info.raw_reader = std::make_shared<tc::io::SubStream>(tc::io::SubStream(mFile, info.offset, info.size));

				// handle encryption if required reader based on encryption type
				if (info.enc_type == pie::hac::nca::EncryptionType_None)
				{
					// no encryption so do nothing
					info.decrypt_reader = info.raw_reader;
				}
				else if (info.enc_type == pie::hac::nca::EncryptionType_AesCtr)
				{
					if (mContentKey.aes_ctr.isNull())
						throw tc::Exception(mModuleName, "AES-CTR Key was not determined");

					// get partition key
					pie::hac::detail::aes128_key_t partition_key = mContentKey.aes_ctr.get();

					// get partition counter
					pie::hac::detail::aes_iv_t partition_ctr = info.aes_ctr;
					tc::crypto::IncrementCounterAes128Ctr(partition_ctr.data(), info.offset >> 4);

					// create decryption stream
					info.decrypt_reader = std::make_shared<tc::crypto::Aes128CtrEncryptedStream>(tc::crypto::Aes128CtrEncryptedStream(info.raw_reader, partition_key, partition_ctr));
				}
				else if (info.enc_type == pie::hac::nca::EncryptionType_AesCtrEx)
				{
					if (mContentKey.aes_ctr.isNull())
						throw tc::Exception(mModuleName, "AES-CTR Key was not determined");

					// get partition key
					pie::hac::detail::aes128_key_t partition_key = mContentKey.aes_ctr.get();

					// get partition counter
					pie::hac::detail::aes_iv_t partition_ctr = info.aes_ctr;
					tc::crypto::IncrementCounterAes128Ctr(partition_ctr.data(), info.offset >> 4);

					// TODO see if AesCtrEx encryption can just be for creating the transparent decryption, with IndirectStorage IStream construction being done after decryption but before hash layer processing
					// this might be relevant when processing compressed or sparse storage

					NcaProcess nca_base = readBaseNCA();
					if (nca_base.mHdr.getProgramId() != mHdr.getProgramId())
					{
						throw tc::Exception(mModuleName, "Invalid base nca. ProgramID diferent.");
					}

					std::shared_ptr<tc::io::IStream> base_reader;
					for (auto& partition_base : nca_base.mPartitions)
					{
						if (partition_base.format_type == pie::hac::nca::FormatType::FormatType_RomFs && partition_base.raw_reader != nullptr)
						{
							base_reader = partition_base.decrypt_reader;
						}
					}
					if (base_reader == nullptr)
					{
						throw tc::Exception(mModuleName, "Cannot determine RomFs from base nca.");
					}

					// create decryption stream
					info.decrypt_reader = std::make_shared<pie::hac::BKTREncryptedStream>(pie::hac::BKTREncryptedStream(info.raw_reader, partition_key, partition_ctr, fs_header.patch_info, base_reader));
				}
				else if (info.enc_type == pie::hac::nca::EncryptionType_AesXts)
				{
					throw tc::Exception(mModuleName, fmt::format("EncryptionType({:s}): UNSUPPORTED", pie::hac::ContentArchiveUtil::getEncryptionTypeAsString(info.enc_type)));
				}
				else
				{
					throw tc::Exception(mModuleName, fmt::format("EncryptionType({:s}): UNKNOWN", pie::hac::ContentArchiveUtil::getEncryptionTypeAsString(info.enc_type)));
				}
			}

			// filter out unrecognised hash types, and hash based readers
			switch (info.hash_type)
			{
			case (pie::hac::nca::HashType_None):
				// no hash layer, do nothing
				info.reader = info.decrypt_reader;
				break;
			case (pie::hac::nca::HashType_HierarchicalSha256):
				info.reader = std::make_shared<pie::hac::HierarchicalSha256Stream>(pie::hac::HierarchicalSha256Stream(info.decrypt_reader, info.hierarchicalsha256_hdr));
				break;
			case (pie::hac::nca::HashType_HierarchicalIntegrity):
				info.reader = std::make_shared<pie::hac::HierarchicalIntegrityStream>(pie::hac::HierarchicalIntegrityStream(info.decrypt_reader, info.hierarchicalintegrity_hdr));
				break;
			default:
				throw tc::Exception(mModuleName, fmt::format("HashType({:s}): UNKNOWN", pie::hac::ContentArchiveUtil::getHashTypeAsString(info.hash_type)));
			}

			// filter out unrecognised format types
			switch (info.format_type)
			{
			case (pie::hac::nca::FormatType_PartitionFs):
				info.fs_snapshot = pie::hac::PartitionFsSnapshotGenerator(info.reader);
				info.fs_reader = std::make_shared<tc::io::VirtualFileSystem>(tc::io::VirtualFileSystem(info.fs_snapshot));
				break;
			case (pie::hac::nca::FormatType_RomFs):
				info.fs_snapshot = pie::hac::RomFsSnapshotGenerator(info.reader);
				info.fs_reader = std::make_shared<tc::io::VirtualFileSystem>(tc::io::VirtualFileSystem(info.fs_snapshot));
				break;
			default:
				throw tc::Exception(mModuleName, fmt::format("FormatType({:s}): UNKNOWN", pie::hac::ContentArchiveUtil::getFormatTypeAsString(info.format_type)));
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
		if (tc::crypto::VerifyRsa2048PssSha2256(mHdrBlock.signature_main.data(), mHdrHash.data(), mKeyCfg.nca_header_sign0_key[mHdr.getSignatureKeyGeneration()]) == false)
		{
			fmt::print("[WARNING] NCA Header Main Signature: FAIL\n");
		}
	}
	else
	{
		fmt::print("[WARNING] NCA Header Main Signature: FAIL (could not load header key)\n");
	}
	

	// validate signature[1]
	if (mHdr.getContentType() == pie::hac::nca::ContentType_Program)
	{
		try {
			if (mPartitions[pie::hac::nca::ProgramContentPartitionIndex_Code].format_type == pie::hac::nca::FormatType_PartitionFs)
			{
				if (mPartitions[pie::hac::nca::ProgramContentPartitionIndex_Code].fs_reader != nullptr)
				{
					std::shared_ptr<tc::io::IStream> npdm_file;
					try {
						mPartitions[pie::hac::nca::ProgramContentPartitionIndex_Code].fs_reader->openFile(tc::io::Path(kNpdmExefsPath), tc::io::FileMode::Open, tc::io::FileAccess::Read, npdm_file);
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

					if (tc::crypto::VerifyRsa2048PssSha2256(mHdrBlock.signature_acid.data(), mHdrHash.data(), npdm.getMeta().getAccessControlInfoDesc().getContentArchiveHeaderSignature2Key()) == false)
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
	fmt::print("  Format Type:     {:s}\n", pie::hac::ContentArchiveUtil::getFormatHeaderVersionAsString((pie::hac::nca::HeaderFormatVersion)mHdr.getFormatVersion()));
	fmt::print("  Dist. Type:      {:s}\n", pie::hac::ContentArchiveUtil::getDistributionTypeAsString(mHdr.getDistributionType()));
	fmt::print("  Content Type:    {:s}\n", pie::hac::ContentArchiveUtil::getContentTypeAsString(mHdr.getContentType()));
	fmt::print("  Key Generation:  {:d}\n", mHdr.getKeyGeneration());
	fmt::print("  Sig. Generation: {:d}\n", mHdr.getSignatureKeyGeneration());
	fmt::print("  Kaek Index:      {:s} ({:d})\n", pie::hac::ContentArchiveUtil::getKeyAreaEncryptionKeyIndexAsString((pie::hac::nca::KeyAreaEncryptionKeyIndex)mHdr.getKeyAreaEncryptionKeyIndex()), mHdr.getKeyAreaEncryptionKeyIndex());
	fmt::print("  Size:            0x{:x}\n", mHdr.getContentSize());
	fmt::print("  ProgID:          0x{:016x}\n", mHdr.getProgramId());
	fmt::print("  Content Index:   {:d}\n", mHdr.getContentIndex());
	fmt::print("  SdkAddon Ver.:   {:s} (v{:d})\n", pie::hac::ContentArchiveUtil::getSdkAddonVersionAsString(mHdr.getSdkAddonVersion()), mHdr.getSdkAddonVersion());
	if (mHdr.hasRightsId())
	{
		fmt::print("  RightsId:        {:s}\n", tc::cli::FormatUtil::formatBytesAsString(mHdr.getRightsId().data(), mHdr.getRightsId().size(), true, ""));
	}
	
	if (mContentKey.kak_list.size() > 0 && mCliOutputMode.show_keydata)
	{
		fmt::print("  Key Area:\n");
		fmt::print("    <--------------------------------------------------------------------------->\n");
		fmt::print("    | IDX | ENCRYPTED KEY                    | DECRYPTED KEY                    |\n");
		fmt::print("    |-----|----------------------------------|----------------------------------|\n");
		for (size_t i = 0; i < mContentKey.kak_list.size(); i++)
		{
			std::string enc_key = tc::cli::FormatUtil::formatBytesAsString(mContentKey.kak_list[i].enc.data(), mContentKey.kak_list[i].enc.size(), true, "");
			std::string dec_key = mContentKey.kak_list[i].decrypted ? tc::cli::FormatUtil::formatBytesAsString(mContentKey.kak_list[i].dec.data(), mContentKey.kak_list[i].dec.size(), true, "") : "<unable to decrypt>";
			
			fmt::print("    | {:3d} | {:32s} | {:32s} |\n", mContentKey.kak_list[i].index, enc_key, dec_key);
		
		}
		fmt::print("    <--------------------------------------------------------------------------->\n");
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
			fmt::print("      Format Type: {:s}\n", pie::hac::ContentArchiveUtil::getFormatTypeAsString(info.format_type));
			fmt::print("      Hash Type:   {:s}\n", pie::hac::ContentArchiveUtil::getHashTypeAsString(info.hash_type));
			fmt::print("      Enc. Type:   {:s}\n", pie::hac::ContentArchiveUtil::getEncryptionTypeAsString(info.enc_type));
			if (info.enc_type == pie::hac::nca::EncryptionType_AesCtr)
			{
				pie::hac::detail::aes_iv_t aes_ctr;
				memcpy(aes_ctr.data(), info.aes_ctr.data(), aes_ctr.size());
				tc::crypto::IncrementCounterAes128Ctr(aes_ctr.data(), info.offset>>4);
				fmt::print("      AesCtr Counter:\n");
				fmt::print("        {:s}\n", tc::cli::FormatUtil::formatBytesAsString(aes_ctr.data(), aes_ctr.size(), true, ""));
			}
			if (info.hash_type == pie::hac::nca::HashType_HierarchicalIntegrity)
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
					fmt::print("          {:s}\n", tc::cli::FormatUtil::formatBytesAsString(hash_hdr.getMasterHashList()[j].data(), 0x10, true, ""));
					fmt::print("          {:s}\n", tc::cli::FormatUtil::formatBytesAsString(hash_hdr.getMasterHashList()[j].data()+0x10, 0x10, true, ""));
				}
			}
			else if (info.hash_type == pie::hac::nca::HashType_HierarchicalSha256)
			{
				auto hash_hdr = info.hierarchicalsha256_hdr;
				fmt::print("      HierarchicalSha256 Header:\n");
				fmt::print("        Master Hash:\n");
				fmt::print("          {:s}\n", tc::cli::FormatUtil::formatBytesAsString(hash_hdr.getMasterHash().data(), 0x10, true, ""));
				fmt::print("          {:s}\n", tc::cli::FormatUtil::formatBytesAsString(hash_hdr.getMasterHash().data()+0x10, 0x10, true, ""));
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
	std::vector<pie::hac::CombinedFsSnapshotGenerator::MountPointInfo> mount_points;

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
		if (mHdr.getContentType() == pie::hac::nca::ContentType_Program)
		{
			mount_point_name = pie::hac::ContentArchiveUtil::getProgramContentParititionIndexAsString((pie::hac::nca::ProgramContentPartitionIndex)index);
		}
		else
		*/
		{
			mount_point_name = fmt::format("{:d}", index);
		}

		mount_points.push_back( { mount_point_name, partition.fs_snapshot } );
	}

	tc::io::VirtualFileSystem::FileSystemSnapshot fs_snapshot = pie::hac::CombinedFsSnapshotGenerator(mount_points);

	std::shared_ptr<tc::io::IFileSystem> nca_fs = std::make_shared<tc::io::VirtualFileSystem>(tc::io::VirtualFileSystem(fs_snapshot));

	mFsProcess.setInputFileSystem(nca_fs);
	mFsProcess.setFsFormatName("ContentArchive");
	mFsProcess.setFsRootLabel(getContentTypeForMountStr(mHdr.getContentType()));
	mFsProcess.process();
}

std::string nstool::NcaProcess::getContentTypeForMountStr(pie::hac::nca::ContentType cont_type) const
{
	std::string str;

	switch (cont_type)
	{
		case (pie::hac::nca::ContentType_Program):
			str = "program";
			break;
		case (pie::hac::nca::ContentType_Meta):
			str = "meta";
			break;
		case (pie::hac::nca::ContentType_Control):
			str = "control";
			break;
		case (pie::hac::nca::ContentType_Manual):
			str = "manual";
			break;
		case (pie::hac::nca::ContentType_Data):
			str = "data";
			break;
		case (pie::hac::nca::ContentType_PublicData):
			str = "publicdata";
			break;
		default:
			str = "";
			break;
	}

	return str;
}