#include <sstream>
#include "SdkApiString.h"

nstool::SdkApiString::SdkApiString(const std::string& full_str) :
	SdkApiString(API_MIDDLEWARE, "", "")
{
	resolveApiString(full_str);
}

nstool::SdkApiString::SdkApiString(ApiType type, const std::string& vender_name, const std::string& module_name) :
	mApiType(type),
	mVenderName(vender_name),
	mModuleName(module_name)
{

}

void nstool::SdkApiString::operator=(const SdkApiString& other)
{
	mApiType = other.mApiType;
	mVenderName = other.mVenderName;
	mModuleName = other.mModuleName;
}

nstool::SdkApiString::ApiType nstool::SdkApiString::getApiType() const
{
	return mApiType;
}

void nstool::SdkApiString::setApiType(ApiType type)
{
	mApiType = type;
}

const std::string& nstool::SdkApiString::getVenderName() const
{
	return mVenderName;
}

void nstool::SdkApiString::setVenderName(const std::string& name)
{
	mVenderName = name;
}

const std::string& nstool::SdkApiString::getModuleName() const
{
	return mModuleName;
}

void nstool::SdkApiString::setModuleName(const std::string& name)
{
	mModuleName = name;
}

void nstool::SdkApiString::resolveApiString(const std::string& full_str)
{
	std::stringstream list_stream(full_str);
	std::string api_type, vender, module;

	std::getline(list_stream, api_type, kSplitChar);
	std::getline(list_stream, vender, kSplitChar);
	std::getline(list_stream, module);


	if (api_type == kSdkMiddleWareApiString)
	{
		if (vender == kVenderNintendo && module.find(kSdkVersionString) != std::string::npos)
		{
			mApiType = API_SDK_VERSION;
		}
		else
		{
			mApiType = API_MIDDLEWARE;
		}
	}
	else if (api_type == kSdkDebugApiString)
	{
		mApiType = API_DEBUG;
	}
	else if (api_type == kSdkPrivateApiString)
	{
		mApiType = API_PRIVATE;
	}
	else if (api_type == kSdkGuidelineApiString)
	{
		mApiType = API_GUIDELINE;
	}
	else
	{
		// TODO?
	}

	mVenderName = vender;
	mModuleName = module;
}