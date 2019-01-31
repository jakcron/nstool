#include <sstream>
#include "SdkApiString.h"

SdkApiString::SdkApiString(const std::string& full_str) :
	SdkApiString(API_MIDDLEWARE, "", "")
{
	resolveApiString(full_str);
}

SdkApiString::SdkApiString(ApiType type, const std::string& vender_name, const std::string& module_name) :
	mApiType(type),
	mVenderName(vender_name),
	mModuleName(module_name)
{

}

void SdkApiString::operator=(const SdkApiString& other)
{
	mApiType = other.mApiType;
	mVenderName = other.mVenderName;
	mModuleName = other.mModuleName;
}

SdkApiString::ApiType SdkApiString::getApiType() const
{
	return mApiType;
}

void SdkApiString::setApiType(ApiType type)
{
	mApiType = type;
}

const std::string& SdkApiString::getVenderName() const
{
	return mVenderName;
}

void SdkApiString::setVenderName(const std::string& name)
{
	mVenderName = name;
}

const std::string& SdkApiString::getModuleName() const
{
	return mModuleName;
}

void SdkApiString::setModuleName(const std::string& name)
{
	mModuleName = name;
}

void SdkApiString::resolveApiString(const std::string& full_str)
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

	mVenderName = vender;
	mModuleName = module;
}