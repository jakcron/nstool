#include <sstream>
#include <iostream>
#include <iomanip>
#include <fnd/types.h>

#include "RoMetadataProcess.h"

RoMetadataProcess::RoMetadataProcess() :
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mInstructionType(nn::hac::npdm::INSTR_64BIT),
	mListApi(false),
	mListSymbols(false),
	mApiInfo(),
	mDynSym(),
	mDynStr(),
	mRoBlob(),
	mSdkVerApiList(),
	mPublicApiList(),
	mDebugApiList(),
	mPrivateApiList(),
	mSymbolList()
{

}

void RoMetadataProcess::process()
{
	importApiList();
	
	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayRoMetaData();
}

void RoMetadataProcess::setRoBinary(const fnd::Vec<byte_t>& bin)
{
	mRoBlob = bin;
}

void RoMetadataProcess::setApiInfo(size_t offset, size_t size)
{
	mApiInfo.offset = offset;
	mApiInfo.size = size;
}
void RoMetadataProcess::setDynSym(size_t offset, size_t size)
{
	mDynSym.offset = offset;
	mDynSym.size = size;
}
void RoMetadataProcess::setDynStr(size_t offset, size_t size)
{
	mDynStr.offset = offset;
	mDynStr.size = size;
}

void RoMetadataProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void RoMetadataProcess::setInstructionType(nn::hac::npdm::InstructionType type)
{
	mInstructionType = type;
}

void RoMetadataProcess::setListApi(bool listApi)
{
	mListApi = listApi;
}

void RoMetadataProcess::setListSymbols(bool listSymbols)
{
	mListSymbols = listSymbols;
}

const std::vector<SdkApiString>& RoMetadataProcess::getSdkVerApiList() const
{
	return mSdkVerApiList;
}

const std::vector<SdkApiString>& RoMetadataProcess::getPublicApiList() const
{
	return mPublicApiList;
}

const std::vector<SdkApiString>& RoMetadataProcess::getDebugApiList() const
{
	return mDebugApiList;
}

const std::vector<SdkApiString>& RoMetadataProcess::getPrivateApiList() const
{
	return mPrivateApiList;
}

const fnd::List<ElfSymbolParser::sElfSymbol>& RoMetadataProcess::getSymbolList() const
{
	return mSymbolList.getSymbolList();
}

void RoMetadataProcess::importApiList()
{
	if (mRoBlob.size() == 0)
	{
		throw fnd::Exception(kModuleName, "No ro binary set.");
	}

	if (mApiInfo.size > 0)
	{
		std::stringstream list_stream(std::string((char*)mRoBlob.data() + mApiInfo.offset, mApiInfo.size));
		std::string api_str;

		while(std::getline(list_stream, api_str, (char)0x00))
		{
			SdkApiString api(api_str);

			if (api.getApiType() == SdkApiString::API_SDK_VERSION)
				mSdkVerApiList.push_back(api);
			else if (api.getApiType() == SdkApiString::API_MIDDLEWARE)
				mPublicApiList.push_back(api);
			else if (api.getApiType() == SdkApiString::API_DEBUG)
				mDebugApiList.push_back(api);
			else if (api.getApiType() == SdkApiString::API_PRIVATE)
				mPrivateApiList.push_back(api);
		}
	}

	if (mDynSym.size > 0)
	{
		mSymbolList.parseData(mRoBlob.data() + mDynSym.offset, mDynSym.size, mRoBlob.data() + mDynStr.offset, mDynStr.size, mInstructionType == nn::hac::npdm::INSTR_64BIT);
	}
}

