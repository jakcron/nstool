#include <sstream>
#include <iostream>
#include <iomanip>
#include <fnd/types.h>

#include "RoMetadataProcess.h"

nstool::RoMetadataProcess::RoMetadataProcess() :
	mCliOutputMode(true, false, false, false),
	mIs64BitInstruction(true),
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

void nstool::RoMetadataProcess::process()
{
	importApiList();
	
	if (mCliOutputMode.show_basic_info)
		displayRoMetaData();
}

void nstool::RoMetadataProcess::setRoBinary(const tc::ByteData& bin)
{
	mRoBlob = bin;
}

void nstool::RoMetadataProcess::setApiInfo(size_t offset, size_t size)
{
	mApiInfo.offset = offset;
	mApiInfo.size = size;
}
void nstool::RoMetadataProcess::setDynSym(size_t offset, size_t size)
{
	mDynSym.offset = offset;
	mDynSym.size = size;
}
void nstool::RoMetadataProcess::setDynStr(size_t offset, size_t size)
{
	mDynStr.offset = offset;
	mDynStr.size = size;
}

void nstool::RoMetadataProcess::setCliOutputMode(CliOutputMode type)
{
	mCliOutputMode = type;
}

void nstool::RoMetadataProcess::setIs64BitInstruction(bool flag)
{
	mIs64BitInstruction = flag;
}

void nstool::RoMetadataProcess::setListApi(bool listApi)
{
	mListApi = listApi;
}

void nstool::RoMetadataProcess::setListSymbols(bool listSymbols)
{
	mListSymbols = listSymbols;
}

const std::vector<SdkApiString>& nstool::RoMetadataProcess::getSdkVerApiList() const
{
	return mSdkVerApiList;
}

const std::vector<SdkApiString>& nstool::RoMetadataProcess::getPublicApiList() const
{
	return mPublicApiList;
}

const std::vector<SdkApiString>& nstool::RoMetadataProcess::getDebugApiList() const
{
	return mDebugApiList;
}

const std::vector<SdkApiString>& nstool::RoMetadataProcess::getPrivateApiList() const
{
	return mPrivateApiList;
}

const std::vector<SdkApiString>& nstool::RoMetadataProcess::getGuidelineApiList() const
{
	return mGuidelineApiList;
}

const std::vector<ElfSymbolParser::sElfSymbol>& nstool::RoMetadataProcess::getSymbolList() const
{
	return mSymbolList.getSymbolList();
}

void nstool::RoMetadataProcess::importApiList()
{
	if (mRoBlob.size() == 0)
	{
		throw tc::Exception(kModuleName, "No ro binary set.");
	}

	if (mApiInfo.size > 0)
	{
		std::stringstream list_stream(std::string((char*)mRoBlob.data() + mApiInfo.offset, mApiInfo.size));
		std::string api_str;

		while(std::getline(list_stream, api_str, (char)0x00))
		{
			SdkApiString api(api_str);

			switch (api.getApiType())
			{
			case SdkApiString::API_SDK_VERSION:
				mSdkVerApiList.push_back(api);
				break;
			case SdkApiString::API_MIDDLEWARE:
				mPublicApiList.push_back(api);
				break;
			case SdkApiString::API_DEBUG:
				mDebugApiList.push_back(api);
				break;
			case SdkApiString::API_PRIVATE:
				mPrivateApiList.push_back(api);
				break;
			case SdkApiString::API_GUIDELINE:
				mGuidelineApiList.push_back(api);
				break;
			default:
				break;
			}
		}
	}

	if (mDynSym.size > 0)
	{
		mSymbolList.parseData(mRoBlob.data() + mDynSym.offset, mDynSym.size, mRoBlob.data() + mDynStr.offset, mDynStr.size, mIs64BitInstruction);
	}
}

void nstool::RoMetadataProcess::displayRoMetaData()
{
	size_t api_num = mSdkVerApiList.size() + mPublicApiList.size() + mDebugApiList.size() + mPrivateApiList.size();
	
	if (api_num > 0 && (mListApi || mCliOutputMode.show_extended_info))
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
		if (mGuidelineApiList.size() > 0)
		{
			std::cout << "  Guideline APIs:" << std::endl;
			for (size_t i = 0; i < mGuidelineApiList.size(); i++)
			{
				std::cout << "    " << mGuidelineApiList[i].getModuleName() << " (vender: " << mGuidelineApiList[i].getVenderName() << ")" << std::endl;
			}
		}
	}
	if (mSymbolList.getSymbolList().size() > 0 && (mListSymbols || mCliOutputMode.show_extended_info))
	{
		std::cout << "[Symbol List]" << std::endl;
		for (size_t i = 0; i < mSymbolList.getSymbolList().size(); i++)
		{
			const ElfSymbolParser::sElfSymbol& symbol = mSymbolList.getSymbolList()[i];
			std::cout << "  " << symbol.name << " [SHN=" << getSectionIndexStr(symbol.shn_index) << " (" << std::hex << std::setw(4) << std::setfill('0') << symbol.shn_index << ")][STT=" << getSymbolTypeStr(symbol.symbol_type) << "][STB=" << getSymbolBindingStr(symbol.symbol_binding) << "]" << std::endl;
		}
	}
}

const char* nstool::RoMetadataProcess::getSectionIndexStr(uint16_t shn_index) const
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

const char* nstool::RoMetadataProcess::getSymbolTypeStr(byte_t symbol_type) const
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

const char* nstool::RoMetadataProcess::getSymbolBindingStr(byte_t symbol_binding) const
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