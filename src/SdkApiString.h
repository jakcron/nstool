#pragma once
#include "types.h"

namespace nstool {

class SdkApiString
{
public:
	enum ApiType
	{
		API_MIDDLEWARE,
		API_DEBUG,
		API_PRIVATE,
		API_SDK_VERSION,
		API_GUIDELINE,
	};

	SdkApiString(const std::string& full_str);
	SdkApiString(ApiType type, const std::string& vender_name, const std::string& module_name);

	void operator=(const SdkApiString& other);

	ApiType getApiType() const;
	void setApiType(ApiType type);

	const std::string& getVenderName() const;
	void setVenderName(const std::string& name);

	const std::string& getModuleName() const;
	void setModuleName(const std::string& name);
private:
	const std::string kModuleName = "SdkApiString";

	const char kSplitChar = '+';
	const std::string kSdkMiddleWareApiString = "SDK MW";
	const std::string kSdkDebugApiString = "SDK Debug";
	const std::string kSdkPrivateApiString = "SDK Private";
	const std::string kSdkGuidelineApiString = "SDK Guideline";
	const std::string kVenderNintendo = "Nintendo";
	const std::string kSdkVersionString = "NintendoSdk_nnSdk-";

	ApiType mApiType;
	std::string mVenderName;
	std::string mModuleName;

	void resolveApiString(const std::string& full_str);
};

}