void RoMetadataProcess::displayRoMetaData()
{
	size_t api_num = mSdkVerApiList.size() + mPublicApiList.size() + mDebugApiList.size() + mPrivateApiList.size();
	
	if (api_num > 0 && (mListApi || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED)))
	{
		std::cout << "[SDK API List]" << std::endl;
		if (mSdkVerApiList.size() > 0)
		{
			std::cout << "  Sdk Revision: " << mSdkVerApiList[0].getModuleName() << std::endl;
		}
		if (mPublicApiList.size() > 0)
		{
			std::cout << "  Public APIs:" << std::endl;
			for (size_t i = 0; i < mPublicApiList.size(); i++)
			{
				std::cout << "    " << mPublicApiList[i].getModuleName() << " (vender: " << mPublicApiList[i].getVenderName() << ")" << std::endl;
			}
		}
		if (mDebugApiList.size() > 0)
		{
			std::cout << "  Debug APIs:" << std::endl;
			for (size_t i = 0; i < mDebugApiList.size(); i++)
			{
				std::cout << "    " << mDebugApiList[i].getModuleName() << " (vender: " << mDebugApiList[i].getVenderName() << ")" << std::endl;
			}
		}
		if (mPrivateApiList.size() > 0)
		{
			std::cout << "  Private APIs:" << std::endl;
			for (size_t i = 0; i < mPrivateApiList.size(); i++)
			{
				std::cout << "    " << mPrivateApiList[i].getModuleName() << " (vender: " << mPrivateApiList[i].getVenderName() << ")" << std::endl;
			}
		}
	}
	if (mSymbolList.getSymbolList().size() > 0 && (mListSymbols || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED)))
	{
		std::cout << "[Symbol List]" << std::endl;
		for (size_t i = 0; i < mSymbolList.getSymbolList().size(); i++)
		{
			const ElfSymbolParser::sElfSymbol& symbol = mSymbolList.getSymbolList()[i];
			std::cout << "  " << symbol.name << " [SHN=" << getSectionIndexStr(symbol.shn_index) << " (" << std::hex << std::setw(4) << std::setfill('0') << symbol.shn_index << ")][STT=" << getSymbolTypeStr(symbol.symbol_type) << "][STB=" << getSymbolBindingStr(symbol.symbol_binding) << "]" << std::endl;
		}
	}
}

const char* RoMetadataProcess::getSectionIndexStr(uint16_t shn_index) const
{
	const char* str;
	switch (shn_index)
	{
		case (fnd::elf::SHN_UNDEF):
			str = "UNDEF";
			break;
		case (fnd::elf::SHN_LOPROC):
			str = "LOPROC";
			break;
		case (fnd::elf::SHN_HIPROC):
			str = "HIPROC";
			break;
		case (fnd::elf::SHN_LOOS):
			str = "LOOS";
			break;
		case (fnd::elf::SHN_HIOS):
			str = "HIOS";
			break;
		case (fnd::elf::SHN_ABS):
			str = "ABS";
			break;
		case (fnd::elf::SHN_COMMON):
			str = "COMMON";
			break;
		default:
			str = "UNKNOWN";
			break;
	}
	return str;
}

const char* RoMetadataProcess::getSymbolTypeStr(byte_t symbol_type) const
{
	const char* str;
	switch (symbol_type)
	{
		case (fnd::elf::STT_NOTYPE):
			str = "NOTYPE";
			break;
		case (fnd::elf::STT_OBJECT):
			str = "OBJECT";
			break;
		case (fnd::elf::STT_FUNC):
			str = "FUNC";
			break;
		case (fnd::elf::STT_SECTION):
			str = "SECTION";
			break;
		case (fnd::elf::STT_FILE):
			str = "FILE";
			break;
		case (fnd::elf::STT_LOOS):
			str = "LOOS";
			break;
		case (fnd::elf::STT_HIOS):
			str = "HIOS";
			break;
		case (fnd::elf::STT_LOPROC):
			str = "LOPROC";
			break;
		case (fnd::elf::STT_HIPROC):
			str = "HIPROC";
			break;
		default:
			str = "UNKNOWN";
			break;
	}
	return str;
}

const char* RoMetadataProcess::getSymbolBindingStr(byte_t symbol_binding) const
{
	const char* str;
	switch (symbol_binding)
	{
		case (fnd::elf::STB_LOCAL):
			str = "LOCAL";
			break;
		case (fnd::elf::STB_GLOBAL):
			str = "GLOBAL";
			break;
		case (fnd::elf::STB_WEAK):
			str = "WEAK";
			break;
		case (fnd::elf::STB_LOOS):
			str = "LOOS";
			break;
		case (fnd::elf::STB_HIOS):
			str = "HIOS";
			break;
		case (fnd::elf::STB_LOPROC):
			str = "LOPROC";
			break;
		case (fnd::elf::STB_HIPROC):
			str = "HIPROC";
			break;
		default:
			str = "UNKNOWN";
			break;
	}
	return str;
}